#include "renderer/renderer.h"

editor::SceneRenderer::SceneRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer)
    : vshade::render::SceneRenderer{frame_buffer}, main_camera_{vshade::render::Camera::create<vshade::render::Camera>(vshade::render::Projection::_PERSPECTIVE_)}
{
    //------------------------------------------------------------------------
    // Grid vertex layout
    //------------------------------------------------------------------------
    // As far as we use Drawable as grid plane we need to follow
    // render::Vertex struct layout
    //------------------------------------------------------------------------
    vshade::render::VertexBuffer::Layout GVLS = {
        {vshade::render::VertexBuffer::Layout::Usage::_PER_VERTEX_,
         {
             {
                 "a_Position",
                 vshade::render::Shader::DataType::_FLOAT_3_, // vec3
             },
             {
                 "a_Normal",
                 vshade::render::Shader::DataType::_FLOAT_3_, // vec3
             },
             {
                 "a_Tangent",
                 vshade::render::Shader::DataType::_FLOAT_3_, // vec3
             },
             {
                 "a_BiTangent",
                 vshade::render::Shader::DataType::_FLOAT_3_, // vec3
             },
             {
                 "a_UV_Coordinates",
                 vshade::render::Shader::DataType::_FLOAT_2_, // vec2
             },
         }},
    };

    //------------------------------------------------------------------------
    // Frame buffer
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    // Create main frame buffer with one collor attachment (Assuming that
    // _RGBA16F_ should be supported on any devices)
    //------------------------------------------------------------------------
    main_frame_buffer_ = !main_frame_buffer_ ? vshade::render::FrameBuffer::create(vshade::render::FrameBuffer::Specification{
                                                   2U, // Width
                                                   2U, // Height
                                                   {
                                                       {vshade::render::Image::Format::_BGRA8UN_, vshade::render::Image::Usage::_ATTACHMENT_,
                                                        vshade::render::Image::Clamp::_CLAMP_TO_EDGE_},
                                                       {vshade::render::Image::Format::_DEPTH_, vshade::render::Image::Usage::_ATTACHMENT_,
                                                        vshade::render::Image::Clamp::_CLAMP_TO_EDGE_},
                                                   },
                                                   glm::vec4{0.f, 0.f, 0.f, 1.f}})
                                             : main_frame_buffer_;
    //------------------------------------------------------------------------
    // Grid render pipeline
    //------------------------------------------------------------------------
    vshade::render::RenderPipeline::Specification grid_pipleine_specification;
    grid_pipleine_specification.name = "Grid";
    grid_pipleine_specification.shader =
        vshade::render::Shader::create(vshade::render::Shader::Specification{"Grid", "/resources/assets/shaders/grid_shader.glsl",
                                                                             std::vector<std::tuple<std::string, std::string>>{}},
                                       true);
    grid_pipleine_specification.frame_buffer          = main_frame_buffer_;
    grid_pipleine_specification.vertex_layout         = GVLS;
    grid_pipleine_specification.is_back_face_culling  = false;
    grid_pipleine_specification.is_depth_test_enabled = true;
    grid_pipleine_specification.primitive_topology    = vshade::render::RenderPipeline::PrimitiveTopology::_TRIANGLE_STRIP_;

    grid_render_pipeline_ = vshade::render::Render::instance().registerNewPipeline<vshade::render::RenderPipeline>(
        grid_pipleine_specification, [this](auto render_pipeline, auto render_command_buffer, auto const& drawable_material_map,
                                            auto const& submitted_frame_fata, auto frame_index, bool is_force_clear)
        { gridDrawCallback(render_pipeline, render_command_buffer, drawable_material_map, submitted_frame_fata, frame_index, is_force_clear); });

    //------------------------------------------------------------------------
    // Color correction compute pipeline
    //------------------------------------------------------------------------
    vshade::render::Pipeline::Specification collor_correction_pipleine_specification;
    collor_correction_pipleine_specification.name   = "Color correction";
    collor_correction_pipleine_specification.shader = vshade::render::Shader::create(
        vshade::render::Shader::Specification{"Color correction", "/resources/assets/shaders/post_process/color_correction_shader.glsl",
                                              std::vector<std::tuple<std::string, std::string>>{}},
        true);
    color_correction_pipeline_ = vshade::render::Render::instance().registerNewPipeline<vshade::render::ComputePipeline>(
        collor_correction_pipleine_specification, [this](auto compute_pipeline, auto render_command_buffer, auto frame_index)
        { colorCorrectionCallback(compute_pipeline, render_command_buffer, frame_index); });
    color_correction_pipeline_->setActive(false);

    //------------------------------------------------------------------------
    // Render command buffer
    //------------------------------------------------------------------------
    render_command_buffer_ = vshade::render::RenderCommandBuffer::create(vshade::render::RenderCommandBuffer::Type::_PRIMARY_,
                                                                                     vshade::render::RenderCommandBuffer::Family::_GRAPHIC_,
                                                                                     vshade::render::Render::instance().getFramesInFlightCount());
    //------------------------------------------------------------------------
    // Other
    //------------------------------------------------------------------------
    grid_plane_ = vshade::render::Plane::create<vshade::render::Plane>(1.f, 1.f, 1U);

    //------------------------------------------------------------------------
    // Get shared resources
    //------------------------------------------------------------------------
    if (auto camera = *vshade::LayerResourceManager::instance().get<std::shared_ptr<vshade::render::Camera>>("Main camera"))
    {
        main_camera_ = camera;
    }

    vshade::LayerResourceManager::instance().declare("Color correction", &color_correction_);
}

