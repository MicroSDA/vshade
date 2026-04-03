#ifndef ENGINE_CONFIG_SYSTEM_SYSTEM_H
#define ENGINE_CONFIG_SYSTEM_SYSTEM_H

#include <engine/core/render/render_api.h>
#include <engine/core/utility/singleton.h>
namespace vshade
{
class VSHADE_API System final : public utility::CRTPSingleton<System>
{
    friend class utility::CRTPSingleton<System>;

public:
    enum FramesInFlight : std::uint32_t
    {
        _SEQUENTIAL_       = 1U,
        _DOUBLE_BUFFERING_ = 2U,
        _TRIPLE_BUFFERING_ = 3U
    };

    enum GpuType : std::uint8_t
    {
        _INTEGRATED_ = 0U,
        _DESCRATE_   = 1U,
        _ANY_GPU_    = 2U
    };

    struct Configuration
    {
        std::string    application_name{"Vshade"};
        FramesInFlight frames_in_flight{FramesInFlight::_SEQUENTIAL_};
        GpuType        gpu_type{GpuType::_ANY_GPU_};
        render::API    render_api{render::API::_VULKAN_};
    };

    struct UserData
    {
    };

public:
    virtual ~System()                  = default;
    System(System const&)              = delete;
    System(System&&)                   = delete;
    System& operator=(System const&) & = delete;
    System& operator=(System&&) &      = delete;

    void                 setConfiguration(Configuration const& config);
    Configuration const& getConfiguration() const
    {
        return configuration_;
    }

    void setUserData(UserData const& data)
    {
        user_data_ = data;
    }
    UserData& getUserData()
    {
        return user_data_;
    }

    std::string getCurentRenderAPIAsString()
    {
        switch (configuration_.render_api)
        {
        case render::API::_VULKAN_:
            return "vulkan";
        case render::API::_OPEN_GL_:
            return "opengl";
        }
    }

protected:
    explicit System() = default;
    Configuration configuration_;
    UserData      user_data_;
    bool          is_set_{false};
};
} // namespace vshade

#endif // ENGINE_CONFIG_SYSTEM_SYSTEM_H