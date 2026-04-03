// Helper for staged/multi-threaded rendering v0.10, for Dear ImGui
// Get latest version at http://www.github.com/ocornut/imgui_club
// Licensed under The MIT License (MIT)
// Based on a discussion at https://github.com/ocornut/imgui/issues/1860#issuecomment-1927630727
// Since 1.92.0, textures also needs to be updated. See discussion at https://github.com/ocornut/imgui/issues/8597
// CHANGELOG:
// - v0.10: (2025/04/30): initial version. Not well tested.
// Usage:
/*
    // Storage. Keep persistent as we reuse buffers across frames.
    static ImDrawDataSnapshot snapshot;
    // [Update thread] Take a snapshot of the ImDrawData
    snapshot.SnapUsingSwap(ImGui::GetDrawData(), ImGui::GetTime());
    // [Render thread] Render later
    ImGui_ImplDX11_RenderDrawData(&snapshot.DrawData);
*/
// FIXME: Could store an ID in ImDrawList to make this easier for user.
#ifndef ENGINE_CORE_LAYER_IMGUI_IMGUI_THREADED_RENDERING_H
#define ENGINE_CORE_LAYER_IMGUI_IMGUI_THREADED_RENDERING_H
#include "imgui_internal.h" // ImPool<>, ImHashData
#include <ankerl/unordered_dense.h>
#include <engine/core/render/render.h>
#include <engine/core/utility/singleton.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <vector>

namespace vshade
{
namespace render
{
struct ImTextureRenderQueueRequest
{
    ImTextureData  TexCopy;
    ImTextureData* TexInMainThread;

    ~ImTextureRenderQueueRequest()
    {
        TexCopy.Pixels = NULL;
    }
};

struct ImTextureRenderQueue
{
    ImVector<ImTextureRenderQueueRequest> Requests;

    // Main/Update thread queue requests.
    void queueRequests(ImVector<ImTextureData*>* textures);

    // Render thread process requests.
    template <typename Backend_UpdateTextureFunc> void processRequests(Backend_UpdateTextureFunc update_texture_func, int in_flight_frames);
};

template <typename Backend_UpdateTextureFunc>
inline void ImTextureRenderQueue::processRequests(Backend_UpdateTextureFunc update_texture_func, int in_flight_frames)
{
    for (auto i = 0; i < Requests.size(); i++)
    {
        auto& req = Requests[i];

        ImTextureData* tex_copy = &req.TexCopy;
        if (tex_copy->Status == ImTextureStatus_OK || tex_copy->Status == ImTextureStatus_Destroyed)
            continue;
        if (tex_copy->Status == ImTextureStatus_WantDestroy && tex_copy->UnusedFrames <= in_flight_frames)
            continue;
        ImTextureStatus prev_status = tex_copy->Status;

        // Call backend function
        update_texture_func(tex_copy);

        // Backend must honor immediately
        if (prev_status == ImTextureStatus_WantCreate || prev_status == ImTextureStatus_WantUpdates)
            IM_ASSERT(tex_copy->Status == ImTextureStatus_OK);
        if (prev_status == ImTextureStatus_WantDestroy)
            IM_ASSERT(tex_copy->Status == ImTextureStatus_Destroyed);

        // Write back to main thread
        if (prev_status == ImTextureStatus_WantCreate)
        {
            // Make sure future requests will access the correct texture
            for (auto j = i + 1; j < Requests.size(); j++)
            {
                auto& subsequent_req = Requests[j];
                if (subsequent_req.TexCopy.UniqueID == req.TexCopy.UniqueID)
                {
                    subsequent_req.TexCopy.SetTexID(tex_copy->TexID);
                    subsequent_req.TexCopy.BackendUserData = tex_copy->BackendUserData;
                }
            }

            req.TexInMainThread->SetTexID(tex_copy->TexID);
            req.TexInMainThread->BackendUserData = tex_copy->BackendUserData;
        }
    }
    Requests.resize(0);
}

struct ImGuiThreadRenderEntry
{
    ImDrawList* src_copy{nullptr};     // Pointer to iriginal ImDrawList from ImGui (current frame)
    ImDrawList* current_copy{nullptr}; // Local copy that we will use for render
    double      last_used_time{0.0};
};
class ImGuiThreadRenderData final : public utility::CRTPSingleton<ImGuiThreadRenderData>
{
    friend class utility::CRTPSingleton<ImGuiThreadRenderData>;
    struct FrameData
    {
        ImDrawData                                                                  draw_data;
        ImPool<ImGuiThreadRenderEntry>                                              cache;
        ankerl::unordered_dense::map<int, std::pair<ImTextureData*, ImTextureData>> textures;
        // ImTextureRenderQueue           textures;
    };

public:
    virtual ~ImGuiThreadRenderData();
    ImGuiThreadRenderData(ImGuiThreadRenderData const&)              = delete;
    ImGuiThreadRenderData(ImGuiThreadRenderData&&)                   = delete;
    ImGuiThreadRenderData& operator=(ImGuiThreadRenderData const&) & = delete;
    ImGuiThreadRenderData& operator=(ImGuiThreadRenderData&&) &      = delete;
    void        createSnapshot(ImDrawData* src, ImVector<ImTextureData*>* textures, std::size_t const frame_index, double const current_time);
    ImDrawData* getDrawData(std::size_t const frame_index);
    ImVector<ImTextureData*>* getTextures(std::size_t const frame_index);
    void                      processRender(std::uint32_t const frame_index, VkCommandBuffer vk_render_command_buffer);
    void                      waitAndSync();

private:
    explicit ImGuiThreadRenderData(std::size_t const frames_in_flight = 1U);
    ImGuiID                 getDrawListID(ImDrawList* src_list, std::size_t const frame_index);
    ImGuiThreadRenderEntry* getOrAddEntry(ImDrawList* src_list, std::size_t const frame_index);
    void                    cleanup(std::size_t const frame_index, double const current_time);
    void                    clear();
    std::vector<FrameData>  entries_;
    std::size_t const       frames_in_flight_{0U};
};

} // namespace render
} // namespace vshade
#endif //  ENGINE_CORE_LAYER_IMGUI_IMGUI_THREADED_RENDERING_H