#ifndef ENGINE_CORE_TIME_FRAME_TIME_H
#define ENGINE_CORE_TIME_FRAME_TIME_H

#include <engine/config/vshade_api.h>
#include <iostream>

namespace vshade
{
namespace time
{

class VSHADE_API FrameTimer final
{
public:
    explicit FrameTimer();
    explicit FrameTimer(double time);
    ~FrameTimer() = default;

    void                                                                           update();
    template <typename T, typename = std::enable_if_t<std::is_scalar<T>::value>> T getDeltaTimeInSeconds() const
    {
        return static_cast<T>(time_delta_);
    }
    template <typename T, typename = std::enable_if_t<std::is_scalar<T>::value>> T getDeltaTimeInMilliseconds() const
    {
        return static_cast<T>(time_delta_ * 1000.0);
    }
    template <typename T, typename = std::enable_if_t<std::is_scalar<T>::value>> T getTimeInSeconds() const
    {
        return static_cast<T>(time_now_);
    }
    template <typename T, typename = std::enable_if_t<std::is_scalar<T>::value>> T getTimeInMilliseconds() const
    {
        return static_cast<T>(time_now_ * 1000.0);
    }

    template <typename T, typename = std::enable_if_t<std::is_scalar<T>::value>> T getFPS() const
    {
        return static_cast<T>(1.0 / time_delta_);
    }
    operator double() const
    {
        return getDeltaTimeInMilliseconds<double>();
    }

private:
    double time_last_{0.0}, time_now_{0.0}, time_delta_{0.0};
};
} // namespace time

} // namespace vshade

#endif // ENGINE_CORE_TIME_FRAME_TIME_H