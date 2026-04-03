#ifndef ENGINE_CORE_RENDER_PIPELINE_H
#define ENGINE_CORE_RENDER_PIPELINE_H

#include <engine/config/vshade_api.h>
#include <engine/core/render/buffers/frame_buffer.h>
#include <engine/core/render/buffers/vertex_buffer.h>
#include <engine/core/render/render_command_buffer.h>
#include <engine/core/render/render_data.h>
#include <engine/core/render/shader/shader.h>
#include <engine/core/utility/factory.h>

namespace vshade
{
namespace render
{
class Render;

/// @brief Base graphics pipeline class.
///
/// Defines the common interface for all rendering pipeline implementations.
/// Used in the rendering system as an abstraction over specific
/// graphics API implementations (e.g., Vulkan, OpenGL, DirectX).
///
/// Inherits from utility::CRTPFactory via CRTP (Curiously Recurring Template Pattern),
/// allowing the creation of concrete pipeline instances through a factory method
/// while preserving static typing.
///
/// Main responsibilities:
/// - Represent a set of configurations and states required for rendering.
/// - Provide a unified interface between the rendering system and a specific
///   graphics API.
/// - Define the interface that all derived pipeline classes must implement.
class Pipeline : public utility::CRTPFactory<Pipeline>
{
    friend class vshade::render::Render;
    friend class utility::CRTPFactory<Pipeline>;
    using utility::CRTPFactory<Pipeline>::create; //< Mute CRTPFactory::create

public:
    /// @brief Descriptor set binding slots.
    ///
    /// Defines predefined descriptor set indexes used by the pipeline:
    /// - GLOBAL: Shared resources such as camera data, scene lighting.
    /// - PER_INSTANCE: Per-object or per-draw resources.
    /// - USER: User-defined custom bindings.
    enum class Set
    {
        _GLOBAL_ = 0U,
        _PER_INSTANCE_,
        _USER_,

        _MAX_ENUM_
    };

    /// @brief Shader stage flags (bitmask).
    ///
    /// Used to specify which pipeline stages a resource or barrier is synchronized with.
    /// Follows Vulkan's VkPipelineStageFlagBits semantics:
    /// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineStageFlagBits.html
    ///
    /// Each flag represents a stage of execution in the GPU pipeline.
    /// Multiple flags can be combined using bitwise OR.
    enum class Stage : std::uint32_t
    {
        /// Top of the pipeline (earliest possible stage).
        /// Typically used for global memory dependencies before any commands are executed.
        _TOP_PIPE_ = 0x00000001,

        /// Stage where indirect draw/dispatch parameters are read from buffers.
        /// Synchronize if indirect command data is modified.
        _DRAW_INDIRECT_ = 0x00000002,

        /// Stage where vertex/index buffers are read.
        /// Synchronize when vertex/index data changes before draw.
        _VERTEX_INPUT_ = 0x00000004,

        /// Vertex shader execution stage.
        /// Synchronize when resources used by vertex shaders are modified.
        _VERTEX_SHADER_ = 0x00000008,

        /// Tessellation control (hull) shader execution stage.
        _TESSELLATION_CONTROL_SHADER_ = 0x00000010,

        /// Tessellation evaluation (domain) shader execution stage.
        _TESSELLATION_EVALUATION_SHADER_ = 0x00000020,

        /// Geometry shader execution stage.
        _GEOMETRY_SHADER_ = 0x00000040,

        /// Fragment (pixel) shader execution stage.
        /// Synchronize when resources used in pixel shading are modified.
        _FRAGMENT_SHADER_ = 0x00000080,

        /// Early fragment tests stage (e.g., depth/stencil tests before fragment shading).
        _EARLY_FRAGMENT_TESTS_ = 0x00000100,

        /// Late fragment tests stage (e.g., depth/stencil tests after fragment shading).
        _LATE_FRAGMENT_TESTS_ = 0x00000200,

