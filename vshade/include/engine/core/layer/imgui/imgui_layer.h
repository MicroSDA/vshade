#ifndef ENGINE_CORE_LAYER_IMGUI_LAYER_H
#define ENGINE_CORE_LAYER_IMGUI_LAYER_H

#include <engine/core/layer/imgui/imgui_renderer.h>
#include <engine/core/layer/layer.h>
#include <engine/core/utility/utils.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace vshade
{
class ImGuiThemeEditor
{
public:
    // ===== Основные цвета =====
    inline static std::uint32_t background_color_{0x25213100};
    inline static std::uint32_t text_color_{0xF4F1DE00};
    inline static std::uint32_t main_color_{0xDA115E00};
    inline static std::uint32_t main_accent_color_{0x79235900};
    inline static std::uint32_t highlight_color_{0xC7EF0000};
    inline static std::uint32_t secondary_color_{0x0081A700};
    inline static std::uint32_t success_color_{0x4CAF5000};  
    inline static std::uint32_t warning_color_{0xFFC10700};  
    inline static std::uint32_t error_color_{0xF4433600}; 
    inline static std::uint32_t black_{0x00000000};
    inline static std::uint32_t white_{0xFFFFFF00};
    inline static std::uint32_t alpha_transparent_{0x00};
    inline static std::uint32_t alpha10_{0x1A};
    inline static std::uint32_t alpha20_{0x33};
    inline static std::uint32_t alpha40_{0x66};
    inline static std::uint32_t alpha50_{0x80};
    inline static std::uint32_t alpha60_{0x99};
    inline static std::uint32_t alpha80_{0xCC};
    inline static std::uint32_t alpha90_{0xE6};
    inline static std::uint32_t alpha100_{0xFF};
    inline static std::uint32_t alpha_dull_{0xFF};

    inline static std::string font_path_;

    static float getR(int const color_code)
    {
        return ((color_code >> 24) & 0xFF) / 255.0f;
    }
    static float getG(int const color_code)
    {
        return ((color_code >> 16) & 0xFF) / 255.0f;
    }
    static float getB(int const color_code)
    {
        return ((color_code >> 8) & 0xFF) / 255.0f;
    }
    static float getA(int const alpha_code)
    {
        return (float)alpha_code / 255.0f;
    }

    static ImVec4 getColor(int const c, int const a = alpha80_)
    {
        return ImVec4(getR(c), getG(c), getB(c), getA(a));
    }

    static ImVec4 darken(ImVec4 const c, float const p)
    {
        return ImVec4(std::max(0.f, c.x * (1.f - p)), std::max(0.f, c.y * (1.f - p)), std::max(0.f, c.z * (1.f - p)), c.w);
    }

    static ImVec4 lighten(ImVec4 const c, float const p)
    {
        return ImVec4(std::min(1.f, c.x * (1.f + p)), std::min(1.f, c.y * (1.f + p)), std::min(1.f, c.z * (1.f + p)), c.w);
    }

    static ImVec4 disabled(ImVec4 const c)
    {
        return darken(c, 0.6f);
    }
    static ImVec4 hovered(ImVec4 const c)
    {
        return lighten(c, 0.2f);
    }
    static ImVec4 active(ImVec4 const c)
    {
        return lighten(ImVec4(c.x, c.y, c.z, 1.0f), 0.1f);
    }
    static ImVec4 collapsed(ImVec4 const c)
    {
        return darken(c, 0.2f);
    }

    static void loadFont(char const* filename, float size_pixels = 0.f)
    {
        ImGui::GetIO().Fonts->AddFontFromFileTTF(std::string{vshade::file::FileManager::instance().getRootDirectory() + filename}.c_str(),
                                                 size_pixels);
    }

    static void setColors(int const background_color, int const text_color, int const main_color, int const main_accent_color,
                          int const highlight_color, int const secondary_color = secondary_color_, int const success_color = success_color_,
                          int const warning_color = warning_color_, int const error_color = error_color_)
    {
        background_color_  = background_color;
        text_color_        = text_color;
        main_color_        = main_color;
        main_accent_color_ = main_accent_color;
        highlight_color_   = highlight_color;
        secondary_color_   = secondary_color;
        success_color_     = success_color;
        warning_color_     = warning_color;
        error_color_       = error_color;
    }

    static void applyTheme()
    {
        ImVec4* colors = ImGui::GetStyle().Colors;

        colors[ImGuiCol_Text]         = getColor(text_color_);
        colors[ImGuiCol_TextDisabled] = disabled(colors[ImGuiCol_Text]);
        colors[ImGuiCol_WindowBg]     = getColor(background_color_);
        colors[ImGuiCol_ChildBg]      = darken(getColor(background_color_), 0.1f);
        colors[ImGuiCol_PopupBg]      = getColor(background_color_, alpha90_);
        colors[ImGuiCol_Border]       = getColor(secondary_color_, alpha40_);
        colors[ImGuiCol_BorderShadow] = getColor(black_);

        colors[ImGuiCol_FrameBg]        = getColor(main_accent_color_);
        colors[ImGuiCol_FrameBgHovered] = hovered(colors[ImGuiCol_FrameBg]);
        colors[ImGuiCol_FrameBgActive]  = active(colors[ImGuiCol_FrameBg]);

        colors[ImGuiCol_Button]        = getColor(main_color_, alpha80_);
        colors[ImGuiCol_ButtonHovered] = hovered(colors[ImGuiCol_Button]);
        colors[ImGuiCol_ButtonActive]  = active(colors[ImGuiCol_Button]);

        colors[ImGuiCol_Header]        = getColor(secondary_color_, alpha80_);
        colors[ImGuiCol_HeaderHovered] = hovered(colors[ImGuiCol_Header]);
        colors[ImGuiCol_HeaderActive]  = active(colors[ImGuiCol_Header]);

        colors[ImGuiCol_CheckMark]        = getColor(highlight_color_);
        colors[ImGuiCol_SliderGrab]       = getColor(highlight_color_);
        colors[ImGuiCol_SliderGrabActive] = active(colors[ImGuiCol_SliderGrab]);
        colors[ImGuiCol_PlotHistogram]        = getColor(success_color_);
        colors[ImGuiCol_PlotHistogramHovered] = hovered(colors[ImGuiCol_PlotHistogram]);
        colors[ImGuiCol_PlotLines]            = getColor(warning_color_);
        colors[ImGuiCol_PlotLinesHovered]     = hovered(colors[ImGuiCol_PlotLines]);
        colors[ImGuiCol_TextSelectedBg] = getColor(highlight_color_, alpha40_);
        colors[ImGuiCol_DragDropTarget] = getColor(error_color_, alpha80_);
        auto& style                    = ImGui::GetStyle();
        style.WindowMenuButtonPosition = ImGuiDir_None;
        style.WindowPadding            = ImVec2{6, 4};
        style.WindowRounding           = 6.0f;
        style.FramePadding             = ImVec2{5, 3};
        style.FrameRounding            = 3.0f;
        style.ItemSpacing              = ImVec2{4, 2};
        style.ItemInnerSpacing         = ImVec2{3, 3};
        style.IndentSpacing            = 30.0f;
        style.ScrollbarSize            = 12.0f;
        style.ScrollbarRounding        = 16.0f;
        style.GrabMinSize              = 20.0f;
        style.GrabRounding             = 2.0f;
        style.WindowTitleAlign.x       = 0.50f;
        style.FrameBorderSize          = 0.0f;
        style.WindowBorderSize         = 0.f;
        style.TabRounding              = 3.0f;
    }
};

class VSHADE_API ImGuiLayer : public Layer
{
    friend class utility::CRTPFactory<Layer>;

    template <typename T> constexpr T lerp(T a, T b, T t)
    {
        return a + t * (b - a);
    }

public:
    virtual ~ImGuiLayer()                      = default;
    ImGuiLayer(ImGuiLayer const&)              = delete;
    ImGuiLayer(ImGuiLayer&&)                   = delete;
    ImGuiLayer& operator=(ImGuiLayer const&) & = delete;
    ImGuiLayer& operator=(ImGuiLayer&&) &      = delete;

    void        drawTexture(std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& border_color, float const alpha = 1.0);
    bool        drawImageButton(char const* title, std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& border_color,
                                float const alpha = 1.0);
    static bool drawFloat(char const* title, float* value, float const min = -FLT_MAX, float const max = FLT_MAX, float const def = 0.f,
                          float const width = 0.f);
    static bool drawFloat2XY(char const* title, float* value, float const min = -FLT_MAX, float const max = FLT_MAX, float const def = 0.f);
    static bool drawFloat2XZ(char const* title, float* value, float const min = -FLT_MAX, float const max = FLT_MAX, float const def = 0.f);
    static bool drawFloat3(char const* title, float* value, float const min = -FLT_MAX, float const max = FLT_MAX, float const def = 0.f);
    static bool drawInt(char const* title, int* value, int const min = -INT32_MAX, int const max = INT32_MAX, int const def = 0.f);

    static void textUTF8(std::string const& string);
    static void drawFontIcon(char const* glyph, unsigned int const font_index, float scale, char const* hint = nullptr);
    static bool drawFontIconButton(char const* id, char const* glyph, unsigned int const font_index, float const scale = 1.f);

    static float getDpiScaleFactor(float const offset = 1.0f);

    static bool drawTimeSequencerLayer(char const* id, float* value, bool* is_playing, float* loop_start, float* loop_end, float const duration,
                                       float* play_speed);

    static std::pair<bool, bool> multiColorGradientWidget(char const* label, std::vector<ImVec4>& colors,
                                                          ImVec2 size = ImVec2{ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() / 2.f});
    template <typename... Funcs>
    static float drawDragBlock(char const* title, std::tuple<Funcs...> const& funcs, float const l_size = 0.f, float const r_size = 0.f)
    {
        float right_width{std::numeric_limits<float>::min()};
        if (ImGui::BeginTable(title, 2U, ImGuiTableFlags_SizingStretchProp)) // ImGuiTableFlags_SizingStretchSame
        {
            std::apply(
                [&](auto const&... pairs)
                {
                    ((ImGui::TableNextRow(), ImGui::TableNextColumn(), right_width = std::max(right_width, ImGui::GetContentRegionAvail().x),
                      ImGui::TextUnformatted(pairs.first), ImGui::TableNextColumn(), pairs.second()),
                     ...);
                },
                funcs);

            ImGui::EndTable();
        }

        return right_width;
    }
    template <typename T, std::size_t Size, typename Func>
    void drawPlotWidget(char const* id, vshade::StackArray<float, Size> const& data, std::size_t const graph_height_in_lines, Func callback,
                        T const vMax = 1)
    {
        if (!data.getSize())
            return;

        T value_min{std::numeric_limits<T>::max()}, value_max{std::numeric_limits<T>::min()}, value_avg{0};

        for (std::size_t i{0U}; i < data.getSize(); ++i)
        {
            value_min = std::min(value_min, static_cast<T>(data[i]));
            value_max = std::max(value_max, static_cast<T>(data[i]));
            value_avg += static_cast<T>(data[i]);
        }

        value_avg /= static_cast<T>(data.getSize());

        float       avg_graph_max{1.f};
        float const padded_max{value_max * 1.5f * vMax};
        avg_graph_max = lerp(avg_graph_max, padded_max, 0.05f);
        avg_graph_max = std::min(1000.f, std::max(avg_graph_max, value_max * 1.1f));

        float const graph_width{ImGui::GetContentRegionAvail().x};
        float const graph_height{ImGui::GetTextLineHeight() * graph_height_in_lines + ImGui::GetStyle().ItemSpacing.y * 2.0f};

        std::string info{callback(value_min, value_max, value_avg)};

        auto draw_list{ImGui::GetWindowDrawList()};

        ImVec2 center{ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2};

        ImGui::PlotLines(id, data.getData(), static_cast<int>(data.getSize()), 0, info.c_str(), 0.f, avg_graph_max * vMax,
                         ImVec2{graph_width, graph_height});

        if (ImGui::IsItemHovered())
        {
            if constexpr (std::is_integral_v<T>)
            {
                ImGui::SetTooltip("Min: %d, Max: %d, Avg: %d", value_min, value_max, value_avg);
            }
            else
            {
                ImGui::SetTooltip("Min: %.2f, Max: %.2f, Avg: %.2f", value_min, value_max, value_avg);
            }
        }
    }

    std::pair<bool, std::filesystem::path> openFile(bool* is_open, char const* path, char const* filter,
                                                    std::shared_ptr<render::Texture2D> background_texture = nullptr);
    std::pair<bool, std::filesystem::path> saveFile(char const* filter);
    std::pair<bool, std::filesystem::path> selectFolder(char const* filter);

protected:
    explicit ImGuiLayer(std::string const& name);

    int window_flags_{0};
    int dock_space_flags_{0};
};
} // namespace vshade

#endif // ENGINE_CORE_LAYER_IMGUI_LAYER_H