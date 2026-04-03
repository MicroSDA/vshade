#include "engine/platforms/render/vulkan/vulkan_shader.h"
#include <engine/core/render/pipeline.h>
#include <engine/core/render/render.h>

namespace shader_utils
{
static char const* getCachedFileExtension(vshade::render::Shader::Stage stage)
{
    switch (stage)
    {
    case vshade::render::Shader::Stage::_VERTEX_:
        return ".vk.c.vert";
    case vshade::render::Shader::Stage::_FRAGMENT_:
        return ".vk.c.frag";
    case vshade::render::Shader::Stage::_GEOMETRY_:
        return ".vk.c.geom";
    case vshade::render::Shader::Stage::_COMPUTE_:
        return ".vk.c.comp";
    default:
        return ".vk.c.undefined";
    }
}
static shaderc_shader_kind toShaderCShaderType(vshade::render::Shader::Stage const& stage)
{
    switch (stage)
    {
    case vshade::render::Shader::Stage::_VERTEX_:
        return shaderc_glsl_vertex_shader;
    case vshade::render::Shader::Stage::_FRAGMENT_:
        return shaderc_glsl_fragment_shader;
    case vshade::render::Shader::Stage::_GEOMETRY_:
        return shaderc_glsl_geometry_shader;
    case vshade::render::Shader::Stage::_COMPUTE_:
        return shaderc_glsl_compute_shader;
    default:
        return (shaderc_shader_kind)0;
    }
}
static char const* shaderCSStageToString(shaderc_shader_kind const& stage)
{
    switch (stage)
    {
    case shaderc_glsl_vertex_shader:
        return "Vertex shader";
    case shaderc_glsl_fragment_shader:
        return "Fragment shader";
    case shaderc_glsl_geometry_shader:
        return "Geometry shader";
    case shaderc_glsl_compute_shader:
        return "Compute shader";
    default:
        return "Undefined shader";
    }
}
static char const* shaderStageToString(vshade::render::Shader::Stage const& stage)
{
    switch (stage)
    {
    case vshade::render::Shader::Stage::_VERTEX_:
        return "Vertex shader";
    case vshade::render::Shader::Stage::_FRAGMENT_:
        return "Fragment shader";
    case vshade::render::Shader::Stage::_GEOMETRY_:
        return "Geometry shader";
    case vshade::render::Shader::Stage::_COMPUTE_:
        return "Compute shader";
    default:
        return "Undefined shader";
    }
}

static std::string getResourceValidName(std::string_view name, std::uint32_t const set, std::uint32_t const binding)
{
#ifdef _VSHADE_DEBUG_
    return std::string{name};
#else
    return std::string{std::to_string(vshade::utils::PairHash{}(std::make_pair(set, binding)))};
#endif // _VSHADE_DEBUG_
}

} // namespace shader_utils
vshade::render::VulkanShader::VulkanShader(Shader::Specification const& specification, bool is_chache_ignored) : Shader(specification)
{
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    //  TODO: From config
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    //------------------------------------------------------------------------
    // Enable global definitions
    //------------------------------------------------------------------------
    for (auto const& [name, value] : Render::_SHADER_BINDINGS_AND_DEFINITIONS_)
    {
        options.AddMacroDefinition(name, std::to_string(value));
    }
    //------------------------------------------------------------------------
    // Enable user definitions
    //------------------------------------------------------------------------
    for (auto const& [name, value] : specification.difinitions)
    {
        options.AddMacroDefinition(name, value);
    }

#ifdef _VSHADE_DEBUG_
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
    options.SetGenerateDebugInfo();
#else
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif // _VSHADE_DEBUG_

    std::filesystem::path cache_directory{getShaderCacheDirectory()};
    std::filesystem::path shader_file_path{specification.file_path};

    if (shader_source_code_.empty())
    {
        VSHADE_CORE_INFO("Trying to load shader form cache or binary packet, path = {}", specification.file_path);

        // Try to find in cache or in packet
        for (Stage stage = Stage::_VERTEX_; stage < Stage::_MAX_ENUM_; ((std::uint16_t&)stage)++)
        {
            std::filesystem::path cached_path{cache_directory / (specification.name + shader_utils::getCachedFileExtension(stage))};

            // File in(cachedPath.string(), File::In | File::SkipMagic | File::SkipChecksum, "", File::VERSION(0, 0, 1));

            if (file::File file = file::FileManager::instance().loadFile(cached_path.string(), ""))
            {
                std::vector<std::uint32_t>& data{vulkan_spirv_[stage]};
                // in case the shader can be as part of packet !
                std::size_t size{file.geFileHeader().content_size};
                data.resize(size / sizeof(std::uint32_t));
                file.getInternalBuffer()->read(reinterpret_cast<char*>(data.data()), size);
            }
            else
            {
                VSHADE_CORE_ERROR("Failed to load shader form cache or binary packet, path = {}", cached_path.string());
            }
        }
    }
    else
    {
        for (auto&& [stage, source] : shader_source_code_)
        {
            std::filesystem::path cached_path{cache_directory / (specification.name + shader_utils::getCachedFileExtension(stage))};
            // File in(cachedPath.string(), File::In | File::SkipMagic | File::SkipChecksum, "", File::VERSION(0, 0, 1));

            if (file::File file = file::FileManager::instance().loadFile(cached_path.string(), ""); file && !is_chache_ignored)
            {
                VSHADE_CORE_INFO("Load shader form cache, path = {}", cached_path.string());

                std::vector<std::uint32_t>& data{vulkan_spirv_[stage]};
                std::uint32_t               size{file.geFileHeader().content_size};
                data.resize(size / sizeof(std::uint32_t));
                file.getInternalBuffer()->read(reinterpret_cast<char*>(data.data()), size);
            }
            else
            {
                // Complile shader without cache reading
                shaderc::SpvCompilationResult module{
                    compiler.CompileGlslToSpv(source, shader_utils::toShaderCShaderType(stage), shader_file_path.string().c_str(), options)};

                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    std::string        line;
                    std::istringstream stream{source};
                    std::uint32_t      index{0U};
                    while (std::getline(stream, line))
                    {
                        VSHADE_CORE_INFO("{0}: {1}", index, line);
                        index++;
                    }

                    VSHADE_CORE_ERROR("{}", module.GetErrorMessage());
                }

                vulkan_spirv_[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

                // File out(cachedPath.string(), File::Out | File::SkipMagic | File::SkipChecksum, "", File::VERSION(0, 0, 1));
                if (file::File file = file::FileManager::instance().saveFile(cached_path.string(), ""))
                {
                    std::vector<std::uint32_t>& data = vulkan_spirv_[stage];
                    file.getInternalBuffer()->write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(std::uint32_t));
                }
                else
                {
                    VSHADE_CORE_ERROR("Failed to save shader form cache or packet, path = {}", specification.file_path);
                }
            }
        }
    }

    for (auto&& [stage, data] : vulkan_spirv_)
    {
        reflectShaderData(stage, data);
    }

    if (!vulkan_spirv_.empty())
    {
        createVkShader();
    }
    else
    {
        VSHADE_CORE_WARNING("Failed to create the shader, shader doesn't exist, path = {}", specification.file_path);
    }
}

