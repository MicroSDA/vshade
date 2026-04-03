#ifndef ENGINE_CORE_RENDER_RENDER_H
#define ENGINE_CORE_RENDER_RENDER_H

#include <atomic>
#include <deque>
#include <engine/core/logs/loger.h>
#include <engine/core/render/buffers/index_buffer.h>
#include <engine/core/render/buffers/render_command_queue.h>
#include <engine/core/render/buffers/vertex_buffer.h>
#include <engine/core/render/drawable/2d/sprite.h>
#include <engine/core/render/drawable/drawable.h>
#include <engine/core/render/drawable/primitives/box.h>
#include <engine/core/render/pipeline.h>
#include <engine/core/render/render_api.h>
#include <engine/core/render/render_command_buffer.h>
#include <engine/core/render/render_context.h>
#include <engine/core/render/swap_chain.h>
#include <engine/core/utility/singleton.h>

namespace vshade
{
class Application;

namespace render
{
/// Bindings for GPU global resources (accessible to all draw calls)
enum GlobalBindings : std::uint32_t
{
    _CAMERA_BUFFER_BINDING_,
    _MATERIAL_BUFFER_BINDING_,
    _MATERIAL_TEXTURES_BINDING_
};
/// Bindings for per-instance GPU resources (data changes for each rendered instance)
enum PerInstanceBindings : std::uint32_t
{
    _TRANSFORM_BUFFER_BINDING_,
};
// Bindings for textures used in materials
enum MaterialTexturesBindings : std::uint32_t
{
    _ALBEDO_TEXTURE_INDEX_,
    _DIFFUSE_TEXTURE_INDEX_,
    _SPECULAR_TEXTURE_INDEX_,
    _NORMAL_MAP_TEXTURE_INDEX_
};

/// @brief Main renderer class.
///
/// Defines the common interface for all rendering implementations.
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
/// - Define the interface that all derived render classes must implement.
class VSHADE_API Render : public utility::CRTPSingleton<Render>
{
    friend class utility::CRTPSingleton<Render>;
    friend class vshade::Application;

    // Hide creation/destruction functions to enforce singleton pattern
    using utility::CRTPSingleton<Render>::create;
    using utility::CRTPSingleton<Render>::destroy;

public:
    // Static table of shader binding definitions and their corresponding binding indices
    // Used for defining constants in shaders (matching CPU-side bindings with GPU-side shader definitions)
    static constexpr std::pair<char const*, std::uint32_t> _SHADER_BINDINGS_AND_DEFINITIONS_[] = {
        {"_GLOBAL_SET_", static_cast<std::uint32_t>(Pipeline::Set::_GLOBAL_)},
        {"_PER_INSTANCE_SET_", static_cast<std::uint32_t>(Pipeline::Set::_PER_INSTANCE_)},
        {"_USER_SET_", static_cast<std::uint32_t>(Pipeline::Set::_USER_)},

        {"_TRANSFORM_BUFFER_BINDING_INDEX_", _TRANSFORM_BUFFER_BINDING_},
        {"_CAMERA_BUFFER_BINDING_INDEX_", _CAMERA_BUFFER_BINDING_},
        {"_MATERIAL_BUFFER_BINDING_INDEX_", _MATERIAL_BUFFER_BINDING_},
        {"_MATERIAL_TEXTURES_BINDING_INDEX_", _MATERIAL_TEXTURES_BINDING_},

        {"_ALBEDO_TEXTURE_INDEX_", _ALBEDO_TEXTURE_INDEX_},
        {"_DIFFUSE_TEXTURE_INDEX_", _DIFFUSE_TEXTURE_INDEX_},
        {"_SPECULAR_TEXTURE_INDEX_", _SPECULAR_TEXTURE_INDEX_},
        {"_NORMAL_MAP_TEXTURE_INDEX_", _NORMAL_MAP_TEXTURE_INDEX_},

        {"_MAX_SAMPLED_IMAGES_", 10000U},
    };

