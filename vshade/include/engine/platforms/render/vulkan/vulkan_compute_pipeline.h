#ifndef ENGINE_PLATFORM_RENDER_VULKAN_COMPUTE_PIPELINE_H
#define ENGINE_PLATFORM_RENDER_VULKAN_COMPUTE_PIPELINE_H

#include <engine/core/render/pipeline.h>
#include <engine/platforms/render/vulkan/buffers/vulkan_storage_buffer.h>
#include <engine/platforms/render/vulkan/descriptors/vulkan_descriptor.h>
#include <engine/platforms/render/vulkan/vulkan_frame_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_image.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <engine/platforms/render/vulkan/vulkan_shader.h>
#include <engine/platforms/render/vulkan/vulkan_texture.h>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
/// @brief Vulkan-specific implementation of the ComputePipeline interface.
///
/// Manages Vulkan pipeline creation, resource binding, and pipeline state management.
/// Provides concrete implementations for attaching buffers, textures, uniforms,
/// and command buffer interaction specific to Vulkan API.
///
/// Responsible for handling Vulkan descriptor sets and pipeline layouts.
class VSHADE_API VulkanComputePipeline : public ComputePipeline
{
    friend class utility::CRTPFactory<ComputePipeline>;

public:
    virtual ~VulkanComputePipeline();
    VulkanComputePipeline(VulkanComputePipeline const&)              = delete;
    VulkanComputePipeline(VulkanComputePipeline&&)                   = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline const&) & = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) &      = delete;

protected:
    /// @brief Bind this pipeline to the given render command buffer for a specific frame.
    ///
    /// This will bind the Vulkan pipeline and descriptor sets for drawing commands.
    ///
    /// @param[in] render_command_buffer The command buffer to bind to.
    /// @param[in] frame_index The current frame index (used for double/triple buffering).
    virtual void bind(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index) override;
    /// @brief Dispatch compute shader workgroups.
    ///
    /// @param render_command_buffer Command buffer to record commands.
    /// @param group_count_x         Number of workgroups in X dimension.
    /// @param group_count_y         Number of workgroups in Y dimension.
    /// @param group_count_z         Number of workgroups in Z dimension.
    virtual void dispatch(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const group_count_x,
                          std::uint32_t const group_count_y, std::uint32_t const group_count_z) override;

    /// Attach storage buffer to the current pipline.
    ///
    /// @param[in] storage_buffer   Storage buffer
    /// @param[in] set              Pipeline set
    /// @param[in] offset           Offset to the first element witing the buffer (In case we want to get accsess to specific buffer's range)
    virtual void setStorageBuffer(std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Set const set,
                                  std::uint32_t const offset) override;
    /// Attach uniform buffer to the current pipline.
    ///
    /// @param[in] uniform_buffer   Uniform buffer
    /// @param[in] set              Pipeline set
    /// @param[in] offset           Offset to the first element witing the buffer (In case we want to get accsess to specific buffer's range)
    virtual void setUniformBuffer(std::shared_ptr<UniformBuffer const> const uniform_buffer, Pipeline::Set const set,
                                  std::uint32_t const offset) override;
    /// Attach texture to the current pipline.
    ///
    /// @param[in] storage_buffer   Texture
    /// @param[in] set              Pipeline set
    /// @param[in] array_index      Array index of texture's array (Use only if shader has texture's array at current set, othervice keep as 0)
    virtual void setTexture(std::shared_ptr<Texture2D const> const texture, Pipeline::Set const set, std::uint32_t const binding,
                            std::size_t const array_index) override;
    /// Write uniform right thru to the current pipline (Aka Push constant), instatn version.
    ///
    /// @param[in] render_command_buffer Render command buffer
    /// @param[in] size Uniform size
    /// @param[in] data Pointer to data that we want to use
    /// @param[in] shader_stage  Shader::Stage (Vertex, fragment, geometry...)
    virtual void setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size, void const* data,
                              Shader::Stage const shader_stage, std::uint32_t const frame_index) override;
    /// Write uniform to the current pipline (Aka Push constant).
    ///
    /// @param[in] render_command_buffer Render command buffer
    /// @param[in] size Uniform size
    /// @param[in] data Pointer to data that we want to use
    /// @param[in] shader_stage  Shader::Stage (Vertex, fragment, geometry...)
    virtual void setUniform(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size, void const* data,
                            Shader::Stage const shader_stage) override;
    /// Make all previously set resources available.
    ///
    /// @param[in] render_command_buffer Render command buffer
    virtual void updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer) override;
    /// @brief Recompile the Vulkan pipeline (e.g., after shader changes).
    virtual void recompile() override;
    /// @brief Bind Vulkan descriptor sets for the current frame.
    ///
    /// Binds all descriptor sets stored in this pipeline to the command buffer.
    ///
    /// @param[in] render_command_buffer The command buffer to bind descriptor sets to.
    /// @param[in] frame_index Current frame index.
    void bindDescriptorsSets(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index);

protected:
    /// Constructor for Pipeline
    ///
    /// @param[in] specification  Pipeline specification
    explicit VulkanComputePipeline(RenderPipeline::Specification const& specification);

private:
    /// @brief Release and cleanup Vulkan pipeline and related resources.
    void             invalidate();
    VkPipelineLayout vk_pipeline_layout_{VK_NULL_HANDLE}; ///< Vulkan pipeline layout handle.
    VkPipeline       vk_pipeline_{VK_NULL_HANDLE};        ///< Vulkan pipeline handle.
    /// Vertex input binding descriptions describing vertex buffer layouts.
    std::vector<VkVertexInputBindingDescription> vk_vertex_input_binding_descriptions_;
    /// Resources (descriptors) grouped by pipeline set (_GLOBAL_, _PER_INSTANCE_, _USER_).
    std::array<descriptor::VulkanDescriptorBindings, static_cast<std::size_t>(Pipeline::Set::_MAX_ENUM_)> resources_;
    /// Descriptor set layouts for each pipeline set.
    std::array<std::shared_ptr<descriptor::VulkanDescriptorSetLayout>, static_cast<std::size_t>(Pipeline::Set::_MAX_ENUM_)> descriptors_layouts_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_COMPUTE_PIPELINE_H