void vshade::render::VulkanShader::createVkShader()
{
    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    for (auto& [stage, shader] : vulkan_spirv_)
    {
        vk_stages_flags_ |= fromShaderStageToVkShaderType(stage);

        /*  This VkPipelineShaderStageCreateInfo will hold information about a single shader stage for the pipeline.
            We build it from a shader stage and a shader module. */
        VkPipelineShaderStageCreateInfo& vk_pipeline_shader_stage_create_info{vk_pipeline_shader_stage_create_infos_.emplace_back()};
        vk_pipeline_shader_stage_create_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vk_pipeline_shader_stage_create_info.pNext               = VK_NULL_HANDLE;
        vk_pipeline_shader_stage_create_info.flags               = 0U;
        vk_pipeline_shader_stage_create_info.stage               = fromShaderStageToVkShaderType(stage);
        vk_pipeline_shader_stage_create_info.module              = createShaderModule(vulkan_instance, vk_logical_device, shader);
        vk_pipeline_shader_stage_create_info.pName               = "main";
        vk_pipeline_shader_stage_create_info.pSpecializationInfo = VK_NULL_HANDLE;

        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_SHADER_MODULE>(vulkan_instance.instance, shader_utils::shaderStageToString(stage),
                                                                   vk_logical_device, vk_pipeline_shader_stage_create_infos_.back().module);
    }
}