    /// Submit policy
    enum class SubmitPolicy
    {
        _DEFAULT_, ///< Static data, render buffer will be created once
        _STREAM_   /// < Dynamic data, each submition set new data into render buffers
    };

public:
    virtual ~Render()                  = default;
    Render(Render const&)              = delete;
    Render(Render&&)                   = delete;
    Render& operator=(Render const&) & = delete;
    Render& operator=(Render&&) &      = delete;

    /// Create the renderer instance.
    ///
    /// @param api               Rendering API (e.g., Vulkan, OpenGL).
    /// @param frames_in_fligh   Number of frames in flight supported by the renderer.
    static void create(const API api, std::uint32_t const frames_in_fligh);
    /// Destroy the renderer instance and release resources.
    static void destroy();
    /// Register a new pipeline and associate a process callback.
    ///
    /// @tparam T                   Pipeline type (must be derived from RenderPipeline).
    /// @param[in] specification    Pipeline creation specification.
    /// @param[in] process_callback Function to process pipeline rendering.
    /// @return Shared pointer to the created pipeline.
    template <typename T>
    std::shared_ptr<T> registerNewPipeline(Pipeline::Specification const& specification, typename T::ProcessCallback&& process_callback)
    {
        std::shared_ptr<T> pipeline{T::create(specification)};
        pipeline->setProcessCallback(process_callback);
        registered_pipelines_.emplace(specification.name, pipeline);
        return pipeline;
    }
    /// Get reference to all registered pipelines.
    ///
    /// @return Map of pipeline name to pipeline instance.
    ankerl::unordered_dense::map<std::string, std::shared_ptr<Pipeline>>& getRegisterdPiplines()
    {
        return registered_pipelines_;
    }
    /// Get a specific registered pipeline by name.
    ///
    /// @param[in] name  Name of the pipeline.
    /// @return Shared pointer to the pipeline, or nullptr if not found.
    std::shared_ptr<Pipeline> getRegisterdPipline(std::string const& name)
    {
        auto pipeline{registered_pipelines_.find(name)};

        if (pipeline != registered_pipelines_.end())
        {
            return pipeline->second;
        }
        return nullptr;
    }
    /// Begin render pass for the specified pipeline.
    ///
    /// @param render_command_buffer  Command buffer to record rendering commands into.
    /// @param render_pipeline        Pipeline to be used for rendering.
    /// @param is_clear               Whether to clear framebuffers before rendering.
    /// @param clear_count            Number of clear attachments (color/depth).
    virtual void beginRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<RenderPipeline> const render_pipeline,
                             bool const is_clear, std::uint32_t const clear_count) = 0;
    /// End the current render pass.
    ///
    /// @param render_command_buffer  Command buffer used in beginRender().
    virtual void endRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer) = 0;
    /// Call a draw call for the specified vertex/index buffers.
    ///
    /// @param render_command_buffer  Command buffer for recording commands.
    /// @param vertex_buffer          Vertex buffer to draw.
    /// @param index_buffer           Index buffer to draw.
    /// @param instance_count         Number of instances to render.
    virtual void drawRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<VertexBuffer const> const vertex_buffer,
                        std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const frame_index,
                        std::uint32_t const instance_count) = 0;
    /// Call a draw call for the specified vertex/index buffers.
    ///
    /// @param render_command_buffer  Command buffer for recording commands.
    /// @param vertex_buffer          Vertex buffer to draw.
    /// @param index_buffer           Index buffer to draw.
    /// @param instance_count         Number of instances to render.
    virtual void draw(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<VertexBuffer const> const vertex_buffer,
                      std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const instance_count) = 0;
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
                            Pipeline::Access const dst_accces, std::uint32_t const mip = 0U) = 0;
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
                            Pipeline::Stage const dst_stage, Pipeline::Access const src_access, Pipeline::Access const dst_accces) = 0;
    /// Begin timestamp query in render thread.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_index            Frame index in flight.
    /// @param name                   Name of the timestamp query.
    virtual void beginTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                  std::string const& name) = 0;
    /// @brief End timestamp query in render thread.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_index            Frame index in flight.
    /// @param name                   Name of the timestamp query.
    virtual void endTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                std::string const& name) = 0;
    /// @brief Retrieve GPU timestamp query result for a given frame.
    ///
    /// @param name         Query name.
    /// @param frame_index  Frame index in flight.
    /// @return Query result in milliseconds.
    virtual double getQueryResult(std::string const& name, std::uint32_t const frame_index) const = 0;
    /// Retrieve latest GPU timestamp query result.
    ///
    /// @param name  Query name.
    /// @return Query result in milliseconds.
    virtual double getQueryResult(std::string const& name) const = 0;
    /// @brief Clear frame buffer color and depth attachments
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param frame_buffer  Frame buffer to clear
    virtual void clearFrameBufferAttachments(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                             std::shared_ptr<FrameBuffer> const         frame_buffer) = 0;
    /// Begin scene rendering with a given camera.
    ///
    /// @param camera                 Active camera.
    /// @param render_command_buffer  Command buffer.
    void beginScene(std::shared_ptr<Camera> const camera, std::shared_ptr<RenderCommandBuffer> const render_command_buffer);
    /// End current scene rendering.
    ///
    /// @param render_command_buffer  Command buffer.
    void endScene(std::shared_ptr<RenderCommandBuffer> const render_command_buffer);
    /// Begin compute pass for the specified pipeline.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param compute_pipeline       Compute pipeline to bind.
    void beginCompute(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<ComputePipeline> const compute_pipeline);
    /// Dispatch compute work.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Compute pipeline.
    /// @param group_count_x          Work group count in X.
    /// @param group_count_y          Work group count in Y.
    /// @param group_count_z          Work group count in Z.
    void dispatch(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<ComputePipeline> const pipeline,
                  std::uint32_t const group_count_x, std::uint32_t const group_count_y, std::uint32_t const group_count_z);
    /// End compute pass.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param compute_pipeline       Compute pipeline.
    /// @param is_submit              Whether to submit compute commands immediately.
    void endCompute(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<ComputePipeline> const compute_pipeline,
                    bool const is_submit = false);
    /// Submit drawable instance to the renderer for later execution.
    ///
    /// @param pipeline       Render pipeline.
    /// @param drawable       Drawable object.
    /// @param material       Material used for rendering.
    /// @param transform      Model transformation matrix.
    /// @param policy         Submit policy (_DEFAULT_ or _STREAM_).
    /// @param split_offset   Uses as an adition split offset(Mostly used in point shadow pass to specify side of cube).
    void submit(std::shared_ptr<RenderPipeline const> const pipeline, std::shared_ptr<Drawable const> const instance,
                std::shared_ptr<Material const> const material, glm::mat4 const& transform, SubmitPolicy const policy = SubmitPolicy::_DEFAULT_,
                std::size_t const split_offset = 0U);
    /// Submit raw vertex/index buffers to the renderer for later execution.
    ///
    /// @param pipeline       Render pipeline.
    /// @param vertex_buffer  Vertex buffer.
    /// @param index_buffer   Index buffer.
    /// @param material       Material used for rendering.
    /// @param transform      Model transformation matrix.
    /// @param split_offset   Uses as an adition split offset(Mostly used in point shadow pass to specify side of cube).
    void submit(std::shared_ptr<RenderPipeline const> const pipeline, std::shared_ptr<VertexBuffer const> const vertex_buufer,
                std::shared_ptr<IndexBuffer const> const index_buffer, std::shared_ptr<Material const> const material, glm::mat4 const& transform,
                std::size_t const split_offset = 0U);
    /// Draw a specific submitted instance.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Pipeline associated with the instance.
    /// @param instance               Instance index.
    /// @param split_offset           Instance buffer offset.
    void drawSubmitedInstance(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<RenderPipeline> const pipeline,
                              std::size_t const instance, std::size_t split_offset = 0U);
    /// Execute all submitted instances for a specific render pipeline.
    ///
    /// @param pipeline            Render pipeline.
    /// @param render_command_buffer  Command buffer.
    /// @param is_force_clear      Whether to force clear before execution.
    /// @return true if pipeline had submitted instances to execute and is active.
    bool executeSubmittedRenderPipeline(std::shared_ptr<RenderPipeline> const      pipeline,
                                        std::shared_ptr<RenderCommandBuffer> const render_comand_buffer, bool is_force_clear = false);
    /// Execute all submitted instances for a compute pipeline.
    ///
    /// @param pipeline            Compute pipeline.
    /// @param render_command_buffer  Command buffer.
    /// @return true if pipeline is active.
    bool executeSubmittedComputePipeline(std::shared_ptr<ComputePipeline> const     pipeline,
                                         std::shared_ptr<RenderCommandBuffer> const render_comand_buffer);
    /// Bind transform data for a submitted instance.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Render pipeline.
    /// @param instance               Instance index.
    /// @param split_offset           Uses as an adition split offset(Mostly used in point shadow pass to specify side of cube).
    void setSubmittedTransforms(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<RenderPipeline> pipeline,
                                std::size_t const instance, std::size_t const split_offset = 0U);
    /// Bind material data for a submitted instance.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Render pipeline.
    /// @param instance               Instance index.
    /// @param split_offset           Uses as an adition split offset(Mostly used in point shadow pass to specify side of cube).
    void setSubmittedMaterials(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<RenderPipeline> const pipeline,
                               std::size_t const instance, std::size_t const split_offset = 0U);
    /// Enqueue a uniform buffer update command for the pipeline.
    ///
    /// @tparam T         Type of the uniform data.
    /// @tparam stage     Shader stage to bind the data to.
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Target pipeline.
    /// @param data                   Pointer to the uniform data.
    template <typename T, typename Shader::Stage stage>
    void setUniform(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<Pipeline> const pipeline, T const* data)
    {
        enqueCommand([this, render_command_buffer_ = render_command_buffer, pipeline_ = pipeline, data_ = *data,
                      stage_ = stage](std::uint32_t const frame_index)
                     { setUniformRT(render_command_buffer_, pipeline_, sizeof(T), &data_, stage_, frame_index); });
    }
    /// Set a uniform in render thread immediately.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Target pipeline.
    /// @param size                   Size of the uniform data in bytes.
    /// @param data                   Pointer to the uniform data.
    /// @param shader_stage           Shader stage to bind the data to.
    /// @param frame_index            Frame index in flight.
    void setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<Pipeline> const pipeline,
                      std::size_t const size, void const* data, Shader::Stage const shader_stage, std::uint32_t const frame_index);
    /// Attach a texture to a pipeline binding.
    ///
    /// @tparam binding   Binding index in the descriptor set.
    /// @tparam set       Descriptor set index.
    /// @param pipeline  Target pipeline.
    /// @param texture   Texture to bind.
    template <typename std::uint32_t binding, typename Pipeline::Set set>
    void setTexture(std::shared_ptr<Pipeline> const pipeline, std::shared_ptr<Texture2D const> const texture)
    {
        pipeline->setTexture(texture, set, binding);
    }
    /// Attach storage buffer to the current pipeline.
    ///
    /// @tparam set       Descriptor set index.
    /// @param pipeline        Target pipeline.
    /// @param storage_buffer  Storage buffer.
    /// @param offset          Offset within the buffer range.
    template <typename Pipeline::Set set>
    void setStorageBuffer(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<StorageBuffer const> const storage_buffer,
                          std::uint32_t const offset = 0U)
    {
        pipeline->setStorageBuffer(storage_buffer, set, offset);
    }
    /// Attach uniform buffer to the current pipeline.
    ///
    /// @tparam set        Descriptor set index.
    /// @param pipeline       Target pipeline.
    /// @param uniform_buffer Uniform buffer.
    /// @param offset         Offset within the buffer range.
    template <typename Pipeline::Set set>
    void setUniformBuffer(std::shared_ptr<Pipeline> pipeline, std::shared_ptr<UniformBuffer const> const uniform_buffer,
                          std::uint32_t const offset = 0U)
    {
        pipeline->setUniformBuffer(uniform_buffer, set, offset);
    }
    /// Update descriptor sets and resources for the given pipeline.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param pipeline               Target pipeline.
    void updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<Pipeline> pipeline);
    /// Set a texture to be used as the swap chain resolve image.
    ///
    /// @param[in] texture  Texture to resolve into the swap chain.
    void resolveIntoSwapChain(std::shared_ptr<Texture2D const> const texture);
    /// Begin timestamp query.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param name                   Name of the timestamp query.
    void beginTimestamp(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::string const& name);
    /// End timestamp query.
    ///
    /// @param render_command_buffer  Command buffer.
    /// @param name                   Name of the timestamp query.
    void endTimestamp(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::string const& name);
    /// Enqueue a render command into the render command queue.
    ///
    /// @tparam Command  Callable type for the command.
    /// @param command    Command to enqueue.
    /// @param sub_group  Optional subgroup classification.
    template <typename Command>
    void enqueCommand(Command&& command, RenderCommandQueue::Subgroup const sub_group = RenderCommandQueue::Subgroup::_DEFUALT_)
    {
        render_command_queues_[getCurrentPrepareFrameIndex()]->enqueue(sub_group, std::forward<Command>(command));
    }
    /// Execute all queued commands for the given frame index (synchronous).
    ///
    /// @param frame_index  Frame index in flight.
    void executeAllPendingCommands(std::uint32_t const frame_index)
    {
        render_command_queues_.at(frame_index)->executeAll(frame_index);
    }
    /// Execute all queued commands asynchronously (Unused).
    ///
    /// @param frame_index  Frame index in flight.
    void executeAllPendingCommandsAsync(std::uint32_t const frame_index)
    {
        render_command_queues_.at(frame_index)->executeAllAsync(frame_index);
    }
    /// Clear all queued commands for the given frame index.
    ///
    /// @param frame_index  Frame index in flight.
    void cleanAllPendingCommands(std::uint32_t const frame_index)
    {
        render_command_queues_[frame_index]->clearQueues();
    }
    /// Get index of the frame currently being prepared on CPU.
    ///
    /// @return Frame index in flight.
    std::uint32_t const getCurrentPrepareFrameIndex() const;

    /// Get index of the frame currently being rendered on GPU.
    ///
    /// @return Frame index in flight.
    std::uint32_t const getCurrentRenderFrameIndex() const;
    /// Get default material used by the renderer.
    ///
    /// @return Shared pointer to the default material.
    std::shared_ptr<Material> getDefaultMaterial() const
    {
        return default_material_;
    }
    /// Get the number of frames in flight supported by this renderer.
    ///
    /// @return Frames in flight count.
    std::uint32_t getFramesInFlightCount() const
    {
        return frames_in_flight_;
    }
    /// Block CPU until the current rendering is finished.
    void waitUntilRenderDone();

