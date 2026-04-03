#include "layer/main_layer.h"

editor::MainLayer::MainLayer()
    : vshade::ImGuiLayer("Main layer")
    , main_camera_{vshade::render::Camera::create<vshade::render::Camera>(vshade::render::Projection::_PERSPECTIVE_)}
{
    ImGui::GetStyle().FontScaleDpi = getDpiScaleFactor(1.5f); // Make font just a bit bigger

    ImGui::GetStyle().FontScaleDpi = getDpiScaleFactor(1.5f); // Make font just a bit bigger

    //------------------------------------------------------------------------
    //  Setup visual style
    //  Setup visual style
    //------------------------------------------------------------------------
    vshade::ImGuiThemeEditor::setColors(0x20252AFF, /*⬛ background */
                                        0xFAFFFDFF, /*⬜ text */
                                        0x3A3448FF, /*🟪 main */
                                        0x323845FF, /*🟦 main_accent */
                                        0xFFC307B1, /*🟨 highlight */
                                        0x6C757DFF, /*⬜ secondary*/
                                        0x28A745FF, /*🟩 success*/
                                        0xFFC107FF, /*🟧 warning  */
                                        0xDC3545FF  /*🟥 error*/
    );

    //------------------------------------------------------------------------
    //   Load base font
    //------------------------------------------------------------------------
    // vshade::ImGuiThemeEditor::loadFont("/resources/assets/fonts/Roboto-Medium.ttf", 14.f);
    // //------------------------------------------------------------------------
    // // Load icons
    // //------------------------------------------------------------------------
    // vshade::ImGuiThemeEditor::loadFont("/resources/assets/fonts/Icons.ttf", 14.f);
    vshade::ImGuiThemeEditor::applyTheme();

    //------------------------------------------------------------------------
    // Load texture resources
    //------------------------------------------------------------------------
    if (auto file = vshade::file::FileManager::instance().getNativeFile("/resources/assets/textures/texture.dds", std::ios::binary))
    {
        vshade::render::Image image;
        vshade::serializer::Serializer::deserialize(file, image);
        auto texture = vshade::render::Texture2D::createExplicit(vshade::render::Image2D::create(image));
    }

    //------------------------------------------------------------------------
    // Declare resources which we want to share
    //------------------------------------------------------------------------
    vshade::LayerResourceManager::instance().declare("Main camera", &main_camera_);
    vshade::LayerResourceManager::instance().declare("Frame size", &frame_size_);
    //------------------------------------------------------------------------
    // Set camera default position
    //------------------------------------------------------------------------
    main_camera_->setPosition(0.f, 3.f, -10.f);
}

void editor::MainLayer::onUpdate(std::shared_ptr<vshade::Scene> scene, vshade::time::FrameTimer const& frame_timer)
{

    //------------------------------------------------------------------------
    //  Update camera movment
    //  Update camera movment
    //------------------------------------------------------------------------
    updateCamera(frame_timer);

    //------------------------------------------------------------------------
    // Get frame buffer to fetch window size. Looks like it's roodenant, can
    // we just use ImGui::GetMainViewport()->WorkSize instead ?
    //------------------------------------------------------------------------
    if (auto frame_buffer = *vshade::LayerResourceManager::instance().get<std::shared_ptr<vshade::render::FrameBuffer>>("ImGuiRenderer frame buffer"))
    {
        frame_size_.x = frame_buffer->getWidth();
        frame_size_.y = frame_buffer->getHeight();
    }

}

