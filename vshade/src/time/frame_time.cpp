#include "engine/core/time/frame_time.h"
#include <glfw/include/GLFW/glfw3.h>

vshade::time::FrameTimer::FrameTimer()
{
}
vshade::time::FrameTimer::FrameTimer(double time) : time_now_{time}, time_delta_{time}
{
}
void vshade::time::FrameTimer::update()
{
    time_last_  = time_now_;
    time_now_   = glfwGetTime();
    time_delta_ = time_now_ - time_last_;
}