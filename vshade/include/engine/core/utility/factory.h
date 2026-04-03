#ifndef ENGINE_CORE_UTILITY_FACTORY_H
#define ENGINE_CORE_UTILITY_FACTORY_H

#include <cassert>
#include <iostream>
#include <memory>

#include <engine/config/vshade_api.h>
#include <engine/core/memory/allocation.h>

namespace vshade
{
namespace utility
{
template <typename T> class CRTPFactory
{
public:
    CRTPFactory(CRTPFactory const&)             = delete;
    CRTPFactory& operator=(CRTPFactory const&)  = delete;
    CRTPFactory(CRTPFactory const&&)            = delete;
    CRTPFactory& operator=(CRTPFactory const&&) = delete;

    template <typename U, typename... Args> static std::shared_ptr<U> create(Args&&... args)
    {
        static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
        return std::shared_ptr<U>(VS_NEW U{std::forward<Args>(args)...});
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
    explicit CRTPFactory() = default;
    virtual ~CRTPFactory() = default;
};
} // namespace utility
} // namespace vshade

#endif ENGINE_CORE_UTILITY_FACTORY_H