        /// Color attachment output stage.
        /// Synchronize when writing to color attachments in a framebuffer.
        _COLOR_ATTACHMENT_OUTPUT_ = 0x00000400,

        /// Compute shader execution stage.
        _COMPUTE_SHADER_ = 0x00000800,

        /// Transfer stage (copy, blit, clear, resolve operations).
        _TRANSFER_ = 0x00001000,

        /// Bottom of the pipeline (latest possible stage).
        /// Used for global memory dependencies after all commands are executed.
        _BOTTOM_PIPE_ = 0x00002000,

        /// Host (CPU) access stage.
        /// Synchronize with memory that will be read/written by the CPU.
        _HOST_ = 0x00004000,

        /// All graphics pipeline stages.
        /// Shorthand for all fixed-function and programmable stages involved in graphics.
        _ALL_GRAPHICS_ = 0x00008000,

        /// All command stages.
        /// Synchronize with all possible stages on the device.
        _ALL_COMMANDS_ = 0x00010000,
    };

    /// @brief Memory access flags.
    ///
    /// Defines the type of memory access performed by a pipeline stage (read/write operations).
    /// These flags are used in synchronization primitives (memory barriers, buffer/image barriers)
    /// to specify which types of memory accesses must complete before subsequent operations begin.
    ///
    /// Based on Vulkan's VkAccessFlagBits:
    /// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
    ///
    /// Multiple flags can be combined using bitwise OR.
    enum class Access : std::uint32_t
    {
        /// Read access to indirect command buffer parameters.
        /// Synchronize if indirect draw/dispatch parameters are written before use.
        _INDIRECT_COMMAND_READ_ = 0x00000001,

        /// Read access to index buffer data during vertex processing.
        _INDEX_READ_ = 0x00000002,

        /// Read access to vertex attribute data (vertex buffers).
        _VERTEX_ATTRIBUTE_READ_ = 0x00000004,

        /// Read access to uniform buffer data.
        _UNIFORM_READ_ = 0x00000008,

        /// Read access to input attachments within a fragment shader.
        _INPUT_ATTACHMENT_READ_ = 0x00000010,

        /// Read access to resources (buffers/images) in any shader stage.
        _SHADER_READ_ = 0x00000020,

        /// Write access to resources (buffers/images) in any shader stage.
        _SHADER_WRITE_ = 0x00000040,

        /// Read access to color attachments during blending or other read-modify-write operations.
        _COLOR_ATTACHMENT_READ_ = 0x00000080,

        /// Write access to color attachments during rendering.
        _COLOR_ATTACHMENT_WRITE_ = 0x00000100,

        /// Read access to depth/stencil attachments (e.g., during depth testing).
        _DEPTH_STENCIL_ATTACHMENT_READ_ = 0x00000200,

        /// Write access to depth/stencil attachments (e.g., during depth or stencil writes).
        _DEPTH_STENCIL_ATTACHMENT_WRITE_ = 0x00000400,

        /// Read access during transfer operations (copy/blit/resolve source).
        _TRANSFER_READ_ = 0x00000800,

        /// Write access during transfer operations (copy/blit/resolve destination).
        _TRANSFER_WRITE_ = 0x00001000,

        /// Read access by the host (CPU) via mapped memory.
        _HOST_READ_ = 0x00002000,

        /// Write access by the host (CPU) via mapped memory.
        _HOST_WRITE_ = 0x00004000,

        /// Read access to any memory (general-purpose).
        _MEMORY_READ_ = 0x00008000,

        /// Write access to any memory (general-purpose).
        _MEMORY_WRITE_ = 0x00010000
    };

