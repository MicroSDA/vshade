#ifndef ENGINE_CORE_RENDER_DRAWABLE_MATERIAL_H
#define ENGINE_CORE_RENDER_DRAWABLE_MATERIAL_H

#include <engine/config/vshade_api.h>
#include <engine/core/render/texture.h>
#include <engine/core/utility/factory.h>
#include <glm/glm/glm.hpp>

namespace vshade
{
namespace render
{
class VSHADE_API Material final : public utility::CRTPFactory<Material>
{
    friend class utility::CRTPFactory<Material>;

public:
    // An enumeration class to represent different shading models available
    enum class ShadingModel : std::uint32_t
    {
        _NON_SHADING_,      // No shading at all
        _FLAT_,             // Uses a single color for each face of a 3D object
        _GOURAUD_,          // Interpolates colors across vertices of a 3D object's faces
        _BLINN_PHONG_,      // Uses a reflection model to simulate highlights on a surface
        _PHYSICALLY_BASED_, // A physically-based model that considers real-world material properties
        _RAY_TRACING_,      // Traces the path of light rays in a scene to simulate realistic lighting effects
        _TOON_,             // Produces a cartoon-like effect on 3D objects
        _OREN_NAYAR_,       // A reflection model that simulates light scattering within a material
        _MINNAERT_,         // A reflection model that simulates light absorption and scattering within a material
        _COOK_TORRANCE_,    // A physically-based reflection model that simulates realistic surface textures and roughness
        _FRESNEL_           // A reflection model that takes into account the angle of incidence of light on a surface
    };

    struct RenderData
    {
        explicit RenderData()
        {
            memset(this, 0U, sizeof(RenderData));
        }

        alignas(16) glm::vec3 color_ambient;
        alignas(16) glm::vec3 color_diffuse;
        alignas(16) glm::vec3 color_albedo;
        alignas(16) glm::vec3 color_specular;
        alignas(16) glm::vec3 transparent_mask;
        alignas(4) bool is_normal_map_enabled;
        alignas(4) bool is_bump_map_enabled;
        float        emissive;
        float        opacity;
        float        shininess;
        float        shininess_strength;
        float        refractive_index;
        ShadingModel shading_model;
    };

    RenderData getRenderData() const
    {
        RenderData render_data;

        render_data.color_ambient         = color_ambient;
        render_data.color_diffuse         = color_diffuse;
        render_data.color_albedo          = color_albedo;
        render_data.color_specular        = color_specular;
        render_data.transparent_mask      = transparent_mask;
        render_data.is_normal_map_enabled = is_normal_map_enabled;
        render_data.is_bump_map_enabled   = is_bump_map_enabled;
        render_data.emissive              = emissive;
        render_data.opacity               = opacity;
        render_data.shininess             = shininess;
        render_data.shininess_strength    = shininess_strength;
        render_data.refractive_index      = refractive_index;
        render_data.shading_model         = shading;

        return render_data;
    }

public:
    virtual ~Material()                    = default;
    Material(Material const&)              = delete;
    Material(Material&&)                   = delete;
    Material& operator=(Material const&) & = delete;
    Material& operator=(Material&&) &      = delete;

public:
    glm::vec3    color_ambient{0.f};
    glm::vec3    color_diffuse{1.f};
    glm::vec3    color_albedo{1.f};
    glm::vec3    color_specular{1.f};
    glm::vec3    transparent_mask{-1.f}; // Minus one, so defualt transparent color doesn't exist
    ShadingModel shading{ShadingModel::_NON_SHADING_};
    float        emissive{0.0f};
    float        opacity{1.0f};
    float        shininess{50.0f};
    float        shininess_strength{1.0f};
    float        refractive_index{0.0f};
    bool         is_normal_map_enabled{false};
    bool         is_bump_map_enabled{false};

    std::shared_ptr<Texture2D> texture_diffuse;
    std::shared_ptr<Texture2D> texture_specular;
    std::shared_ptr<Texture2D> texture_normals;
    std::shared_ptr<Texture2D> texture_roughness;
    std::shared_ptr<Texture2D> texture_albedo;
    std::shared_ptr<Texture2D> texture_metallic;

protected:
    explicit Material() = default;
};

static constexpr std::size_t _MaterialDataSize_{sizeof(Material::RenderData)};

static std::size_t MATERIAL_DATA_SIZE(std::size_t const count)
{
    return _MaterialDataSize_ * count;
}

} // namespace render
} // namespace vshade

#endif ENGINE_CORE_RENDER_DRAWABLE_MATERIAL_H