void editor::MainLayer::onRender(std::shared_ptr<vshade::Scene> const scene, vshade::time::FrameTimer const& frame_timer)
{
    ImGui::SetCurrentContext(renderer_->as<vshade::render::ImGuiRenderer>().getImGuiContext());

    float constexpr alpha{0.6f};
    //------------------------------------------------------------------------
    // Draw time sequencer and file menu
    //------------------------------------------------------------------------
    float y_offset{drawFileMenu()};

    //------------------------------------------------------------------------
    // Draw collapse buttons to hide/show performance overlay and settings
    //------------------------------------------------------------------------
    y_offset += drawCollapseButtons(y_offset);

    //------------------------------------------------------------------------
    // Draw performace overalys fps
    //------------------------------------------------------------------------
    if (is_performace_opend_)
    {
        drawPerformanceOverlay(frame_timer, y_offset + ImGui::GetFrameHeight() / 3, alpha);
    }

    //------------------------------------------------------------------------
    // Draw camera  and other settings
    //------------------------------------------------------------------------
    if (is_setting_opend_)
    {
        drawSettings(y_offset + ImGui::GetFrameHeight() / 3, alpha);
    }

    drawTimeSequencer();
}

void editor::MainLayer::onEvent(std::shared_ptr<vshade::Scene> scene, std::shared_ptr<vshade::event::Event> const event,
                                vshade::time::FrameTimer const& frame_timer)
{
    //------------------------------------------------------------------------
    // Update camera zoom
    //------------------------------------------------------------------------
    if (event->getType() == vshade::event::Event::Type::_MOUSE_SCROLLED_)
    {
        main_camera_->setFov(main_camera_->getFov() - event->as<vshade::event::MouseScrolledEvent>().getYOffset() * camera_movement_speed_ * 20.f);
        main_camera_->setFov(std::clamp(main_camera_->getFov(), 10.f, 120.f));
    }
}

