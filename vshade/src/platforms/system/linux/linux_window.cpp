#include "engine/platforms/system/linux/linux_window.h"
#include <engine/core/events/event_manager.h>
#include <vulkan/vulkan.h>

vshade::window::LinuxWindow::LinuxWindow(Properties const& properties) : Window(properties)
{
}

void vshade::window::LinuxWindow::initWindowEvents()
{
    //------------------------------------------------------------------------
    // Window Close Event
    //------------------------------------------------------------------------
    glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(window_ptr_),
                               [](GLFWwindow* window)
                               {
                                   System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                                   static_cast<void>(data);
                                   event::EventManager::pushEvent<event::WindowClose>();
                               });

    //------------------------------------------------------------------------
    // Window Resize Event
    //------------------------------------------------------------------------
    glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(window_ptr_),
                              [](GLFWwindow* window, int width, int height)
                              {
                                  System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                                  static_cast<void>(data);
                                  event::EventManager::pushEvent<event::WindowResize>(static_cast<std::uint32_t>(width),
                                                                                      static_cast<std::uint32_t>(height));
                              });

    //------------------------------------------------------------------------
    // Window Minimize Event
    //------------------------------------------------------------------------
    // Callback to handle window minimize events.
    // glfwSetWindowIconifyCallback(static_cast<GLFWwindow*>(window_ptr_), [](GLFWwindow* window, int iconified)
    //                              { reinterpret_cast<UserData*>(glfwGetWindowUserPointer(window))->IsMinimaized = iconified; });

    //------------------------------------------------------------------------
    // Keyboard Events
    //------------------------------------------------------------------------
    // Callback to handle keyboard input events.
    glfwSetKeyCallback(static_cast<GLFWwindow*>(window_ptr_),
                       [](GLFWwindow* window, int key, int scancode, int action, int mods)
                       {
                           System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                           static_cast<void>(data);
                           switch (action)
                           {
                           case GLFW_PRESS:
                           {
                               event::KeyPressedEvent event{static_cast<event::KeyCode>(key), 0U};
                               event::EventManager::pushEvent<event::KeyPressedEvent>(event);
                               break;
                           }
                           case GLFW_RELEASE:
                           {
                               event::KeyReleasedEvent event{static_cast<event::KeyCode>(key)};
                               event::EventManager::pushEvent<event::KeyReleasedEvent>(event);
                               break;
                           }
                           case GLFW_REPEAT:
                           {
                               event::KeyPressedEvent event{static_cast<event::KeyCode>(key), 1U};
                               event::EventManager::pushEvent<event::KeyPressedEvent>(event);
                               break;
                           }
                           }
                       });
    //------------------------------------------------------------------------
    // Keyboard Typing Events
    //------------------------------------------------------------------------
    // Callback to handle character input events.
    glfwSetCharCallback(static_cast<GLFWwindow*>(window_ptr_),
                        [](GLFWwindow* window, unsigned int keycode)
                        {
                            System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                            static_cast<void>(data);
                            event::EventManager::pushEvent<event::KeyTypedEvent>(keycode);
                        });

    //------------------------------------------------------------------------
    // Mouse Button Events
    //------------------------------------------------------------------------
    // Callback to handle mouse button events.
    glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(window_ptr_),
                               [](GLFWwindow* window, int button, int action, int mods)
                               {
                                   System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                                   static_cast<void>(data);
                                   switch (action)
                                   {
                                   case GLFW_PRESS:
                                   {
                                       event::MouseButtonPressedEvent event{static_cast<event::MouseButtonCode>(button)};
                                       event::EventManager::pushEvent<event::MouseButtonPressedEvent>(event);
                                       break;
                                   }
                                   case GLFW_RELEASE:
                                   {
                                       event::MouseButtonReleasedEvent event{static_cast<event::MouseButtonCode>(button)};
                                       event::EventManager::pushEvent<event::MouseButtonReleasedEvent>(event);
                                       break;
                                   }
                                   }
                               });

    //------------------------------------------------------------------------
    // Mouse Scroll Events
    //------------------------------------------------------------------------
    // Callback to handle mouse scroll events.
    glfwSetScrollCallback(static_cast<GLFWwindow*>(window_ptr_),
                          [](GLFWwindow* window, double x_offset, double y_offset)
                          {
                              System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                              static_cast<void>(data);
                              event::MouseScrolledEvent event{x_offset, y_offset};
                              event::EventManager::pushEvent<event::MouseScrolledEvent>(event);
                          });

    //------------------------------------------------------------------------
    // Mouse Movement Events
    //------------------------------------------------------------------------
    // Callback to handle mouse movement events.
    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(window_ptr_),
                             [](GLFWwindow* window, double x_pos, double y_pos)
                             {
                                 System::UserData* data{static_cast<System::UserData*>(glfwGetWindowUserPointer(window))};
                                 static_cast<void>(data);
                                 event::MouseMovedEvent event{x_pos, y_pos};
                                 event::EventManager::pushEvent<event::MouseMovedEvent>(event);
                             });
}

void vshade::window::LinuxWindow::gLFWErrorCallback(int error, char const* description)
{
    //-------------------------------------------------------------------------
    // Error Handling
    //-------------------------------------------------------------------------
    // Callback function for handling GLFW errors.
    VSHADE_CORE_WARNING("GLFW Error ({0}): {1}", error, description);
}

void vshade::window::LinuxWindow::processEvents()
{
    glfwPollEvents();
}