protected:
    /// Constructor for the renderer.
    ///
    /// @param[in] api               Rendering API.
    /// @param[in] frames_in_flight  Number of frames in flight supported.
    explicit Render(API api, std::uint32_t const frames_in_flight);
    /// Collect GPU query results for the given frame index.
    ///
    /// @param[in] frame_index  Frame index in flight.
    virtual void queryResults(std::uint32_t const frame_index) = 0;

protected:
    /// Index of the frame currently being processed in the "prepare" stage
    std::atomic<std::uint32_t> current_process_frame_index_{0U};

    /// Array of per-frame timestamps (used for removing deprecated buffers)
    std::vector<std::shared_ptr<std::atomic<std::int64_t>>> time_stamps_;

    /// Index of the frame currently being rendered; initialized to max value to indicate "no rendering yet"
    std::atomic<std::uint32_t> current_render_frame_index_{std::numeric_limits<std::uint32_t>::max()};

    // Graphics API currently in use (e.g., Vulkan, OpenGL)
    API const current_api_;

    // Mutex for synchronizing access to rendering-related data
    std::mutex render_mutex_;

    // Number of frames that can be processed/rendered in parallel ("frames in flight")
    std::uint32_t const frames_in_flight_;

    // Flag indicating whether a quit/exit request was made
    std::atomic<bool> is_quit_reqested_{false};

    // Condition variable used to wake the render thread when a frame is ready
    std::condition_variable render_condition_variable_;

    // Lock used for synchronizing frame preparation
    std::unique_lock<std::mutex> prepare_frame_lock_;

    // Lock used for synchronizing frame rendering
    std::unique_lock<std::mutex> render_frame_lock_;

    // Queue of frames that are prepared and waiting to be rendered
    std::deque<std::uint32_t> frames_in_flight_queue_;

    // Dedicated rendering thread
    std::shared_ptr<std::thread> render_thread_;

    // Texture used to resolve the final image before presenting it to the swap chain
    std::shared_ptr<Texture2D const> swap_chain_resolve_texture_;

    // List of GPU render command queues
    std::vector<std::shared_ptr<RenderCommandQueue>> render_command_queues_;

    // Global texture offset used for indexing bound textures
    std::size_t global_texture_offset_{0U};

    // Number of rendered instances in the current frame
    std::uint32_t rendered_instances_{0U};

    // Map of registered graphics pipelines (identified by string keys)
    ankerl::unordered_dense::map<std::string, std::shared_ptr<Pipeline>> registered_pipelines_;

    // Data structure containing information about a submitted frame
    data::SubmittedFrameData submitted_frame_data_;

    // Default material used when an object does not specify its own material
    std::shared_ptr<Material> default_material_;