float editor::MainLayer::drawFileMenu()
{
    static bool is_open{true};

    ImGuiViewport const* imgui_view_port{ImGui::GetMainViewport()};
    ImGui::SetNextWindowSize(ImVec2{imgui_view_port->Size.x, 0.f});
    ImGui::SetNextWindowPos(ImVec2{imgui_view_port->Pos.x, 0.f}, ImGuiCond_Always);

    if (ImGui::Begin("File menu", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_MenuBar |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::MenuItem("Open file ..."))
            {
                is_file_dialog_open_ = true;
            }
            if (ImGui::MenuItem("Settings"))
            {
            }
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();

    return ImGui::GetCursorScreenPos().y;
}

void editor::MainLayer::drawSettings(float const y_offset, float const alpha)
{
    ImGuiViewport const* imgui_viewport{ImGui::GetMainViewport()};
    ImVec2 const         window_size{400.f * getDpiScaleFactor(1.5f), 0.f};

    //------------------------------------------------------------------------
    // Set window size depends on dpi scale
    //------------------------------------------------------------------------
    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowSizeConstraints(ImVec2{0, 0}, ImVec2{imgui_viewport->Size.x, imgui_viewport->Size.y - y_offset});
    ImGui::SetNextWindowBgAlpha(alpha);

    //------------------------------------------------------------------------
    // Make window movable out from main viewport
    //------------------------------------------------------------------------
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
    {
        ImGui::SetNextWindowPos(ImVec2{imgui_viewport->Pos.x + imgui_viewport->Size.x - window_size.x, imgui_viewport->Pos.y + y_offset},
                                ImGuiCond_Always);
    }

    if (ImGui::Begin("Settings", nullptr,
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImVec2 const screen_position{ImGui::GetCursorScreenPos()};
        float const width = drawDragBlock(
            "Settings",
            std::make_tuple(
                std::pair{"Camera",
                          [&]()
                          {
                              float const width{(ImGui::GetContentRegionAvail().x / 4.1f)  * getDpiScaleFactor()};
                              if (ImGui::Button("2D", ImVec2{width, 0.f}))
                              {
                                  main_camera_->setPosition(0.f, 1.5f, 0.1f);
                                  main_camera_->setForwardDirection(0.f, 0.f, 1.f);
                                  main_camera_->setPitch(glm::radians(0.f));
                                  main_camera_->setFov(70.f);
                              };
                              ImGui::SameLine();
                              if (ImGui::Button("Up", ImVec2{width, 0.f}))
                              {
                                  main_camera_->setPosition(0.f, 40.f, 110.f);
                                  main_camera_->setForwardDirection(0.f, -1.f, 0.f);
                                  main_camera_->setPitch(glm::radians(-89.99f));
                                  main_camera_->setFov(120.f);
                              };
                              ImGui::SameLine();
                              if (ImGui::Button("Left", ImVec2{width, 0.f}))
                              {
                                  main_camera_->setPosition(-80.f, 1.f, 50.f);
                                  main_camera_->setForwardDirection(-1.f, 0.f, 0.f);
                                  main_camera_->setYaw(glm::radians(0.f));
                                  main_camera_->setFov(50.f);
                              };
                              ImGui::SameLine();
                              if (ImGui::Button("Reset", ImVec2{width, 0.f}))
                              {
                                  main_camera_->setPosition(0.f, 3.f, -10.f);
                                  main_camera_->setForwardDirection(0.f, 0.f, 1.f);
                                  main_camera_->setPitch(glm::radians(-5.f));
                                  main_camera_->setFov(50.f);
                              };
                          }},
                std::pair{"Projection",
                          [&]()
                          {
                              // 0 — Perspective, 1 — Orthographic
                              static int  projection_type_index = 0;
                              char const* projection_types[]    = {"Perspective", "Orthographic"};

                              ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                              if (ImGui::Combo("##Projection", &projection_type_index, projection_types, IM_ARRAYSIZE(projection_types)))
                              {
                                  if (projection_type_index == 0)
                                      main_camera_->setProjection(vshade::render::Projection::_PERSPECTIVE_);
                                  else
                                      main_camera_->setProjection(vshade::render::Projection::_ORTHOGRAPHIC_);
                              }
                              ImGui::PopItemWidth();
                          }},
                std::pair{"Position",
                          [&]()
                          {
                              drawFloat3("Camera position", glm::value_ptr(main_camera_->getPosition()), -FLT_MAX, FLT_MAX);
                              ImGui::BeginDisabled();
                          }},
                std::pair{"PRY",
                          [&]()
                          {
                              glm::vec3 orientation{glm::degrees(main_camera_->getPitch()), glm::degrees(main_camera_->getYaw()),
                                                    glm::degrees(main_camera_->getRoll())};
                              if (drawFloat3("PYR", glm::value_ptr(orientation), -90.f, 90.f))
                              {
                                  main_camera_->setPitch(glm::radians(orientation.x));
                                  main_camera_->setYaw(glm::radians(orientation.y));
                                  main_camera_->setRoll(glm::radians(orientation.z));
                              }
                              ImGui::EndDisabled();
                          }},
                std::pair{"Fov", [&]() { drawFloat("Camera fov", &main_camera_->getFov(), 0.f, 120.f); }},
                std::pair{"Near", [&]() { drawFloat("Camera zNear", &main_camera_->getNear(), -FLT_MAX, FLT_MAX); }},
                std::pair{"Far", [&]() { drawFloat("Camera zFar", &main_camera_->getFar(), -FLT_MAX, FLT_MAX); }},
                std::pair{"Movment speed", [&]() { drawFloat("Movment speed", &camera_movement_speed_, 0.f, FLT_MAX, 0.03f); }},
                std::pair{"Rotation speed", [&]() { drawFloat("Rotation speed", &camera_rotation_speed_, 0.f, FLT_MAX, 0.15f); }},
                std::pair{"", [&]() {}},

                std::pair{"Show grid", [&]() { 
                    
                     bool is_active{vshade::render::Render::instance().getRegisterdPipline("Grid")->isActive()};
                     if(ImGui::Checkbox("##Show grid", &is_active))
                     {
                           vshade::render::Render::instance().getRegisterdPipline("Grid")->setActive(is_active);
                     }
                
                }}));

        //------------------------------------------------------------------------
        // Pipelines recompilation
        //------------------------------------------------------------------------

        // #ifdef _VSHADE_DEBUG_
        if (ImGui::BeginTable("Pipelines", 2, ImGuiTableFlags_SizingStretchProp)) // ImGuiTableFlags_SizingStretchSame
        {
            for (auto& [name, pipeline] : vshade::render::Render::instance().getRegisterdPiplines())
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(name.c_str());
                ImGui::Dummy(ImVec2{width, 0});
                ImGui::TableNextColumn();

                ImGui::PushID(name.c_str());
                if (ImGui::Button("Recompile", ImVec2{ImGui::GetContentRegionAvail().x, 0}))
                {
                    pipeline->recompile();
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // #endif // _VSHADE_DEBUG_
    }
    ImGui::End();
}

float editor::MainLayer::drawTimeSequencer()
{
    ImGuiViewport const* imgui_view_port{ImGui::GetMainViewport()};
    ImGui::SetNextWindowSize(ImVec2{imgui_view_port->Size.x, 0.f});
    ImGui::SetNextWindowPos(ImVec2{imgui_view_port->Pos.x, imgui_view_port->Size.y}, ImGuiCond_Always, ImVec2(0.0f, 1.0f));

    static float min{0.f};
    static float max{std::numeric_limits<float>::max()};
    float const  duration{std::numeric_limits<float>::max()};

    float current_time{0.f};
    bool is_playing{false};
    drawTimeSequencerLayer("drawTimeSequencerLayer", &current_time, &is_playing, &min, &max,
                           duration, &play_speed_);



    return ImGui::GetCursorScreenPos().y;
}

void editor::MainLayer::drawPerformanceOverlay(vshade::time::FrameTimer const& frame_timer, float const y_offset, float const alpha)
{
    ImGuiViewport const* imgui_viewport{ImGui::GetMainViewport()};
    float const          width{400.f};

    //------------------------------------------------------------------------
    // Set window size depends on dpi scale
    //------------------------------------------------------------------------
    ImGui::SetNextWindowSize(ImVec2{width * getDpiScaleFactor(), 0.f});
    ImGui::SetNextWindowBgAlpha(alpha);

    //------------------------------------------------------------------------
    // Make window movable out from main viewport
    //------------------------------------------------------------------------
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
    {
        ImGui::SetNextWindowPos(ImVec2{imgui_viewport->Pos.x, imgui_viewport->Pos.y + y_offset}, ImGuiCond_Always);
    }

    if (ImGui::Begin("Perfromance", nullptr,
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text(" ");
        //------------------------------------------------------------------------
        // Collect statistics data
        //------------------------------------------------------------------------
        static vshade::StackArray<float, 100U> frame_time_history;   // for 100 frames
        static vshade::StackArray<float, 20U>  blockage_history;     // for 20 frames
        static double                          time_accamulator{0.f};

        double constexpr ms{1'000'000.0};
        double constexpr s{1'000'000'000.0};
        double static cpu{0.0}, gpu{0.0}, ui{0.0}, fps{0.0};

        time_accamulator += frame_timer.getDeltaTimeInMilliseconds<double>();

        //------------------------------------------------------------------------
        // Set small pause to make graphs smoother
        //------------------------------------------------------------------------
        if (time_accamulator >= 20.f)
        {
            cpu         = frame_timer.getDeltaTimeInMilliseconds<double>();
            gpu         = vshade::render::SwapChain::instance().getPresentDeltaTime() / ms;
            ui          = vshade::render::Render::instance().getQueryResult("VulkanImGuiRenderer") / ms;
            fps         = 1.0 / (vshade::render::SwapChain::instance().getPresentDeltaTime() / s);

            frame_time_history.pushFront(fps);
            time_accamulator = 0.f;
        }
        //------------------------------------------------------------------------
        // Show FPS statistic
        //------------------------------------------------------------------------
        drawPlotWidget<float, 100U>("##FPS", frame_time_history, 4U,
                                    [=](auto min, auto max, int avg)
                                    {
                                        return std::string{"FPS: " + std::to_string(avg) + " (" + std::to_string(frame_size_.x) + "x" +
                                                           std::to_string(frame_size_.y) + ")"};
                                    });
       
        if (auto color_correction =
                vshade::LayerResourceManager::instance().get<editor::SceneRenderer::ColorCorrection>("Color correction"))
        {
            if (auto pipeline = vshade::render::Render::instance().getRegisterdPipline("Color correction"))
            {
                bool is_active{pipeline->isActive()};

                color_correction->time = frame_timer.getDeltaTimeInMilliseconds<float>();

                drawDragBlock(
                    "Color correction",
                    std::make_tuple(std::pair{"Color correction",
                                              [&]()
                                              {
                                                  if (ImGui::Checkbox("##Color correction", &is_active))
                                                      pipeline->setActive(is_active);
                                                  (!is_active) ? ImGui::BeginDisabled() : void();
                                              }},
                                    std::pair{"Exposure", [&]() { drawFloat("Exposure", &color_correction->exposure, 0.f, FLT_MAX, 1.f); }},
                                    std::pair{"Gamma", [&]() { drawFloat("Gamma", &color_correction->gamma, 0.f, FLT_MAX, 1.f); }},
                                    std::pair{"Contrast", [&]() { drawFloat("Contrast", &color_correction->contrast, 0.f, FLT_MAX, 1.f); }},
                                    std::pair{"Film Grain", [&]() { drawFloat("Grain ", &color_correction->grain_intensity, 0.f, FLT_MAX, 0.f); }},
                                    std::pair{"Aberation", [&]()
                                              {
                                                  drawFloat("Aberation ", &color_correction->chromatic_aberation_intensity, 0.f, FLT_MAX, 0.f);
                                                  (!is_active) ? ImGui::EndDisabled() : void();
                                              }}));
            }
        }
    }
    ImGui::End();
}

float editor::MainLayer::drawCollapseButtons(float const y_offset)
{
    ImGuiViewport const* imgui_view_port{ImGui::GetMainViewport()};

    ImGui::SetNextWindowViewport(imgui_view_port->ID);
    ImGui::SetNextWindowPos(ImVec2{imgui_view_port->Pos.x, imgui_view_port->Pos.y + y_offset}, ImGuiCond_Always);

    if (ImGui::Begin("ButtonPerformance", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        if (ImGui::ArrowButton("Open performance", (is_performace_opend_) ? ImGuiDir_Up : ImGuiDir_Down))
        {
            is_performace_opend_ = !is_performace_opend_;
        }
    }
    ImGui::End();

    ImGui::SetNextWindowViewport(imgui_view_port->ID);
    ImGui::SetNextWindowPos(
        ImVec2{imgui_view_port->Pos.x + imgui_view_port->Size.x - ImGui::GetFrameHeight() - 15.f, imgui_view_port->Pos.y + y_offset},
        ImGuiCond_Always);

    if (ImGui::Begin("ButtonSettings", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        if (ImGui::ArrowButton("Open settings", (is_setting_opend_) ? ImGuiDir_Up : ImGuiDir_Down))
        {
            is_setting_opend_ = !is_setting_opend_;
        }
    }
    ImGui::End();

    return ImGui::GetCursorScreenPos().y;
}

void editor::MainLayer::drawFunnyAnimation(vshade::time::FrameTimer const& frame_timer)
{
    //------------------------------------------------------------------------
    // No comments :|
    //------------------------------------------------------------------------
    static bool is_li_show{true};
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);

    if (is_li_show &&
        ImGui::Begin("Texture", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
    {
        static double time_accamulator{0.0};
        time_accamulator += frame_timer.getDeltaTimeInSeconds<double>() / 2.0;
        double const alpha{1.0 - time_accamulator};
        //drawTexture(texture, ImGui::GetMainViewport()->Size, {0.f, 0.f, 0.f, 0.f}, 1.0f - time_accamulator);

        if (alpha < 0.0)
        {
            is_li_show = false;
        }

        ImGui::End();
    }
}

void editor::MainLayer::updateCamera(vshade::time::FrameTimer const& frame_timer)
{
    float const delta_time{frame_timer.getDeltaTimeInMilliseconds<float>()};

    //------------------------------------------------------------------------
    // Gamepad camera control
    //------------------------------------------------------------------------
    {
        //------------------------------------------------------------------------
        // Camera movement
        //------------------------------------------------------------------------
        {
            float const left_y{vshade::event::Input::getGamepadAxies(vshade::event::gamepad_axis::_GAMEPAD_AXIS_LEFT_Y_)};
            float const left_x{vshade::event::Input::getGamepadAxies(vshade::event::gamepad_axis::_GAMEPAD_AXIS_LEFT_X_)};

            if (std::abs(left_y) > 0.1f)
            {
                main_camera_->moveForward(-left_y * camera_movement_speed_ * delta_time);
            }
            if (std::abs(left_x) > 0.1f)
            {
                main_camera_->moveLeft(-left_x * camera_movement_speed_ * delta_time);
            }
        }

        //------------------------------------------------------------------------
        // Camera rotation
        //------------------------------------------------------------------------
        {
            float const right_y{vshade::event::Input::getGamepadAxies(vshade::event::gamepad_axis::_GAMEPAD_AXIS_RIGHT_Y_)};
            float const right_x{vshade::event::Input::getGamepadAxies(vshade::event::gamepad_axis::_GAMEPAD_AXIS_LEFT_TRIGGER_)};
            main_camera_->rotate(glm::vec3{std::abs(right_y) > 0.1f ? right_y : 0.f, std::abs(right_x) > 0.1f ? right_x : 0.f, 0.0f} *
                                 camera_rotation_speed_ / 100.f * delta_time);
        }
    }

    //------------------------------------------------------------------------
    // Mouse camera control
    //------------------------------------------------------------------------
    if (vshade::event::Input::isMouseButtonPressed(vshade::event::mouse_button::_RIGTH_))
    {
        vshade::event::Input::showMouseCursor(false);
        //------------------------------------------------------------------------
        // Camera movement
        //------------------------------------------------------------------------
        {
            if (vshade::event::Input::isKeyPressed(vshade::event::key::_KEY_W_))
            {
                main_camera_->moveForward(camera_movement_speed_ * delta_time);
            }
            if (vshade::event::Input::isKeyPressed(vshade::event::key::_KEY_S_))
            {
                main_camera_->moveBackward(camera_movement_speed_ * delta_time);
            }
            if (vshade::event::Input::isKeyPressed(vshade::event::key::_KEY_A_))
            {
                main_camera_->moveLeft(camera_movement_speed_ * delta_time);
            }
            if (vshade::event::Input::isKeyPressed(vshade::event::key::_KEY_D_))
            {
                main_camera_->moveRight(camera_movement_speed_ * delta_time);
            }
        }
        //------------------------------------------------------------------------
        // Camera rotation
        //------------------------------------------------------------------------
        {
            if (vshade::event::Input::isMouseButtonPressed(vshade::event::mouse_button::_RIGTH_))
            {
                glm::ivec2 const screen_center{frame_size_.x / 2U, frame_size_.y / 2U};
                glm::ivec2       mouse_position{vshade::event::Input::getMousePosition()};
                glm::ivec2 const mouse_position_screen{mouse_position.x - screen_center.x, mouse_position.y - screen_center.y};

                main_camera_->rotate(glm::vec3{mouse_position_screen, 0.0f} * camera_rotation_speed_ / 1000.f);

                vshade::event::Input::setMousePosition(screen_center.x, screen_center.y);
            }
        }
    }
    else
    {
        vshade::event::Input::showMouseCursor(true);
    }
}