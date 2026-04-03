#include "engine/core/layer/imgui/imgui_layer.h"

vshade::ImGuiLayer::ImGuiLayer(std::string const& name) : vshade::Layer(name, vshade::render::ImGuiRenderer::create())
{
    window_flags_ = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags_ |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags_ |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    dock_space_flags_ = ImGuiDockNodeFlags_None;
}

bool vshade::ImGuiLayer::drawFloat(char const* title, float* value, float const min, float const max, float const def, float const width)
{
    bool has_been_edited{false};

    ImGui::PushID(title);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.257f, 0.542f, 0.852f, 0.709f});
    if (ImGui::Button("V", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
        *value = def;
    ImGui::PopStyleColor();

    ImGui::SameLine();

    !width ? ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x) : ImGui::PushItemWidth(width);
    has_been_edited = ImGui::DragFloat("##V", &value[0], 0.001f, min, max);
    ImGui::PopItemWidth();
    ImGui::PopID();

    return has_been_edited;
}

bool vshade::ImGuiLayer::drawFloat2XY(char const* title, float* value, float const min, float const max, float const def)
{
    bool has_been_edited{false};
    if (ImGui::BeginTable(title, 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.753f, 0.000f, 0.099f, 0.709f});
            if (ImGui::Button("X", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[0] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##X", &value[0], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.000f, 0.698f, 0.008f, 0.709f});
            if (ImGui::Button("Y", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[1] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##Y", &value[1], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::EndTable();
    }

    return has_been_edited;
}

bool vshade::ImGuiLayer::drawFloat2XZ(char const* title, float* value, float const min, float const max, float const def)
{
    bool has_been_edited{false};
    if (ImGui::BeginTable(title, 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.753f, 0.000f, 0.099f, 0.709f});
            if (ImGui::Button("X", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[0] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##X", &value[0], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.257f, 0.542f, 0.852f, 0.709f});
            if (ImGui::Button("Z", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[1] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##Z", &value[1], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::EndTable();
    }

    return has_been_edited;
}

bool vshade::ImGuiLayer::drawFloat3(char const* title, float* value, float const min, float const max, float const def)
{
    bool has_been_edited{false};
    if (ImGui::BeginTable(title, 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.753f, 0.000f, 0.099f, 0.709f});
            if (ImGui::Button("X", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[0] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##X", &value[0], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.000f, 0.698f, 0.008f, 0.709f});
            if (ImGui::Button("Y", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[1] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##Y", &value[1], 0.01f, min, max);
            ImGui::PopItemWidth();
        }
        ImGui::TableNextColumn();
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.257f, 0.542f, 0.852f, 0.709f});
            if (ImGui::Button("Z", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
                *&value[2] = def;
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            has_been_edited += ImGui::DragFloat("##Z", &value[2], 0.01f, min, max);
            ImGui::PopItemWidth();
        }

        ImGui::EndTable();
    }

    return has_been_edited;
}

bool vshade::ImGuiLayer::drawInt(char const* title, int* value, int const min, int const max, int const def)
{
    bool has_been_edited{false};

    ImGui::PushID(title);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.257f, 0.542f, 0.852f, 0.709f});
    if (ImGui::Button("V", ImVec2{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()}))
        *value = def;
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    has_been_edited = ImGui::DragInt("##V", &value[0], 0.01f, min, max);
    ImGui::PopItemWidth();
    ImGui::PopID();

    return has_been_edited;
}

void vshade::ImGuiLayer::ImGuiLayer::textUTF8(std::string const& string)
{
    ImGui::TextUnformatted((char*)string.c_str(), (char*)string.c_str() + string.size());
}

void vshade::ImGuiLayer::drawFontIcon(char const* glyph, unsigned int const font_index, float scale, char const* hint)
{
    ImGui::GetIO().Fonts->Fonts[font_index]->Scale = scale;
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[font_index]);
    ImGui::Text(glyph);
    ImGui::PopFont();
    ImGui::GetIO().Fonts->Fonts[font_index]->Scale = 1.f;

    if (hint && ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
        ImGui::TextUnformatted(hint);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool vshade::ImGuiLayer::drawFontIconButton(char const* id, char const* glyph, unsigned int const font_index, float const scale)
{
    float const font_size{ImGui::GetIO().Fonts->Fonts[font_index]->LegacySize * 2};
    float const font_scale{ImGui::GetIO().Fonts->Fonts[font_index]->Scale};

    float const button_dim{font_size * font_scale * scale};

    ImVec2 const screen_position{ImGui::GetCursorScreenPos()};

    bool const is_clicked{ImGui::InvisibleButton(id, ImVec2{button_dim, button_dim})};
    bool const is_hoverd{ImGui::IsItemHovered()};

    ImVec4 const color{is_clicked  ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]
                       : is_hoverd ? ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]
                                   : ImGui::GetStyle().Colors[ImGuiCol_Button]};

    ImGui::SetCursorScreenPos(screen_position);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    drawFontIcon(glyph, font_index, scale);
    ImGui::PopStyleColor();

    ImGui::SetCursorScreenPos(screen_position);
    ImGui::Dummy({ImVec2{button_dim, button_dim}});
    ImGui::Dummy({ImVec2{button_dim, button_dim}});
    // Debug
    // ImRect const buttonRect{screen_position, ImVec2{screen_position.x + button_dim, screen_position.y + button_dim}};
    // ImGui::GetWindowDrawList()->AddRectFilled(buttonRect.Min, buttonRect.Max, ImGui::ColorConvertFloat4ToU32(color));

    return is_clicked && is_hoverd;
}

float vshade::ImGuiLayer::getDpiScaleFactor(float const offset)
{
    ImVec2 framebuffer_scale = ImGui::GetIO().DisplayFramebufferScale;
    float  scale             = framebuffer_scale.x * offset;
    return scale;
}

static std::string formatTime(float time_seconds)
{
    int total_seconds = static_cast<int>(time_seconds);
    int hours         = total_seconds / 3600;
    int minutes       = (total_seconds % 3600) / 60;
    int seconds       = total_seconds % 60;

    std::ostringstream oss;
    if (hours > 0)
        oss << std::setfill('0') << std::setw(2) << hours << ":";
    oss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}

bool vshade::ImGuiLayer::drawTimeSequencerLayer(char const* id, float* value, bool* is_playing, float* loop_start, float* loop_end,
                                                float const duration, float* play_speed)
{
    *loop_start = glm::clamp(*loop_start, 0.0f, *loop_end);
    *loop_end   = glm::clamp(*loop_end, *loop_start, duration);
    *value      = std::clamp(*value, *loop_start, *loop_end);

    if (ImGui::Begin(id, nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoTitleBar))
    {
        ImVec2 offset{ImGui::GetFrameHeight(), ImGui::GetFrameHeight()};
        ImVec2 screen_position{ImGui::GetCursorScreenPos().x + offset.x, ImGui::GetCursorScreenPos().y + offset.y};

        ImGui::SetCursorScreenPos(screen_position);

        std::string const button_title{*is_playing ? "Pause" : "Play"};

        if (ImGui::ArrowButton("<-", ImGuiDir_Left))
        {
            *value -= 0.5f;
        }
        ImGui::SameLine();

        if (ImGui::Button(button_title.c_str(), ImVec2{100.f, 0.f}))
        {
            *is_playing = !*is_playing;
        }
        ImGui::SameLine();

        if (ImGui::ArrowButton("->", ImGuiDir_Right))
        {
            *value += 0.5f;
        }
        ImGui::SameLine();

        drawFloat("Play speed", play_speed, 0.f, 10.f, 1.f, 100.f);

        ImGui::SameLine();
        screen_position.x = ImGui::GetCursorScreenPos().x + offset.x;

        constexpr uint32_t num_segments{10U};
        constexpr uint32_t num_sub_segments{2U};
        float const        track_height{ImGui::GetFrameHeight()};

        ImDrawList* draw_list{ImGui::GetWindowDrawList()};

        ImVec2 window_size{ImGui::GetContentRegionAvail()};

        ImVec2 frame_size{window_size.x - offset.x * 2.f, track_height};

        float const segment_size{frame_size.x / num_segments};
        float const segment_time_size{duration / num_segments};

        // Track background
        draw_list->AddRectFilled(screen_position, ImVec2{screen_position.x + frame_size.x, screen_position.y + frame_size.y},
                                 IM_COL32(40, 40, 40, 255), 5.0f);

        // Play range
        float const loop_start_x{screen_position.x + (*loop_start / duration) * frame_size.x};
        float const loop_end_x{screen_position.x + (*loop_end / duration) * frame_size.x};

        draw_list->AddRectFilled(ImVec2{loop_start_x, screen_position.y}, ImVec2{loop_end_x, screen_position.y + frame_size.y},
                                 ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]), 5.0f);

        // Progress
        float const progress_x{screen_position.x + (*value / duration) * frame_size.x};
        draw_list->AddRectFilled(ImVec2{loop_start_x, screen_position.y}, ImVec2{progress_x, screen_position.y + frame_size.y},
                                 ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_SliderGrab]), 5.0f);

        // Time
        for (std::uint32_t i{0U}; i <= num_segments; ++i)
        {
            float const x_pos{screen_position.x + segment_size * i};
            draw_list->AddLine(ImVec2{x_pos, screen_position.y}, ImVec2{x_pos, screen_position.y + frame_size.y},
                               ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), 3.0f);

            std::string const label{formatTime(segment_time_size * i)};
            ImVec2 const      text_size{ImGui::CalcTextSize(label.c_str())};

            draw_list->AddText(ImVec2{x_pos - text_size.x / 2.f, screen_position.y + frame_size.y},
                               ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label.c_str());

            if (i < num_segments)
            {
                for (std::uint32_t j{1U}; j < num_sub_segments; ++j)
                {
                    float const sub_x{x_pos + (segment_size / num_sub_segments) * j};
                    draw_list->AddLine(ImVec2{sub_x, screen_position.y + frame_size.y * 0.5f}, ImVec2{sub_x, screen_position.y + frame_size.y},
                                       ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), 2.0f);
                }
            }
        }

        draw_list->AddCircleFilled(ImVec2{progress_x, screen_position.y + frame_size.y / 2.0f}, 6.0f, IM_COL32(255, 255, 255, 255));

        // Current time
        {
            std::string const label{formatTime(*value)};
            ImVec2 const      text_size{ImGui::CalcTextSize(label.c_str())};
            ImVec2 const      text_pos{progress_x - text_size.x * 0.5f, screen_position.y - text_size.y - ImGui::GetStyle().ItemInnerSpacing.y};

            draw_list->AddText(text_pos, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label.c_str());
        }

        float const handle_height{ImGui::GetFrameHeight() * 0.8f};
        float const handle_half_width{handle_height * 0.5f};

        auto DrawHandle = [&](char const* id, float x, ImU32 col, float* loop_pos, float min_value, float max_value, bool const is_flipped)
        {
            ImVec2 const handle_center{x, screen_position.y};
            ImVec2 const top_left{x - handle_half_width, screen_position.y - handle_height};
            ImVec2 const top_right{x + handle_half_width, screen_position.y - handle_height};

            draw_list->AddTriangleFilled(handle_center, top_left, top_right, col);

            ImVec2 const button_pos{x - handle_half_width * 1.2f, screen_position.y - handle_height};
            ImVec2 const button_size(handle_half_width * 3.f, handle_height * 3.f);

            ImGui::SetCursorScreenPos(button_pos);
            ImGui::InvisibleButton(id, button_size);

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                float const rel_x{std::clamp(ImGui::GetIO().MousePosPrev.x - screen_position.x, 0.0f, frame_size.x)};
                *loop_pos = std::clamp((rel_x / frame_size.x) * duration, min_value, max_value);
            }
        };

        DrawHandle("min", loop_start_x, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TabSelectedOverline]), loop_start, 0.0f,
                   *loop_end - 0.1f, false);
        DrawHandle("max", loop_end_x, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TabSelectedOverline]), loop_end,
                   *loop_start + 0.1f, duration, true);

        ImGui::PushItemWidth(frame_size.x);
        ImGui::SetCursorScreenPos(ImVec2{screen_position.x, screen_position.y});
        ImGui::InvisibleButton(id, frame_size);

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            float const mouse_x{ImGui::GetIO().MousePos.x};
            float const rel_x{std::clamp(mouse_x - screen_position.x, 0.0f, frame_size.x)};
            *value = (rel_x / frame_size.x) * duration;
        }

        ImGui::PopItemWidth();
    }
    ImGui::End();
    return false;
}

