#include "engine/core/layer/imgui/imgui_threaded_rendering.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

vshade::render::ImGuiThreadRenderData::ImGuiThreadRenderData(std::size_t const frames_in_flight) : frames_in_flight_{frames_in_flight}
{
    entries_.resize(frames_in_flight_);
}
vshade::render::ImGuiThreadRenderData::~ImGuiThreadRenderData()
{
    clear();
}

ImGuiID vshade::render::ImGuiThreadRenderData::getDrawListID(ImDrawList* src_list, std::size_t const frame_index)
{
    return ImHashData(&src_list, sizeof(src_list));
}

vshade::render::ImGuiThreadRenderEntry* vshade::render::ImGuiThreadRenderData::getOrAddEntry(ImDrawList* src_list, std::size_t const frame_index)
{
    return entries_.at(frame_index).cache.GetOrAddByKey(getDrawListID(src_list, frame_index));
}

void vshade::render::ImGuiThreadRenderData::createSnapshot(ImDrawData* src, ImVector<ImTextureData*>* textures, std::size_t const frame_index,
                                                           double const current_time)
{

    IM_ASSERT(src && src->Valid);
    IM_ASSERT(frame_index < frames_in_flight_);
    FrameData& frame = entries_.at(frame_index);

    frame.draw_data = *src;
    frame.draw_data.CmdLists.clear();

    for (ImDrawList* src_list : src->CmdLists)
    {
        ImGuiThreadRenderEntry* entry = getOrAddEntry(src_list, frame_index);

        if (!entry->current_copy)
            entry->current_copy = IM_NEW(ImDrawList)(src_list->_Data);

        entry->src_copy = src_list;

        entry->src_copy->CmdBuffer.swap(entry->current_copy->CmdBuffer);
        entry->src_copy->IdxBuffer.swap(entry->current_copy->IdxBuffer);
        entry->src_copy->VtxBuffer.swap(entry->current_copy->VtxBuffer);

        entry->src_copy->CmdBuffer.reserve(entry->current_copy->CmdBuffer.Capacity);
        entry->src_copy->IdxBuffer.reserve(entry->current_copy->IdxBuffer.Capacity);
        entry->src_copy->VtxBuffer.reserve(entry->current_copy->VtxBuffer.Capacity);

        entry->last_used_time = current_time;
        frame.draw_data.CmdLists.push_back(entry->current_copy);
    }

    //------------------------------------------------------------------------
    //  Cleanup unused data
    //------------------------------------------------------------------------
    cleanup(frame_index, current_time);
}

ImDrawData* vshade::render::ImGuiThreadRenderData::getDrawData(std::size_t const frame_index)
{
    IM_ASSERT(frame_index < frames_in_flight_);
    return &entries_.at(frame_index).draw_data;
}

void vshade::render::ImGuiThreadRenderData::processRender(std::uint32_t const frame_index, VkCommandBuffer vk_render_command_buffer)
{
    ImGui_ImplVulkan_UpdateTexture(*ImGui::GetPlatformIO().Textures.Data);
    ImGui_ImplVulkan_RenderDrawData(ImGuiThreadRenderData::instance().getDrawData(frame_index), vk_render_command_buffer);
}

void vshade::render::ImGuiThreadRenderData::waitAndSync()
{
    // It's very fragile, we want to make one first frame sync because of imgui font atlas
    static std::uint8_t count{0U};

    if (count == 0U)
    {
        count++;
    }
    else if (count == 1U)
    {
        render::Render::instance().waitUntilRenderDone();
        count++;
    }
}

void vshade::render::ImGuiThreadRenderData::cleanup(std::size_t const frame_index, double const current_time)
{
    double const gc_threshold = current_time - 20.0;
    FrameData&   frame        = entries_.at(frame_index);
    for (std::size_t i{0U}; i < frame.cache.GetMapSize(); ++i)
    {
        if (ImGuiThreadRenderEntry * entry{frame.cache.TryGetMapData(i)})
        {
            if (entry->last_used_time > gc_threshold)
                continue;
            IM_DELETE(entry->current_copy);
            frame.cache.Remove(getDrawListID(entry->src_copy, frame_index), entry);
        }
    }
}

void vshade::render::ImGuiThreadRenderData::clear()
{
    for (FrameData& frame : entries_)
    {
        for (std::size_t i{0U}; i < frame.cache.GetMapSize(); ++i)
        {
            if (ImGuiThreadRenderEntry * entry{frame.cache.TryGetMapData(i)})
            {
                IM_DELETE(entry->current_copy);
            }
        }

        frame.cache.Clear();
        frame.draw_data.Clear();
    }
}