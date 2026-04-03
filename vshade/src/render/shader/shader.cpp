#include "engine/core/render/shader/shader.h"
#include <engine/core/render/render.h>
#include <engine/platforms/render/vulkan/vulkan_shader.h>

namespace shader_utils
{
static void createShaderCacheDirectory(std::string const& path)
{
    if (!std::filesystem::exists(path))
    {
        VSHADE_CORE_DEBUG("Creating shader's cache directory : {}", path)
        std::filesystem::create_directories(path);
    }
}
} // namespace shader_utils

std::string vshade::render::Shader::getShaderCacheDirectory()
{
    return std::string("/resources/" + System::instance().getCurentRenderAPIAsString() + "/");
}

std::shared_ptr<vshade::render::Shader> vshade::render::Shader::create(Specification const& specification, bool is_chache_ignored)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<Shader>::create<VulkanShader>(specification, is_chache_ignored);
    }
}

vshade::render::Shader::Shader(Specification const& specification)
    : specification_{specification}, directory_path_{std::filesystem::path(specification.file_path).remove_filename().string()}
{
    std::string source = readSourceFile(specification.file_path);
    if (!source.empty())
    {
        preProcess(source, std::filesystem::path(specification.file_path).filename().stem().string());
        shader_utils::createShaderCacheDirectory(file::FileManager::instance().getRootDirectory() + getShaderCacheDirectory());
    }
}

std::string vshade::render::Shader::readSourceFile(std::string const& file_path)
{
    std::string source;

    if (std::ifstream file = file::FileManager::instance().getNativeFile(file_path, std::ios::binary))
    {
        file.seekg(0, std::ios::end);
        std::size_t fileSize = file.tellg();
        if (fileSize)
        {
            file.seekg(0, std::ios::beg);
            source.resize(fileSize, ' ');
            file.read(source.data(), fileSize);
        }

        file.close();
    }

    return source;
}

void vshade::render::Shader::preProcess(std::string& source, std::string const& origin)
{
    // String used to identify the start of the shader type in the source code
    std::string const shade_stage_token   = "#pragma : ";
    std::string const shade_version_token = "#version";

    // Find the first occurrence of the shade_stage_token in the source code
    std::size_t position{0U};
    std::size_t start{0U};

    // Keep searching the source code until all occurrences of the shade_stage_token have been found
    while (position != std::string::npos)
    {   
        start    = position;
        position = source.find(shade_version_token, position);
        // Get the index of the first end-of-line character after the shade_stage_token
        std::size_t eol = source.find_first_of("\r\n", position);
        // If the variable "eol" doesn't have a value that represents the end of the string

        if (eol != std::string::npos)
        {
            std::size_t stage_position{source.find(shade_stage_token, eol)};
            // End of stage token !
            eol = source.find_first_of("\r\n", stage_position);

            std::string stage{source.substr(stage_position + shade_stage_token.size(), eol - stage_position - shade_stage_token.size())};
            stage.erase(std::remove_if(stage.begin(), stage.end(), ::isspace), stage.end());

            Stage shader_stage = getStageFromString(stage);
            if (shader_stage != Shader::Stage::_UNDEFINED_)
            {
                // Need to find end of current shader !
                eol = source.find(shade_version_token, eol);

                std::string complited{(eol == std::string::npos) ? source.substr(start) : source.substr(start, eol - start)};
                includer(complited, shader_stage, directory_path_, origin);
                // Add complited to the shaderCode using type as the key
                shader_source_code_[shader_stage] = complited;
            }
            else
            {
                VSHADE_CORE_WARNING("Undefined shader stage !");
            }
        }

        position = eol;
    }

    // for (auto& [stage, code] : shader_source_code_)
    // {
    //     std::cout << "--------------------" << getStageAsString(stage) << "--------------------------" << std::endl;
    //     std::cout << code << std::endl;
    // }
}