bool vshade::ImGuiLayer::drawImageButton(char const* title, std::shared_ptr<render::Texture2D> texture, ImVec2 const& size,
                                         ImVec4 const& border_color, float const alpha)
{
    return renderer_->as<vshade::render::ImGuiRenderer>().drawImageButton(title, texture, size, border_color, alpha);
}

void vshade::ImGuiLayer::drawTexture(std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& border_color, float const alpha)
{
    renderer_->as<vshade::render::ImGuiRenderer>().drawTexture(texture, size, border_color, alpha);
}

std::pair<bool, bool> vshade::ImGuiLayer::multiColorGradientWidget(char const* label, std::vector<ImVec4>& colors, ImVec2 size)
{
    ImGui::PushID(label);
    std::size_t const num_colors{colors.size()};
    if (num_colors < 2U)
        return {false, false};

    ImVec2       reset_size = ImVec2{ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2.0f, size.y + ImGui::GetFrameHeight()};
    float        total_reset_width = reset_size.x + ImGui::GetStyle().ItemSpacing.x;
    ImVec2 const segment_size{(size.x - total_reset_width) / (num_colors - 1U), size.y};

    bool        edit{false}, reset{false};
    ImDrawList* draw_list{ImGui::GetWindowDrawList()};
    ImVec2      base_pos{ImGui::GetCursorScreenPos()};
    float       color_picker_y{base_pos.y + segment_size.y + ImGui::GetStyle().ItemSpacing.y};

    ImVec2 const p0{base_pos.x, base_pos.y};
    ImVec2 const p1{base_pos.x + segment_size.x, base_pos.y + segment_size.y};

    draw_list->AddRectFilledMultiColor(p0, p1, ImGui::ColorConvertFloat4ToU32(colors.at(0U)), ImGui::ColorConvertFloat4ToU32(colors.at(1U)),
                                       ImGui::ColorConvertFloat4ToU32(colors.at(1U)), ImGui::ColorConvertFloat4ToU32(colors.at(0U)));

    ImGui::SetCursorScreenPos(ImVec2{p0.x, color_picker_y});
    edit += ImGui::ColorEdit4(("##Color" + std::to_string(0U)).c_str(), (float*)&colors.at(0U),
                              ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);

    for (std::size_t i{1U}; i < num_colors - 1U; ++i)
    {
        ImVec2 const p0{base_pos.x + segment_size.x * i, base_pos.y};
        ImVec2 const p1{base_pos.x + segment_size.x * (i + 1U), base_pos.y + segment_size.y};

        draw_list->AddRectFilledMultiColor(p0, p1, ImGui::ColorConvertFloat4ToU32(colors.at(i)), ImGui::ColorConvertFloat4ToU32(colors.at(i + 1U)),
                                           ImGui::ColorConvertFloat4ToU32(colors.at(i + 1U)), ImGui::ColorConvertFloat4ToU32(colors.at(i)));
        ImGui::SetCursorScreenPos(ImVec2{p0.x - ImGui::GetFrameHeight() / 2.f, color_picker_y});
        edit += ImGui::ColorEdit4(("##Color" + std::to_string(i)).c_str(), (float*)&colors.at(i),
                                  ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
    }

    ImVec2 const last_pos{base_pos.x + segment_size.x * (num_colors - 1U) - ImGui::GetFrameHeight(), color_picker_y};
    ImGui::SetCursorScreenPos(last_pos);
    edit += ImGui::ColorEdit4(("##Color" + std::to_string(num_colors - 1U)).c_str(), (float*)&colors.at(num_colors - 1U),
                              ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);

    ImVec2 const reset_pos{base_pos.x + size.x - reset_size.x, base_pos.y};
    ImGui::SetCursorScreenPos(reset_pos);
    if (ImGui::Button("Reset", reset_size))
    {
        reset = true;
    }
    ImGui::PopID();
    return {edit, reset};
}

std::pair<bool, std::filesystem::path> vshade::ImGuiLayer::openFile(bool* is_open, char const* path, char const* filter,
                                                                    std::shared_ptr<render::Texture2D> background_texture)
{
    std::filesystem::path selected_file;
    bool                  is_file_selected{false};

    if (*is_open)
    {
        static std::filesystem::path current_path{std::filesystem::current_path()};
        static std::filesystem::path root_path{path};
        static std::filesystem::path last_path{path};

        ImVec2 const view_port_size{ImGui::GetMainViewport()->Size};
        ImVec2 const view_port_position{ImGui::GetMainViewport()->Pos};

        ImGui::SetNextWindowSize(view_port_size);
        ImGui::SetNextWindowPos(view_port_position);
        ImGui::SetNextWindowBgAlpha(1.0f);

        if (ImGui::Begin("File dialog", is_open))
        {
            ImVec2 const size{ImGui::GetContentRegionAvail()};
            ImVec2 const position{ImGui::GetCursorScreenPos()};

            if (background_texture)
                drawTexture(background_texture, size, {0.f, 0.f, 0.f, 0.f}, 0.3f);

            ImGui::SetCursorScreenPos(position);

            if (ImGui::BeginChild("Root", ImVec2{size.x / 3, 0}, ImGuiChildFlags_Borders))
            {
                ImGui::Text("Root");
                ImGui::Separator();
                for (auto const& entry : std::filesystem::directory_iterator(root_path))
                {
                    if (entry.is_directory())
                    {
                        auto name{entry.path().filename().string() + "/"};

                        if (ImGui::Selectable(name.c_str(), false))
                        {
                            current_path = entry.path();
                        }
                    }
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            if (ImGui::BeginChild("Folders", ImVec2{0, 0}, ImGuiChildFlags_Borders))
            {
                if (current_path.has_parent_path())
                {
                    if (ImGui::Selectable("...", false))
                    {
                        current_path = current_path.parent_path();
                    }
                }
                ImGui::Separator();
                for (auto const& entry : std::filesystem::directory_iterator(current_path))
                {
                    auto name{entry.path().filename().string() + "/"};

                    if (entry.is_directory())
                    {
                        if (ImGui::Selectable(name.c_str(), false))
                        {
                            current_path = entry.path();
                            last_path    = current_path;
                        }
                    }
                }
                for (auto const& entry : std::filesystem::directory_iterator(current_path))
                {
                    if (entry.is_regular_file())
                    {
                        if (strcmp(entry.path().extension().string().c_str(), filter) == 0)
                        {
                            if (ImGui::Selectable(entry.path().filename().string().c_str(), false))
                            {
                                selected_file    = entry;
                                is_file_selected = true;
                                *is_open         = false;
                            }
                        }
                    }
                }
            }
            ImGui::EndChild();

            ImGui::End();
        }
    }

    return {is_file_selected, selected_file};
}

std::pair<bool, std::filesystem::path> vshade::ImGuiLayer::saveFile(char const* filter)
{
    return {false, std::filesystem::path()};
}

std::pair<bool, std::filesystem::path> vshade::ImGuiLayer::selectFolder(char const* filter)
{
    return {false, std::filesystem::path()};
}
