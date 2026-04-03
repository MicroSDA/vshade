#include "application/editor.h"
#include <glm/gtc/type_ptr.hpp>

void vshade::createApplication(std::string const& root_directory)
{
    //------------------------------------------------------------------------
    // Set configuration
    //------------------------------------------------------------------------
    vshade::System::instance().setConfiguration(vshade::System::Configuration{"V-shade", vshade::System::FramesInFlight::_SEQUENTIAL_,
                                                                              vshade::System::GpuType::_DESCRATE_, vshade::render::API::_VULKAN_

    });

    Application::create<editor::Application>(root_directory);
}

void vshade::destroyApplication()
{
    Application::destroy();
}

editor::Application::Application(std::string const& root_directory_) : vshade::Application(root_directory_)
{
}
void editor::Application::onCreate()
{
    //------------------------------------------------------------------------
    // Create window
    //------------------------------------------------------------------------
    vshade::window::Window::create(vshade::window::Properties{"50 6F 69 6E 74 20 63 6C 6F 75 64", 1920, 1080, false, false});

    //------------------------------------------------------------------------
    // Create layer
    //------------------------------------------------------------------------
    std::shared_ptr<MainLayer>     main_layer{MainLayer::create<MainLayer>()};
    std::shared_ptr<vshade::Layer> layer{
        vshade::Layer::create<vshade::Layer>("Layer", SceneRenderer::create<SceneRenderer>(main_layer->getRenderer()->getMainFrameBuffer()))};

    //------------------------------------------------------------------------
    // Push layers into stack, main layer aka ImGuiLayer should be on top
    //------------------------------------------------------------------------
    vshade::LayerManager::instance().pushLayer(layer);
    vshade::LayerManager::instance().pushOverlay(main_layer);
}

void editor::Application::onDestroy()
{
}

void editor::Application::onUpdate()
{
}

bool editor::Application::onEvent(std::shared_ptr<vshade::event::Event> const event)
{
    return false;
}