#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_SHADER_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_SHADER_H

#include <engine/core/render/shader/shader.h>
#include <engine/core/serialalizing/file_manager.h>
#include <engine/core/utility/hash.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
namespace shader_resource
{
struct StorageBuffer
{
    std::uint32_t      binding{0U};
    std::uint32_t      set{0U};
    std::uint32_t      size{1U};
    std::string        name;
    VkShaderStageFlags stage{0U};
};
struct UniformBuffer
{
    std::uint32_t      binding{0U};
    std::uint32_t      set{0U};
    std::uint32_t      size{1U};
    std::string        name;
    VkShaderStageFlags stage{0U};
};
struct ImageSampler
{
    std::uint32_t      binding{0U};
    std::uint32_t      set{0U};
    std::uint32_t      array_size{1U};
    std::string        name;
    VkShaderStageFlags stage{0U};
};
struct ImageStorage
{
    std::uint32_t      binding{0U};
    std::uint32_t      set{0U};
    std::uint32_t      array_size{1U};
    std::string        name;
    VkShaderStageFlags stage{0U};
};
struct PushConstant
{
    std::uint32_t      set{0U};
    std::uint32_t      size{1U};
    std::string        name;
    VkShaderStageFlags stage{0U};
};
}; // namespace shader_resource

class VulkanShader final : public Shader
{
    friend class utility::CRTPFactory<Shader>;

public:
    struct ShaderResources
    {
        // Name -> Binding -> Buffer
        ankerl::unordered_dense::map<std::string, shader_resource::StorageBuffer> storage_buffers;
        ankerl::unordered_dense::map<std::string, shader_resource::UniformBuffer> uniform_buffers;
        ankerl::unordered_dense::map<std::string, shader_resource::ImageSampler>  image_samplers;
        ankerl::unordered_dense::map<std::string, shader_resource::ImageStorage>  image_storage;
        ankerl::unordered_dense::map<std::string, shader_resource::PushConstant>  push_constants;
    };

public:
    virtual ~VulkanShader();
    VulkanShader(VulkanShader const&)              = delete;
    VulkanShader(VulkanShader&&)                   = delete;
    VulkanShader& operator=(VulkanShader const&) & = delete;
    VulkanShader& operator=(VulkanShader&&) &      = delete;

    ankerl::unordered_dense::map<std::uint32_t, ShaderResources>& getReflectedData()
    {
        return reflected_data_;
    }
    std::vector<VkPipelineShaderStageCreateInfo> const& getVkPipelineShaderStageCreateInfo() const
    {
        return vk_pipeline_shader_stage_create_infos_;
    }

    static VkFormat              getShaderDataToVulkanFormat(Shader::DataType const& type);
    static VkShaderStageFlagBits fromShaderStageToVkShaderType(Shader::Stage const& stage);
    static VkShaderStageFlags    fromShaderStageFlagsToVkShaderStageFlags(Shader::StageFlags flags);

protected:
    explicit VulkanShader(Shader::Specification const& specification, bool is_chache_ignored);

private:
    void           createVkShader();
    void           reflectShaderData(Shader::Stage stage, std::vector<uint32_t> const& shader_data);
    VkShaderModule createShaderModule(VulkanInstance& vulkan_instance, VkDevice device, std::vector<std::uint32_t> const& code);

    VkShaderStageFlags                           vk_stages_flags_{0U};
    std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_infos_;

    spirv_cross::ShaderResources                                            sprv_reflection_data_;
    ankerl::unordered_dense::map<std::uint32_t, ShaderResources>            reflected_data_;
    ankerl::unordered_dense::map<Shader::Stage, std::vector<std::uint32_t>> vulkan_spirv_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_SHADER_H