void editor::SceneRenderer::onUpdate(std::shared_ptr<vshade::Scene> const scene, vshade::time::FrameTimer const& frame_timer,
                                               std::uint32_t const frame_index)
{
    //------------------------------------------------------------------------
    // Resize main frame buffer depends on swapchain
    //------------------------------------------------------------------------
    if (!is_external_frame_buffer_)
    {
        std::pair<std::uint32_t, std::uint32_t> const resolution{vshade::render::SwapChain::instance().getResolution()};

        if (main_frame_buffer_->getWidth() != resolution.first || main_frame_buffer_->getHeight() != resolution.second)
        {
            main_frame_buffer_->resize(resolution.first, resolution.second);
        }
    }

    //------------------------------------------------------------------------
    // Set camera aspect ratio depends on frame buffer size
    //------------------------------------------------------------------------
    main_camera_->setAspect(static_cast<float>(main_frame_buffer_->getWidth()), static_cast<float>(main_frame_buffer_->getHeight()));
    vshade::render::Render::instance().submit(grid_render_pipeline_, grid_plane_, nullptr, glm::identity<glm::mat4>());
}

void editor::SceneRenderer::onRenderBegin(vshade::time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
    vshade::render::Render::instance().beginScene(main_camera_, render_command_buffer_);
}

void editor::SceneRenderer::onRender(std::shared_ptr<vshade::Scene> const scene, vshade::time::FrameTimer const& frame_timer,
                                               std::uint32_t const frame_index)
{
    vshade::render::Render::instance().beginTimestamp(render_command_buffer_, "SceneRenderer");

    vshade::render::Render::instance().executeSubmittedRenderPipeline(grid_render_pipeline_, render_command_buffer_, true);

    vshade::render::Render::instance().endTimestamp(render_command_buffer_, "SceneRenderer");
}

void editor::SceneRenderer::onRenderEnd(vshade::time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
     // Set as swapchain resolve image
    vshade::render::Render::instance().resolveIntoSwapChain(main_frame_buffer_->getColorAttachment());
    vshade::render::Render::instance().endScene(render_command_buffer_);
}

