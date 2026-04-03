#ifndef ENGINE_PLATFORMS_SYSTEM_LINUX_WINDOW_H
#define ENGINE_PLATFORMS_SYSTEM_LINUX_WINDOW_H

#include <engine/core/logs/loger.h>
#include <engine/core/window/window.h>
#include <glfw/include/GLFW/glfw3.h>

namespace vshade
{
namespace window
{
class LinuxWindow : public Window
{
public:
    explicit LinuxWindow(Properties const& properties);
    virtual ~LinuxWindow()                       = default;
    LinuxWindow(LinuxWindow const&)              = delete;
    LinuxWindow(LinuxWindow&&)                   = delete;
    LinuxWindow& operator=(LinuxWindow const&) & = delete;
    LinuxWindow& operator=(LinuxWindow&&) &      = delete;

    virtual void  processEvents() override;
    virtual void  initWindowEvents() override;
    virtual void* getWindowNativeHanlder() override
    {
        return window_ptr_;
    }

    virtual void setWindowOpacity(float const alpha, void* window) override
    {
        glfwSetWindowOpacity((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_, alpha);
    }

    virtual void setWindowPosition(glm::ivec2 const& position, void* window = nullptr) override
    {
        glfwSetWindowPos((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_, position.x, position.y);
    }

    virtual void moveWindow(glm::ivec2 const& position, void* window = nullptr) override
    {
        glm::ivec2 const current_position{getWindowPosition(window)};
        setWindowPosition(current_position + position, window);
    }

    virtual glm::ivec2 getWindowPosition(void* window = nullptr) const override
    {
        glm::ivec2 position;
        glfwGetWindowPos((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_, &position.x, &position.y);
        return position;
    }

    virtual glm::ivec2 getCursorPosition(void* window = nullptr) const
    {
        double xpos{0.0}, ypos{0.0};
        glfwGetCursorPos((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_, &xpos, &ypos);
        return {static_cast<int>(std::floor(xpos)), static_cast<int>(std::floor(ypos))};
    }

    virtual void minimazeWindow(void* window) override
    {
        glfwIconifyWindow((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_);
    }

    virtual void maximizeWindow(void* window) override
    {
        glfwMaximizeWindow((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_);
    }

    virtual void closeWindow(void* window) override
    {
        glfwSetWindowShouldClose((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_, true);
    }

    virtual bool isShouldClose(void* window) const override
    {
        return glfwWindowShouldClose((window) ? reinterpret_cast<GLFWwindow*>(window) : window_ptr_);
    }

protected:
    GLFWwindow* window_ptr_{nullptr};
    static void gLFWErrorCallback(int error, char const* description);
};
} // namespace window
} // namespace vshade
#endif // ENGINE_PLATFORMS_SYSTEM_LINUX_WINDOW_H