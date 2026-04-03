#ifndef ENGINE_CORE_RENDER_SHADER_H
#define ENGINE_CORE_RENDER_SHADER_H

#include <ankerl/unordered_dense.h>
#include <engine/core/serialalizing/file.h>
#include <engine/core/utility/factory.h>
#include <filesystem>
#include <fstream>

namespace vshade
{
namespace render
{
class VSHADE_API Shader : public utility::CRTPFactory<Shader>
{
    friend class utility::CRTPFactory<Shader>;
    using utility::CRTPFactory<Shader>::create;

public:
    struct Specification
    {
        std::string                                       name;
        std::string                                       file_path;
        std::vector<std::tuple<std::string, std::string>> difinitions;
    };

    enum Stage : std::uint32_t
    {
        _UNDEFINED_               = 0x00000000, // Undefined
        _VERTEX_                  = 0x00000001, // VK_SHADER_STAGE_VERTEX_BIT
        _TESSELLATION_CONTROL_    = 0x00000002, // VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
        _TESSELLATION_EVALUATION_ = 0x00000004, // VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
        _GEOMETRY_                = 0x00000008, // VK_SHADER_STAGE_GEOMETRY_BIT
        _FRAGMENT_                = 0x00000010, // VK_SHADER_STAGE_FRAGMENT_BIT
        _COMPUTE_                 = 0x00000020, // VK_SHADER_STAGE_COMPUTE_BIT

        _ALL_GRAPHICS_ = 0x0000001F, // VK_SHADER_STAGE_ALL_GRAPHICS
        _ALL_          = 0x7FFFFFFF, // VK_SHADER_STAGE_ALL

        _MAX_ENUM_
    };

    enum class DataType
    {
        _NONE_ = 0,
        _FLOAT_,
        _FLOAT_2_,
        _FLOAT_3_,
        _FLOAT_4_,
        _MAT_3_,
        _MAT_4_,
        _INT_,
        _INT_2_,
        _INT_3_,
        _INT_4_,
        _BOOL_,

        _MAX_ENUM_
    };

    using StageFlags = std::uint32_t;

public:
    virtual ~Shader()                  = default;
    Shader(Shader const&)              = delete;
    Shader(Shader&&)                   = delete;
    Shader& operator=(Shader const&) & = delete;
    Shader& operator=(Shader&&) &      = delete;

    static std::shared_ptr<Shader> create(Specification const& specification, bool is_chache_ignored = false);

    static Stage         getStageFromString(std::string const& str);
    static std::string   getStageAsString(Stage stage);
    static std::uint32_t getDataTypeSize(DataType const& type);
    static std::string   getShaderCacheDirectory();

    Specification const& getSpecification() const
    {
        return specification_;
    }

protected:
    explicit Shader(Specification const& specification);
    std::string readSourceFile(std::string const& file_path);
    void        preProcess(std::string& source, std::string const& origin);
    void        includer(std::string& source, Stage stage, std::string const& file_path, std::string const& origin);

    Specification                                                                                                             specification_;
    std::string const                                                                                                         directory_path_;
    ankerl::unordered_dense::map<Stage, std::string>                                                                          shader_source_code_;
    ankerl::unordered_dense::map<Stage, ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::set<std::string>>> headers_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_SHADER_H