    /// @brief Defines the primitive topology used for rendering.
    /// Specifies how the GPU interprets vertex data when assembling primitives.
    enum class PrimitiveTopology : std::uint32_t
    {
        _POINT_          = 0U, ///< Each vertex is rendered as an individual point.
        _LINE_           = 1U, ///< Each pair of vertices forms an individual line segment.
        _LINE_STRIP_     = 2U, ///< A connected series of line segments.
        _TRIANGLE_       = 3U, ///< Each group of three vertices forms a separate triangle.
        _TRIANGLE_STRIP_ = 4U, ///< A connected strip of triangles sharing vertices.
        _TRIANGLE_FAN_   = 5U, ///< A fan-shaped set of triangles sharing a central vertex.
    };

    /// @brief Specifies the polygon rasterization mode.
    /// Determines how primitives are filled or outlined during rasterization.
    enum class PrimitivePolygonMode : std::uint32_t
    {
        _FILL_  = 0U, ///< Fill the interior of the polygon.
        _LINE_  = 1U, ///< Draw only the polygon edges (wireframe mode).
        _POINT_ = 2U, ///< Render only the polygon vertices as points.
    };

    /// @brief Depth comparison function used in depth testing.
    /// Determines whether a fragment passes the depth test based on its depth value.
    enum class DepthTestFunction : std::uint32_t
    {
        _NEVER_            = 0U, ///< Always fails the depth test.
        _LESS_             = 1U, ///< Passes if the fragment's depth is less than the stored depth.
        _EQUAL_            = 2U, ///< Passes if the fragment's depth is equal to the stored depth.
        _LESS_OR_EQUAL_    = 3U, ///< Passes if the fragment's depth is less than or equal to the stored depth.
        _GREATER_          = 4U, ///< Passes if the fragment's depth is greater than the stored depth.
        _NOT_EQUAL_        = 5U, ///< Passes if the fragment's depth is not equal to the stored depth.
        _GREATER_OR_EQUAL_ = 6U, ///< Passes if the fragment's depth is greater than or equal to the stored depth.
        _ALWAYS_           = 7U, ///< Always passes the depth test.
    };

    ///
    /// @brief Pipeline configuration structure.
    ///
    /// Holds all parameters required to create a specific pipeline instance.
    struct Specification
    {
        std::string                  name;          ///< Debug name
        std::shared_ptr<Shader>      shader;        ///< Linked shader program
        std::shared_ptr<FrameBuffer> frame_buffer;  ///< Framebuffer target
        VertexBuffer::Layout         vertex_layout; ///< Vertex buffer layout
        PrimitiveTopology            primitive_topology{PrimitiveTopology::_TRIANGLE_};
        PrimitivePolygonMode         primitive_polygon_mode{PrimitivePolygonMode::_FILL_};
        DepthTestFunction            depth_test{DepthTestFunction::_LESS_OR_EQUAL_};
        bool                         is_back_face_culling{true};      ///< Enable back-face culling
        bool                         is_depth_clamping{false};        ///< Enable depth clamping
        bool                         is_depth_test_enabled{true};     ///< Enable depth testing
        float                        depth_bias_constant_factor{0.f}; ///< Depth bias constant
        float                        depth_bias_clamp{0.f};           ///< Depth bias clamp
        float                        depth_bias_slope_factor{0.f};    ///< Depth bias slope
        float                        line_width{2.f};                 ///< Line rendering width
    };

public:
    virtual ~Pipeline()                    = default;
    Pipeline(Pipeline const&)              = delete;
    Pipeline(Pipeline&&)                   = delete;
    Pipeline& operator=(Pipeline const&) & = delete;
    Pipeline& operator=(Pipeline&&) &      = delete;

    /// Bind pipeline to the given render command buffer
    ///
    /// @param[in] render_command_buffer Render command buffer
    /// @param[in] frame_index           Frame index
    virtual void bind(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index) = 0;

    /// Get pipeline specification.
    ///
    /// @return Pipeline::Specification
    Specification const& getSpecification() const
    {
        return specification_;
    }

    /// If pipeline is active.
    ///
    /// @return bool
    bool isActive() const
    {
        return is_active_;
    }

    /// Set pipeline active
    ///
    /// @param[in] active Set active flag
    void setActive(bool const active)
    {
        is_active_ = active;
    }

