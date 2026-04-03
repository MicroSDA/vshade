#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_H
#include <engine/core/render/render.h>
#include <engine/platforms/render/vulkan/descriptors/vulkan_descriptor.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <engine/platforms/render/vulkan/vulkan_render_pipeline.h>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
/// @brief Vulkan-specific implementation of the Render interface.
class VulkanRender final : public Render
{
    friend class utility::CRTPSingleton<Render>;

public:
    virtual ~VulkanRender();
    VulkanRender(VulkanRender const&)              = delete;
    VulkanRender(VulkanRender&&)                   = delete;
    VulkanRender& operator=(VulkanRender const&) & = delete;
    VulkanRender& operator=(VulkanRender&&) &      = delete;

public:
    /// Begin render pass for the specified pipeline.
    ///
    /// @param render_command_buffer  Command buffer to record rendering commands into.
    /// @param render_pipeline        Pipeline to be used for rendering.
    /// @param is_clear               Whether to clear framebuffers before rendering.
    /// @param clear_count            Number of clear attachments (color/depth).
    virtual void beginRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<RenderPipeline> const render_pipeline,
                             bool const is_clear, std::uint32_t const clear_count) override;
    /// End the current render pass.
    ///
    /// @param render_command_buffer  Command buffer used in beginRender().
    virtual void endRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer) override;
    /// Call a draw call for the specified vertex/index buffers.
    ///
    /// @param render_command_buffer  Command buffer for recording commands.
    /// @param vertex_buffer          Vertex buffer to draw.
    /// @param index_buffer           Index buffer to draw.
    /// @param instance_count         Number of instances to render.
    virtual void drawRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<VertexBuffer const> const vertex_buffer,
                        std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const frame_index,
                        std::uint32_t const instance_count) override;
    /// Call a draw call for the specified vertex/index buffers.
    ///
    /// @param render_command_buffer  Command buffer for recording commands.
    /// @param vertex_buffer          Vertex buffer to draw.
    /// @param index_buffer           Index buffer to draw.
    /// @param instance_count         Number of instances to render.
    virtual void draw(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<VertexBuffer const> const vertex_buffer,
                      std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const instance_count) override;
    /// Set an image memory barrier for a texture.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param texture                Texture to set the barrier on.
    /// @param src_stage              Source pipeline stage.
    /// @param dst_stage              Destination pipeline stage.
    /// @param src_access             Source access mask.
    /// @param dst_accces             Destination access mask.
    /// @param mip                    Mipmap level to transition.
    virtual void setBarrier(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<Texture2D const> const texture,
                            Pipeline::Stage const src_stage, Pipeline::Stage const dst_stage, Pipeline::Access const src_access,
                            Pipeline::Access const dst_accces, std::uint32_t const mip = 0U) override;
    /// Set a buffer memory barrier for a storage buffer.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param storage_buffer         Storage buffer to set the barrier on.
    /// @param src_stage              Source pipeline stage.
    /// @param dst_stage              Destination pipeline stage.
    /// @param src_access             Source access mask.
    /// @param dst_accces             Destination access mask.
    virtual void setBarrier(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                            std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Stage const src_stage,
                            Pipeline::Stage const dst_stage, Pipeline::Access const src_access, Pipeline::Access const dst_accces) override;
    /// Begin timestamp query in render thread.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_index            Frame index in flight.
    /// @param name                   Name of the timestamp query.
    virtual void beginTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                  std::string const& name) override;
    /// @brief End timestamp query in render thread.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_index            Frame index in flight.
    /// @param name                   Name of the timestamp query.
    virtual void endTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                std::string const& name) override;
    /// @brief Retrieve GPU timestamp query result for a given frame.
    ///
    /// @param name         Query name.
    /// @param frame_index  Frame index in flight.
    /// @return Query result in milliseconds.
    virtual double getQueryResult(std::string const& name, std::uint32_t const frame_index) const override;
    /// @brief Clear frame buffer color and depth attachments
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_buffer  Frame buffer to clear
    virtual void clearFrameBufferAttachments(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                             std::shared_ptr<FrameBuffer> const         frame_buffer) override;
    /// Retrieve latest GPU timestamp query result.
    ///
    /// @param name  Query name.
    /// @return Query result in milliseconds.
    virtual double getQueryResult(std::string const& name) const override;
    /// @brief Get the global Vulkan descriptor set layout.
    ///
    /// This layout defines the structure of the descriptor set used for global resources
    /// such as camera buffers, material buffers, and global textures across all frames.
    ///
    /// @return const std::unique_ptr<descriptor::VulkanDescriptorSetLayout>&
    ///         Reference to the unique pointer holding the global descriptor set layout.
    std::unique_ptr<descriptor::VulkanDescriptorSetLayout> const& getGlobalDescriptorSetLayout()
    {
        return vulkan_global_scene_data_.vulkan_descriptor_set_layout_;
    }
    /// @brief Get the global push constant ranges for shaders.
    ///
    /// Push constants are small blocks of data sent directly to shaders without
    /// using descriptor sets, allowing fast updates for frequently changing data.
    ///
    /// @return const std::vector<VkPushConstantRange>&
    ///         Reference to the vector containing the push constant ranges.
    std::vector<VkPushConstantRange> const& globalPushConstantRanges()
    {
        return vulkan_global_scene_data_.vk_push_constant_ranges_;
    }

    /// @brief Retrieve the Vulkan descriptor set for global resources for a specific frame.
    ///
    /// Uses the VulkanDescriptorManager to get or allocate a descriptor set
    /// based on the global descriptor set layout and bound resources for the given frame index.
    ///
    /// @param frame_index Index of the frame (used for multi-frame rendering / frames in flight).
    /// @return std::shared_ptr<descriptor::VulkanDescriptorSet>
    ///       Shared pointer to the Vulkan descriptor set for the specified frame.
    std::shared_ptr<descriptor::VulkanDescriptorSet> const getGlobalDescriptorSet(std::uint32_t const frame_index)
    {
        return descriptor::VulkanDescriptorManager::instance().reciveDescriptor(*vulkan_global_scene_data_.vulkan_descriptor_set_layout_,
                                                                                vulkan_global_scene_data_.bindings_, frame_index);
    }
    /// @brief Retrieve the Vulkan push constan ranges set for global.
    ///
    /// @return std::vector<VkPushConstantRange> array of push constant ranges
    std::vector<VkPushConstantRange> const& getGlobalVkPushConstantRanges() const
    {
        return vulkan_global_scene_data_.vk_push_constant_ranges_;
    }

protected:
    /// Constructor for the renderer.
    ///
    /// @param[in] api               Rendering API.
    /// @param[in] frames_in_flight  Number of frames in flight supported.
    explicit VulkanRender(const API api, std::uint32_t const frames_in_flight);
    /// Collect GPU query results for the given frame index.
    ///
    /// @param[in] frame_index  Frame index in flight.
    virtual void queryResults(std::uint32_t const frame_index) override;

private:
    /// @brief Global render scene data
    struct VulkanGlobalSceneData
    {
        descriptor::VulkanDescriptorBindings                   bindings_;
        std::unique_ptr<descriptor::VulkanDescriptorSetLayout> vulkan_descriptor_set_layout_;
        std::vector<VkPushConstantRange>                       vk_push_constant_ranges_;
    };

    VulkanGlobalSceneData                                                                  vulkan_global_scene_data_;
    std::vector<ankerl::unordered_dense::map<std::string, std::pair<VkQueryPool, double>>> query_pools_;
};
} // namespace render

} // namespace  vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_H