void vshade::render::Shader::includer(std::string& source, Stage stage, std::string const& file_path, std::string const& origin)
{
    std::string const shader_include_token = "#include";
    std::size_t       position{0U};
    // Loop through the source code until all include tokens are processed
    while (position != std::string::npos)
    {
        // Search for the first occurrence of the include token starting from the specified position
        position = source.find(shader_include_token, position);
        // Search for the first occurrence of a line break character after the include token
        std::size_t eol = source.find_first_of("\r\n", position);
        // If a line break character was found
        if (eol != std::string::npos)
        {
            // Get the path to the file being included, which is between the include token and the line break character
            std::size_t begin = position + shader_include_token.size();
            std::string path  = source.substr(begin, eol - begin);
            // Remove any quotation marks or white space from the file path
            path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
            path.erase(std::remove_if(path.begin(), path.end(), ::isspace), path.end());
            // path.erase(path.begin(), std::find_if(path.begin(), path.end(), [](auto c) { return !std::isspace(c); }));

            auto const exist = headers_[stage][origin].find(std::filesystem::path(path).filename().stem().string());
            if (exist == headers_[stage][origin].end())
            {
                headers_[stage][origin].insert(std::filesystem::path(path).filename().stem().string());
                // Remove the entire include statement from the source code
                source.erase(position, eol - position);

                std::string include_source;
                {
                    std::size_t count{0U};
                    std::size_t up_dir_position{path.find("./")};

                    while (up_dir_position != std::string::npos)
                    {
                        up_dir_position = path.find("./", up_dir_position + 2U);
                        count++;
                    }

                    path.erase(0U, count * 2U);
                    std::string up_directory = file_path;
                    for (std::size_t i{0U}; i < count; ++i)
                    {
                        up_directory = std::filesystem::path(up_directory).parent_path().parent_path().string();
                    }

                    // Read the contents of the included file
                    include_source = readSourceFile(up_directory + "/" + path);
                }
                // If the included file was successfully read, insert its contents into the source code at the position of the include statement
                if (include_source.size())
                {
                    includer(include_source, stage, file_path, origin);
                    source.insert(position, include_source);
                }
            }
            else
            {
                // Remove the entire include statement from the source code
                source.erase(position, eol - position);
                position++;
            }
        }
    }
}

vshade::render::Shader::Stage vshade::render::Shader::getStageFromString(std::string const& str)
{
    if (str == "vertex")
    {
        return vshade::render::Shader::Stage::_VERTEX_;
    }
    if (str == "fragment")
    {
        return vshade::render::Shader::Stage::_FRAGMENT_;
    }
    if (str == "geometry")
    {
        return vshade::render::Shader::Stage::_GEOMETRY_;
    }
    if (str == "compute")
    {
        return vshade::render::Shader::Stage::_COMPUTE_;
    }

    return vshade::render::Shader::Stage::_UNDEFINED_;
}

std::string vshade::render::Shader::getStageAsString(Stage stage)
{
    switch (stage)
    {
    case Stage::_VERTEX_:
        return "Vertex";
    case Stage::_FRAGMENT_:
        return "Fragment";
    case Stage::_COMPUTE_:
        return "Compute";
    case Stage::_GEOMETRY_:
        return "Geometry";
    default:
        return "Undefined";
    }
}

std::uint32_t vshade::render::Shader::getDataTypeSize(DataType const& type)
{
    switch (type)
    {
    case DataType::_FLOAT_:
        return 4U;
    case DataType::_FLOAT_2_:
        return 4U * 2U;
    case DataType::_FLOAT_3_:
        return 4U * 3U;
    case DataType::_FLOAT_4_:
        return 4U * 4U;
    case DataType::_MAT_3_:
        return 4U * 3U * 3U;
    case DataType::_MAT_4_:
        return 4U * 4U * 4U;
    case DataType::_INT_:
        return 4U;
    case DataType::_INT_2_:
        return 4U * 2U;
    case DataType::_INT_3_:
        return 4U * 3U;
    case DataType::_INT_4_:
        return 4U * 4U;
    case DataType::_BOOL_:
        return 1U;
    default:
        return 0U;
    }
}