    /// Recompile pipeline
    virtual void recompile() = 0;

protected:
    /// Attach storage buffer to the current pipline.
    ///
    /// @param[in] storage_buffer   Storage buffer
    /// @param[in] set              Pipeline set
    /// @param[in] offset           Offset to the first element witing the buffer (In case we want to get accsess to specific buffer's range)
    virtual void setStorageBuffer(std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Set const set,
                                  std::uint32_t const offset = 0U) = 0;
    /// Attach uniform buffer to the current pipline.
    ///
    /// @param[in] uniform_buffer   Uniform buffer
    /// @param[in] set              Pipeline set
    /// @param[in] offset           Offset to the first element witing the buffer (In case we want to get accsess to specific buffer's range)
    virtual void setUniformBuffer(std::shared_ptr<UniformBuffer const> const uniform_buffer, Pipeline::Set const set,
                                  std::uint32_t const offset = 0U) = 0;
    /// Attach texture to the current pipline.
    ///
    /// @param[in] storage_buffer   Texture
    /// @param[in] set              Pipeline set
    /// @param[in] array_index      Array index of texture's array (Use only if shader has texture's array at current set, othervice keep as 0)
    virtual void setTexture(std::shared_ptr<Texture2D const> const texture, Pipeline::Set const set, std::uint32_t const binding,
                            std::size_t const array_index = 0U) = 0;
    /// Write uniform right thru to the current pipline (Aka Push constant), instatn version.
    ///
    /// @param[in] render_command_buffer Render command buffer
    /// @param[in] size Uniform size
    /// @param[in] data Pointer to data that we want to use
    /// @param[in] shader_stage  Shader::Stage (Vertex, fragment, geometry...)                                    = 0;
    virtual void setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size, void const* data,
                              Shader::Stage const shader_stage, std::uint32_t const frame_index) = 0;
    /// Write uniform to the current pipline (Aka Push constant).
    ///
    /// @param[in] render_command_buffer Render command buffer
    /// @param[in] size Uniform size
    /// @param[in] data Pointer to data that we want to use
    /// @param[in] shader_stage  Shader::Stage (Vertex, fragment, geometry...)
    virtual void setUniform(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size, void const* data,
                            Shader::Stage const shader_stage) = 0;
    /// Make all previously set resources available.
    ///
    /// @param[in] render_command_buffer Render command buffer
    virtual void updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer) = 0;

protected:
    /// Constructor for Pipeline
    ///
    /// @param[in] specification  Pipeline specification
    explicit Pipeline(Specification const& specification);
    Specification specification_;      ///< Pipeline configuration
    bool          is_active_{true};    ///< Active state flag
    std::uint32_t uniform_offset_{0U}; ///< Uniform push offset (unused)
};

/// @brief Rendering pipeline specialization.
///
/// Extends Pipeline for traditional graphics rendering (vertex/fragment/geometry stages).
class VSHADE_API RenderPipeline : public Pipeline
{
    friend class vshade::render::Render;
    friend class utility::CRTPFactory<RenderPipeline>;

public:
    ///  @brief Callback function type for processing draw commands.
    ///
    ///  @param pipeline             Pointer to the RenderPipeline instance.
    ///  @param render_command_buffer Command buffer to record rendering commands.
    ///  @param drawable_materials   Map of materials and associated drawable instances.
    ///  @param frame_data           Frame-specific submitted data.
    ///  @param frame_index          Current frame index in flight.
    ///  @param is_clear             Did clear has been requested.
    using ProcessCallback = std::function<void(std::shared_ptr<RenderPipeline> const, std::shared_ptr<RenderCommandBuffer> const,
                                               data::DrawableMaterialMap const&, data::SubmittedFrameData const&, std::uint32_t const, bool const)>;

public:
    virtual ~RenderPipeline()                          = default;
    RenderPipeline(RenderPipeline const&)              = delete;
    RenderPipeline(RenderPipeline&&)                   = delete;
    RenderPipeline& operator=(RenderPipeline const&) & = delete;
    RenderPipeline& operator=(RenderPipeline&&) &      = delete;

