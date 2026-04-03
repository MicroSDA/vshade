#ifndef ENGINE_CORE_UTILITY_SINGLETON_H
#define ENGINE_CORE_UTILITY_SINGLETON_H

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

#include <engine/config/vshade_api.h>
#include <engine/core/memory/allocation.h>
//#include <source_location>
#include <iostream>


namespace vshade
{
namespace utility
{
template <typename T> class CRTPSingleton
{
public:
    CRTPSingleton(CRTPSingleton const&)             = delete;
    CRTPSingleton& operator=(CRTPSingleton const&)  = delete;
    CRTPSingleton(CRTPSingleton const&&)            = delete;
    CRTPSingleton& operator=(CRTPSingleton const&&) = delete;

    static T& instance()
    {
        assert(initialized_);
        assert(instance_ptr_ != nullptr);

        return *instance_ptr_;
    }

    template <typename U, typename... Args> static T& create(Args&&... args)
    {
        if (!initialized_)
        {
            std::call_once(init_flag_, [&]() { instance_ptr_.reset(VS_NEW U{std::forward<Args>(args)...}); });
            initialized_ = true;
        }
        return *instance_ptr_;
    }
    
    static void destroy()
    {
        instance_ptr_.reset(nullptr);
        initialized_ = false;
    }

    template <typename U> U& as()
    {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        return static_cast<U&>(*this);
    }

    template <typename U> U const& as() const
    {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        return static_cast<U const&>(*this);
    }

protected:
    explicit CRTPSingleton() = default;
    virtual ~CRTPSingleton() = default;

private:
    static inline std::unique_ptr<T> instance_ptr_{nullptr};
    static inline std::once_flag     init_flag_;
    static inline bool               initialized_{false};
};
} // namespace utility
} // namespace vshade

#endif ENGINE_CORE_UTILITY_SINGLETON_H