void editor::SceneRenderer::gridDrawCallback(std::shared_ptr<vshade::render::RenderPipeline>      render_pipeline,
                                                       std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer,
                                                       vshade::render::data::DrawableMaterialMap const&     drawable_material_map,
                                                       vshade::render::data::SubmittedFrameData const&      submitted_frame_fata, std::uint32_t const,
                                                       bool const                                           is_force_clear)
{
    vshade::render::Render::instance().beginRender(render_command_buffer, render_pipeline, is_force_clear, 1U);

    for (auto& [instance, materials_per_models] : drawable_material_map)
    {
        vshade::render::Render::instance().setSubmittedTransforms(render_command_buffer, render_pipeline, instance);
        vshade::render::Render::instance().drawSubmitedInstance(render_command_buffer, render_pipeline, instance);
    }

    vshade::render::Render::instance().endRender(render_command_buffer);
}

void editor::SceneRenderer::flatDrawCallback(std::shared_ptr<vshade::render::RenderPipeline>      render_pipeline,
                                                       std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer,
                                                       vshade::render::data::DrawableMaterialMap const&     drawable_material_map,
                                                       vshade::render::data::SubmittedFrameData const&      submitted_frame_fata, std::uint32_t const,
                                                       bool const                                           is_force_clear)
{
    vshade::render::Render::instance().beginRender(render_command_buffer, render_pipeline, is_force_clear, 1U);

    for (auto& [instance, materials_per_models] : drawable_material_map)
    {
        vshade::render::Render::instance().setSubmittedMaterials(render_command_buffer, render_pipeline, instance);
        vshade::render::Render::instance().setSubmittedTransforms(render_command_buffer, render_pipeline, instance);
        vshade::render::Render::instance().drawSubmitedInstance(render_command_buffer, render_pipeline, instance);
    }

    vshade::render::Render::instance().endRender(render_command_buffer);
}

void editor::SceneRenderer::colorCorrectionCallback(std::shared_ptr<vshade::render::ComputePipeline>     compute_pipeline,
                                                              std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer,
                                                              std::uint32_t const)
{
    std::shared_ptr<vshade::render::Texture2D const> const texture{main_frame_buffer_->getColorAttachment()};
    glm::uvec3 const                                       execution_groups{std::ceil(static_cast<float>(texture->getSpecification().width) / 16.f),
                                      std::ceil(static_cast<float>(texture->getSpecification().height) / 16.f), 1U};

    vshade::render::Render::instance().beginCompute(render_command_buffer, compute_pipeline);
    // Send color corection values
    vshade::render::Render::instance().setUniform<ColorCorrection, vshade::render::Shader::Stage::_COMPUTE_>(render_command_buffer, compute_pipeline,
                                                                                                             &color_correction_);
    vshade::render::Render::instance().setTexture<0U, vshade::render::Pipeline::Set::_USER_>(compute_pipeline, texture);
    vshade::render::Render::instance().setTexture<1U, vshade::render::Pipeline::Set::_USER_>(compute_pipeline, texture);

    vshade::render::Render::instance().setBarrier(
        render_command_buffer, texture, vshade::render::Pipeline::Stage::_COLOR_ATTACHMENT_OUTPUT_, vshade::render::Pipeline::Stage::_COMPUTE_SHADER_,
        vshade::render::Pipeline::Access::_COLOR_ATTACHMENT_WRITE_, vshade::render::Pipeline::Access::_SHADER_READ_);
    vshade::render::Render::instance().dispatch(render_command_buffer, compute_pipeline, execution_groups.x, execution_groups.y, execution_groups.z);
    vshade::render::Render::instance().setBarrier(
        render_command_buffer, texture, vshade::render::Pipeline::Stage::_COMPUTE_SHADER_, vshade::render::Pipeline::Stage::_COLOR_ATTACHMENT_OUTPUT_,
        vshade::render::Pipeline::Access::_SHADER_WRITE_, vshade::render::Pipeline::Access::_COLOR_ATTACHMENT_WRITE_);

    vshade::render::Render::instance().endCompute(render_command_buffer, compute_pipeline);
}