void vshade::render::VulkanShader::reflectShaderData(Shader::Stage stage, std::vector<uint32_t> const& shader_data)
{
    spirv_cross::Compiler compiler(shader_data);
    sprv_reflection_data_ = compiler.get_shader_resources();

    VSHADE_CORE_DEBUG("=========================== Reflect shader data ===========================");

    for (auto const& resource : sprv_reflection_data_.storage_buffers)
    {
        spirv_cross::SPIRType const& buffer_type{compiler.get_type(resource.base_type_id)};
        std::uint32_t const          set{compiler.get_decoration(resource.id, spv::DecorationDescriptorSet)};
        std::uint32_t const          binding{compiler.get_decoration(resource.id, spv::DecorationBinding)};

        std::string const name{std::move(shader_utils::getResourceValidName(resource.name, set, binding))};

        shader_resource::StorageBuffer& buffer{reflected_data_[set].storage_buffers[name]};
        buffer.name    = name;
        buffer.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        buffer.set     = set;
        buffer.size    = compiler.get_declared_struct_size(buffer_type);
        buffer.stage |= fromShaderStageFlagsToVkShaderStageFlags(stage);

        VSHADE_CORE_DEBUG("Storage buffer: name: {0}, set: {1}, binding: {2}, size: {3}", buffer.name, buffer.set, buffer.binding, buffer.size);
    }
    for (auto const& resource : sprv_reflection_data_.uniform_buffers)
    {
        spirv_cross::SPIRType const& buffer_type{compiler.get_type(resource.base_type_id)};
        std::uint32_t const          set{compiler.get_decoration(resource.id, spv::DecorationDescriptorSet)};
        std::uint32_t const          binding{compiler.get_decoration(resource.id, spv::DecorationBinding)};

        std::string const name{std::move(shader_utils::getResourceValidName(resource.name, set, binding))};

        shader_resource::UniformBuffer& buffer{reflected_data_[set].uniform_buffers[name]};
        buffer.name    = name;
        buffer.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        buffer.set     = set;
        buffer.size    = compiler.get_declared_struct_size(buffer_type);
        buffer.stage |= fromShaderStageFlagsToVkShaderStageFlags(stage);

        VSHADE_CORE_DEBUG("UniformBuffer: name: {0}, set: {1}, binding: {2}, size: {3}", buffer.name, buffer.set, buffer.binding, buffer.size);
    }

    for (auto const& resource : sprv_reflection_data_.sampled_images)
    {
        std::uint32_t const set{compiler.get_decoration(resource.id, spv::DecorationDescriptorSet)};
        std::uint32_t const binding{compiler.get_decoration(resource.id, spv::DecorationBinding)};

        std::string const name{std::move(shader_utils::getResourceValidName(resource.name, set, binding))};

        shader_resource::ImageSampler& image{reflected_data_[set].image_samplers[name]};
        image.name    = name;
        image.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        image.set     = set;
        image.stage |= fromShaderStageFlagsToVkShaderStageFlags(stage);

        spirv_cross::SmallVector<std::uint32_t> const& array = compiler.get_type(resource.type_id).array;
        image.array_size                                     = (array.size()) ? array[0] : image.array_size; // Shader texture array size

        VSHADE_CORE_DEBUG("Sampled image: name: {0}, set: {1}, binding: {2}", image.name, image.set, image.binding);
    }

    for (auto const& resource : sprv_reflection_data_.storage_images)
    {
        std::uint32_t const set{compiler.get_decoration(resource.id, spv::DecorationDescriptorSet)};
        std::uint32_t const binding{compiler.get_decoration(resource.id, spv::DecorationBinding)};

        std::string const name{std::move(shader_utils::getResourceValidName(resource.name, set, binding))};

        shader_resource::ImageStorage& image{reflected_data_[set].image_storage[name]};
        image.name    = name;
        image.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        image.set     = set;
        image.stage |= fromShaderStageFlagsToVkShaderStageFlags(stage);

        spirv_cross::SmallVector<std::uint32_t> const& array = compiler.get_type(resource.type_id).array;
        image.array_size                                     = (array.size()) ? array[0] : image.array_size; // Shader texture array size

        VSHADE_CORE_DEBUG("Storage image: name: {0}, set: {1}, binding: {2}", image.name, image.set, image.binding);
    }
    for (auto const& resource : sprv_reflection_data_.push_constant_buffers)
    {
        spirv_cross::SPIRType const& buffer_type{compiler.get_type(resource.base_type_id)};
        std::uint32_t const          set{compiler.get_decoration(resource.id, spv::DecorationDescriptorSet)};

        std::string const name{std::move(shader_utils::getResourceValidName(compiler.get_block_fallback_name(resource.id), 0U, 0U))};

        shader_resource::PushConstant& push_constant{reflected_data_[set].push_constants[name]};
        push_constant.name = name;
        push_constant.set  = set;
        push_constant.size = compiler.get_declared_struct_size(buffer_type);
        push_constant.stage |= fromShaderStageFlagsToVkShaderStageFlags(stage);

        VSHADE_CORE_DEBUG("Uniform: name: {0}, size: {1}", name, push_constant.size);
    }
}
VkShaderModule vshade::render::VulkanShader::createShaderModule(VulkanInstance& vulkan_instance, VkDevice device,
                                                                std::vector<std::uint32_t> const& code)
{
    VkShaderModuleCreateInfo vk_shader_module_create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    vk_shader_module_create_info.pNext    = VK_NULL_HANDLE;
    vk_shader_module_create_info.flags    = 0U;
    vk_shader_module_create_info.codeSize = sizeof(std::uint32_t) * code.size();
    vk_shader_module_create_info.pCode    = code.data();

    VkShaderModule vk_shader_module{VK_NULL_HANDLE};

    VK_CHECK_RESULT(vkCreateShaderModule(device, &vk_shader_module_create_info, vulkan_instance.allocation_callbaks, &vk_shader_module),
                    "Failed to crate shader module!");

    return vk_shader_module;
}
VkFormat vshade::render::VulkanShader::getShaderDataToVulkanFormat(Shader::DataType const& type)
{
    switch (type)
    {
    case Shader::DataType::_FLOAT_:
        return VK_FORMAT_R32_SFLOAT;
    case Shader::DataType::_FLOAT_2_:
        return VK_FORMAT_R32G32_SFLOAT;
    case Shader::DataType::_FLOAT_3_:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Shader::DataType::_FLOAT_4_:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Shader::DataType::_MAT_4_:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Shader::DataType::_INT_:
        return VK_FORMAT_R32_SINT;
    case Shader::DataType::_INT_2_:
        return VK_FORMAT_R32G32_SINT;
    case Shader::DataType::_INT_3_:
        return VK_FORMAT_R32G32B32_SINT;
    case Shader::DataType::_INT_4_:
        return VK_FORMAT_R32G32B32A32_SINT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

VkShaderStageFlagBits vshade::render::VulkanShader::fromShaderStageToVkShaderType(Shader::Stage const& stage)
{
    switch (stage)
    {
    case Shader::Stage::_VERTEX_:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case Shader::Stage::_FRAGMENT_:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case Shader::Stage::_GEOMETRY_:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case Shader::Stage::_COMPUTE_:
        return VK_SHADER_STAGE_COMPUTE_BIT;

    default:
        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; // Undefined
    }
}

VkShaderStageFlags vshade::render::VulkanShader::fromShaderStageFlagsToVkShaderStageFlags(Shader::StageFlags flags)
{
    VkShaderStageFlags vk_shader_stage_flags{0U};

    if (flags & static_cast<std::uint32_t>(Shader::Stage::_VERTEX_))
        vk_shader_stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (flags & static_cast<std::uint32_t>(Shader::Stage::_FRAGMENT_))
        vk_shader_stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (flags & static_cast<std::uint32_t>(Shader::Stage::_GEOMETRY_))
        vk_shader_stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (flags & static_cast<std::uint32_t>(Shader::Stage::_COMPUTE_))
        vk_shader_stage_flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    return vk_shader_stage_flags;
}

vshade::render::VulkanShader::~VulkanShader()
{
    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    RenderContext::instance().enqueDelete(
        [vk_pipeline_shader_stage_create_infos = vk_pipeline_shader_stage_create_infos_, vk_logical_device,
         vulkan_instance](std::uint32_t const frame_index)
        {
            for (VkPipelineShaderStageCreateInfo vk_pipeline_shader_stage_create_info : vk_pipeline_shader_stage_create_infos)
            {
                if (vk_pipeline_shader_stage_create_info.module != VK_NULL_HANDLE)
                {
                    vkDestroyShaderModule(vk_logical_device, vk_pipeline_shader_stage_create_info.module, vulkan_instance.allocation_callbaks);
                }
            }
        });
}
