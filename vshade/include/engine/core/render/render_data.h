#ifndef ENGINE_CORE_RENDER_RENDER_DATA_H
#define ENGINE_CORE_RENDER_RENDER_DATA_H

#include <ankerl/unordered_dense.h>
#include <glm/glm/glm.hpp>
#include <iostream>

#include <engine/core/render/buffers/index_buffer.h>
#include <engine/core/render/buffers/storage_buffer.h>
#include <engine/core/render/buffers/uniform_buffer.h>

#include <engine/core/render/buffers/vertex_buffer.h>
#include <engine/core/render/camera/camera.h>
#include <engine/core/render/drawable/drawable.h>
#include <engine/core/render/drawable/material.h>

namespace vshade
{
namespace render
{
namespace data
{
/**
 * @brief Stores a set of materials used by a single model (Drawable).
 */
struct ModelsPerMaterial final
{
    ankerl::unordered_dense::set<std::shared_ptr<Material const>> materials;      ///< Unique materials used with the model.
    std::size_t                                                   model_hash{0U}; ///< Hash of the Drawable this structure corresponds to.
    std::int64_t                                                  time_stamp{0};
};

/**
 * @brief Contains per-instance transform and material data.
 *
 * This data will be grouped by unique (Pipeline, Drawable, Material) combination.
 */
struct InstanceSubmission final
{
    std::vector<glm::mat4>                       transforms;            ///< Local transforms of each instance.
    std::vector<Material::RenderData>            materials_render_data; ///< Material data per instance.
    std::vector<std::shared_ptr<Material const>> materials;             ///< Material data per instance.
    std::size_t                                  tansform_offset{0U};   ///< Offset to the first transform in a global buffer.
    std::size_t                                  material_offset{0U};   ///< Offset to the first material in a global buffer.
};

/**
 * @brief Geometry buffers required to render a Drawable.
 */
struct InstanceGeometryRenderBuffers final
{
    std::shared_ptr<VertexBuffer> vertex_buffer; ///< Vertex buffer.
    std::shared_ptr<IndexBuffer>  index_buffer;  ///< Index buffer.
    std::shared_ptr<VertexBuffer> bone_buffer;   ///< Bone transform buffer.
};

/**
 * @brief Maps a unique (Pipeline, Drawable, Material) hash to its instance submission data.
 */
using SortedSubmissionMap = ankerl::unordered_dense::map<std::size_t, InstanceSubmission>;

/**
 * @brief Maps each Drawable to the set of materials it's rendered with.
 */
using DrawableMaterialMap = ankerl::unordered_dense::map<std::size_t, ModelsPerMaterial>;

/**
 * @brief Maps each Pipeline (by hash) to the set of Drawables and their associated materials.
 *
 * This structure allows tracking which materials are used with which Drawable for a given Pipeline.
 */
using PipelineSubmissionInstanceMap = ankerl::unordered_dense::map<std::size_t, DrawableMaterialMap>;

/**
 * @brief Contains all rendering data submitted for a single frame.
 */
struct SubmittedFrameData final
{
    ankerl::unordered_dense::map<std::size_t, InstanceGeometryRenderBuffers>
                                  instance_geometry_render_buffers; ///< Geometry buffers mapped by Drawable hash.
    SortedSubmissionMap           sorted_submission_map;            ///< Instance data grouped by hashed (Pipeline, Drawable, Material).
    PipelineSubmissionInstanceMap pipeline_submission_instance_map; ///< Submitted instances corresponding to a specific pipeline.

    std::shared_ptr<StorageBuffer> global_transform_buffer; ///< Global buffer of all transforms.
    std::shared_ptr<StorageBuffer> global_materials_buffer; ///< Global buffer of all materials.
    // Future resources to be added:
    // std::shared_ptr<StorageBuffer> global_lights_buffer;
    // std::shared_ptr<StorageBuffer> point_lights_buffer;
    // std::shared_ptr<StorageBuffer> spot_lights_buffer;
    // std::shared_ptr<StorageBuffer> bone_transforms_buffer;
    std::shared_ptr<UniformBuffer> camera_buffer;
    // std::shared_ptr<UniformBuffer> scene_render_data_buffer;
    // std::shared_ptr<UniformBuffer> render_settings_data_buffer;
};

} // namespace data
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_RENDER_DATA_H
