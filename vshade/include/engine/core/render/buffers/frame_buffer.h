#ifndef ENGINE_CORE_RENDER_FRAME_BUFFER_H
#define ENGINE_CORE_RENDER_FRAME_BUFFER_H
#include <engine/core/render/image.h>
#include <engine/core/render/render_command_buffer.h>
#include <engine/core/render/texture.h>
#include <engine/core/utility/factory.h>
#include <glm/glm/glm.hpp>
#include <initializer_list>
#include <vector>

namespace vshade
{
namespace render
{
class VSHADE_API FrameBuffer : public utility::CRTPFactory<FrameBuffer>
{
    friend class utility::CRTPFactory<FrameBuffer>;

    // Hide functions
    using utility::CRTPFactory<FrameBuffer>::create;

public:
    struct Textures
    {
        Textures() = default;
        Image::Format format{Image::Format::_UNDEFINED_};
    };

    struct Attachments
    {
        Attachments() = default;
        Attachments(std::initializer_list<Image::Specification> const& attachments) : texture_attachments{attachments}
        {
        }
        std::vector<Image::Specification> texture_attachments;
    };
    struct Specification
    {
        Specification() = default;
        std::uint32_t width{0U}, height{0U};
        Attachments   attachments;
        glm::vec4     clear_color{0.f, 0.f, 0.f, 1.f};
        float         depth_clear_value{1.f};
    };

public:
    virtual ~FrameBuffer()                       = default;
    FrameBuffer(FrameBuffer const&)              = delete;
    FrameBuffer(FrameBuffer&&)                   = delete;
    FrameBuffer& operator=(FrameBuffer const&) & = delete;
    FrameBuffer& operator=(FrameBuffer&&) &      = delete;

    virtual std::shared_ptr<Texture2D> getColorAttachment(std::uint32_t index = 0U)                                                         = 0;
    virtual std::shared_ptr<Texture2D> getDepthAttachment(std::uint32_t index = 0U)                                                         = 0;
    virtual void                       resize(std::uint32_t const width, std::uint32_t const height)                                        = 0;
    virtual void clearAttachmentsRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index = 0U) = 0;

    Specification const& getSpecification() const
    {
        return specification_;
    }
    Specification& getSpecification()
    {
        return specification_;
    }
    std::uint32_t getWidth() const
    {
        return specification_.width;
    }
    std::uint32_t getHeight() const
    {
        return specification_.height;
    }
    static std::shared_ptr<FrameBuffer> create(Specification const& specification);
    static std::shared_ptr<FrameBuffer> create(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const& images);

protected:
    FrameBuffer(Specification const& specification);
    FrameBuffer(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const& images);

    Specification specification_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_FRAME_BUFFER_H