private:
    /// Initialize renderer internals.
    ///
    /// @param[in] frames_in_flight  Number of frames in flight supported.
    void initialize(std::uint32_t const frames_in_flight);
    /// Begin preparing resources for a new frame on CPU.
    ///
    /// @return Current prepare frame index.
    std::uint32_t beginPrepareNewFrame();
    /// Finish CPU-side frame preparation.
    ///
    /// Availible only in application.
    void endPrepareNewFrame();
    /// Begin a new render frame on GPU.
    ///
    /// @return Current render frame index.
    std::uint32_t beginRenderFrame();
    /// Finish the current render frame on GPU.
    void endRenderFrame();
    /// Start the render thread loop.
    void processRenderThread();
    /// Signal the render thread to stop and wait for it.
    void closeRenderThread();
    /// Prepare internal offsets for per-frame buffers before rendering.
    void prepareBuffersOffsets();
    /// Create buffers for rendering a drawable instance.
    ///
    /// @param[in] instance  Drawable object.
    /// @param[in] policy    Submit policy.
    void createInstanceGeometryBuffer(std::shared_ptr<Drawable const> const instance, SubmitPolicy const policy);

    /// Upload geometry data for a drawable instance to GPU buffers.
    ///
    /// @param[in] instance  Drawable object.
    /// @param[in] buffers   GPU buffer references for geometry data.
    /// @param[in] policy    Submit policy.
    /// @param[in] offset    Offset in the instance buffer.
    void setDataInstanceGeometryBuffer(std::shared_ptr<Drawable const> const instance, data::InstanceGeometryRenderBuffers& buffers,
                                       SubmitPolicy const policy, std::uint32_t const offset);
};
} // namespace render
} // namespace vshade
#endif // ENGINE_CORE_RENDER_RENDER_H