    /// @brief Create a new RenderPipeline instance.
    ///
    /// @param specification Pipeline::Specification
    static std::shared_ptr<RenderPipeline> create(Specification const& specification);

    /// Get process callback function.
    ///
    /// @return RenderPipeline::ProcessCallback
    ProcessCallback& getProcessCallback()
    {
        return process_callback_;
    }

    /// Set process callback function.
    ///
    /// @param[in] callback RenderPipeline::ProcessCallback
    void setProcessCallback(ProcessCallback& callback)
    {
        process_callback_ = std::move(callback);
    }

protected:
    /// Constructor for Pipeline
    ///
    /// @param[in] specification  Pipeline specification
    explicit RenderPipeline(Specification const& specification);

    ProcessCallback process_callback_; ///< Callback function invoked during rendering.
};

/// @brief Compute pipeline specialization.
///
/// Extends Pipeline for GPU compute workloads (no fixed-function graphics stages).
class VSHADE_API ComputePipeline : public Pipeline
{
    friend class vshade::render::Render;
    friend class utility::CRTPFactory<ComputePipeline>;

public:
    /// @brief Callback function type for processing compute work.
    ///
    /// @param pipeline              Pointer to the ComputePipeline instance.
    /// @param render_command_buffer Command buffer to record compute commands.
    /// @param frame_index           Current frame index in flight.
    using ProcessCallback =
        std::function<void(std::shared_ptr<ComputePipeline> const, std::shared_ptr<RenderCommandBuffer> const, std::uint32_t const)>;

public:
    virtual ~ComputePipeline()                           = default;
    ComputePipeline(ComputePipeline const&)              = delete;
    ComputePipeline(ComputePipeline&&)                   = delete;
    ComputePipeline& operator=(ComputePipeline const&) & = delete;
    ComputePipeline& operator=(ComputePipeline&&) &      = delete;

    /// @brief Create a new ComputePipeline instance.
    ///
    /// @param specification Pipeline::Specification
    static std::shared_ptr<ComputePipeline> create(Specification const& specification);

    /// Get process callback function.
    ///
    /// @return ComputePipeline::ProcessCallback
    ProcessCallback& getProcessCallback()
    {
        return process_callback_;
    }

    /// Set process callback function.
    ///
    /// @param[in] callback ComputePipeline::ProcessCallback
    void setProcessCallback(ProcessCallback& callback)
    {
        process_callback_ = std::move(callback);
    }

protected:
    /// @brief Dispatch compute shader workgroups.
    ///
    /// @param render_command_buffer Command buffer to record commands.
    /// @param group_count_x         Number of workgroups in X dimension.
    /// @param group_count_y         Number of workgroups in Y dimension.
    /// @param group_count_z         Number of workgroups in Z dimension.
    virtual void dispatch(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const group_count_x,
                          std::uint32_t const group_count_y, std::uint32_t const group_count_z) = 0;

protected:
    /// Constructor for Pipeline
    ///
    /// @param[in] specification  Pipeline specification
    explicit ComputePipeline(Specification const& specification);

    ProcessCallback process_callback_; ///< Callback function invoked during rendering.
};

///@brief Increment operator for Pipeline::Set.
///
/// Advances to the next enum value.
inline Pipeline::Set& operator++(Pipeline::Set& set)
{
    set = static_cast<Pipeline::Set>(static_cast<std::underlying_type_t<Pipeline::Set>>(set) + 1);
    return set;
}

/// @brief Less-than comparison for Pipeline::Set.
inline bool operator<(Pipeline::Set lhs, Pipeline::Set rhs)
{
    return static_cast<std::underlying_type_t<Pipeline::Set>>(lhs) < static_cast<std::underlying_type_t<Pipeline::Set>>(rhs);
}

} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_PIPELINE_H
