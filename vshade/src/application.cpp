#include "engine/core/application/application.h"
#include <engine/core/events/event_manager.h>

vshade::Application::Application(std::string const& root_directory) : root_directory_{root_directory}
{
}

void vshade::Application::initialize()
{
    Logger::create<Logger>();

    file::FileManager::create<file::FileManager>(root_directory_);

    VSHADE_CORE_INFO("Creating application : {}", vshade::System::instance().getConfiguration().application_name)

    thread::ThreadPool::create<thread::ThreadPool>();

    render::Render::create(vshade::System::instance().getConfiguration().render_api, vshade::System::instance().getConfiguration().frames_in_flight);

    event::EventManager::create();

    LayerManager::create<LayerManager>();

    onCreate();
}
void vshade::Application::launch()
{
    whileRunning();
}

void vshade::Application::whileRunning()
{
    while (!is_quit_requested_ && !window::Window::instance().isShouldClose())
    {
        frame_timer_.update();

        //------------------------------------------------------------------------
        // Process events
        //------------------------------------------------------------------------
        window::Window::instance().processEvents();
        event::EventManager::instance().processEvents();

        //------------------------------------------------------------------------
        // Start new preparation frame, it's CPU blocking operation
        //------------------------------------------------------------------------
        std::uint32_t const frame_index{render::Render::instance().beginPrepareNewFrame()};
        {
            //------------------------------------------------------------------------
            // Process all layers and thier renderers
            //------------------------------------------------------------------------
            for (std::shared_ptr<Layer> layer : LayerManager::instance())
            {
                std::shared_ptr<Scene> layer_scene{LayerManager::instance().getScene(layer)};
                layer->onUpdate(layer_scene, frame_timer_);

                if (std::shared_ptr<render::SceneRenderer> renderer = layer->getRenderer())
                {
                    renderer->onUpdate(layer_scene, frame_timer_, frame_index);
                    renderer->onRenderBegin(frame_timer_, frame_index);
                    layer->onRender(layer_scene, frame_timer_);
                    renderer->onRender(layer_scene, frame_timer_, frame_index);
                    renderer->onRenderEnd(frame_timer_, frame_index);
                }
            }
        }

        //------------------------------------------------------------------------
        // End new preparation frame, it's CPU blocking operation
        //------------------------------------------------------------------------
        render::Render::instance().endPrepareNewFrame();
        //------------------------------------------------------------------------
        // Process render async or sync, depends on System::Configuration
        //------------------------------------------------------------------------
        render::Render::instance().processRenderThread();
    }

    //------------------------------------------------------------------------
    // Join render thread before exiting
    //------------------------------------------------------------------------
    render::Render::instance().closeRenderThread();
}

void vshade::Application::onEvent_(std::shared_ptr<event::Event> const event)
{
    //------------------------------------------------------------------------
    // Dispatch event callbacks
    //------------------------------------------------------------------------
    event::EventManager::instance().dispatchEvent<event::WindowClose>(
        [&](std::shared_ptr<event::WindowClose> const event)
        {
            Application::instance().quitRequest();

            return onEvent(event) | window::Window::instance().onCloseEvent(event);
        },
        event);
    event::EventManager::instance().dispatchEvent<event::WindowResize>(
        [&](std::shared_ptr<event::WindowResize> const event)
        { return onEvent(event) | window::Window::instance().onResizeEvent(event) | render::SwapChain::instance().onResizeEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::KeyPressedEvent>([&](std::shared_ptr<event::KeyPressedEvent> const event)
                                                                          { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::KeyReleasedEvent>([&](std::shared_ptr<event::KeyReleasedEvent> const event)
                                                                           { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::KeyTypedEvent>([&](std::shared_ptr<event::KeyTypedEvent> const event)
                                                                        { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::MouseMovedEvent>([&](std::shared_ptr<event::MouseMovedEvent> const event)
                                                                          { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::MouseButtonPressedEvent>([&](std::shared_ptr<event::MouseButtonPressedEvent> const event)
                                                                                  { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::MouseButtonReleasedEvent>([&](std::shared_ptr<event::MouseButtonReleasedEvent> const event)
                                                                                   { return onEvent(event); }, event);
    event::EventManager::instance().dispatchEvent<event::MouseScrolledEvent>([&](std::shared_ptr<event::MouseScrolledEvent> const event)
                                                                             { return onEvent(event); }, event);

    //------------------------------------------------------------------------
    // Push event into the layer
    //------------------------------------------------------------------------
    for (std::shared_ptr<Layer> layer : LayerManager::instance())
    {
        layer->onEvent(LayerManager::instance().getScene(layer), event, frame_timer_);
    }
}

void vshade::Application::quitRequest()
{
    is_quit_requested_ = true;
}

void vshade::Application::terminate()
{
    onDestroy();

    LayerManager::destroy();

    event::EventManager::destroy();

    window::Window::destroy();

    render::Render::destroy();

    VSHADE_CORE_INFO("Destroying application : {}", vshade::System::instance().getConfiguration().application_name)

    Logger::destroy();
}
