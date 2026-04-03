#include "engine/core/render/scene/scene_renderer.h"

vshade::render::MainSceneRenderer::MainSceneRenderer()
{
    // Vertex layout
    render::VertexBuffer::Layout MGVLS = {
        {render::VertexBuffer::Layout::Usage::_PER_VERTEX_,
         {
             {
                 "a_Position",
                 render::Shader::DataType::_FLOAT_3_,
             },
             {
                 "a_Normal",
                 render::Shader::DataType::_FLOAT_3_,
             },
             {
                 "a_Tangent",
                 render::Shader::DataType::_FLOAT_3_,
             },
             {
                 "a_BiTangent",
                 render::Shader::DataType::_FLOAT_3_,
             },
             {
                 "a_UV_Coordinates",
                 render::Shader::DataType::_FLOAT_2_,
             },
         }},
    };

    RenderPipeline::Specification main_pipleine_specification;
    Shader::Specification         main_shader_specification{"dummy", "/resources/assets/shaders/shader.glsl", std::vector<std::tuple<std::string, std::string>>{}};
    FrameBuffer::Specification    main_frame_buffer_specification;

    main_frame_buffer_specification.width       = 3000;
    main_frame_buffer_specification.height      = 2000;
    main_frame_buffer_specification.attachments = {{Image::Format::_RGBA32F_, Image::Usage::_ATTACHMENT_, Image::Clamp::_REPEAT_}};

    main_frame_buffer_specification.clear_color[0] = 0.1;
    main_frame_buffer_specification.clear_color[1] = 0.1;
    main_frame_buffer_specification.clear_color[2] = 0.1;

    main_pipleine_specification.name                 = "main";
    main_pipleine_specification.shader               = Shader::create(main_shader_specification, true);
    main_pipleine_specification.frame_buffer         = FrameBuffer::create(main_frame_buffer_specification);
    main_pipleine_specification.vertex_layout        = MGVLS;
    main_pipleine_specification.is_back_face_culling = false;
    main_pipleine_specification.primitive_topology   = RenderPipeline::PrimitiveTopology::_TRIANGLE_;

    main_render_pipeline_ = Render::instance().registerNewPipeline<RenderPipeline>(
        main_pipleine_specification, [this](auto render_pipeline, auto render_command_buffer, auto const& drawable_material_map,
                                            auto const& submitted_frame_fata, auto frame_index, bool is_force_clear)
        { basicDrawCallback(render_pipeline, render_command_buffer, drawable_material_map, submitted_frame_fata, frame_index, is_force_clear); });

    main_pipleine_specification.name = "off";
    off_render_pipeline_             = Render::instance().registerNewPipeline<RenderPipeline>(
        main_pipleine_specification, [this](auto render_pipeline, auto render_command_buffer, auto const& drawable_material_map,
                                            auto const& submitted_frame_fata, auto frame_index, bool is_force_clear)
        { basicDrawCallback(render_pipeline, render_command_buffer, drawable_material_map, submitted_frame_fata, frame_index, is_force_clear); });

    main_render_command_buffer_ =
        render::RenderCommandBuffer::create(render::RenderCommandBuffer::Type::_PRIMARY_, render::RenderCommandBuffer::Family::_GRAPHIC_, 3U);

    material_[0] = Material::create<Material>();
    material_[1] = Material::create<Material>();
    material_[2] = Material::create<Material>();
    material_[3] = Material::create<Material>();
    material_[4] = Material::create<Material>();

    if (auto file = file::FileManager::instance().getNativeFile("/resources/assets/texture1.dds", std::ios::binary))
    {
        render::Image image;
        serializer::Serializer::deserialize(file, image);

        material_[0]->texture_albedo = Texture2D::createExplicit(Image2D::create(image));
        material_[0]->color_ambient  = glm::vec3(0.5, 0.5, 0);
    }
    if (auto file = file::FileManager::instance().getNativeFile("/resources/assets/texture2.dds", std::ios::binary))
    {
        render::Image image;
        serializer::Serializer::deserialize(file, image);

        material_[1]->texture_albedo = Texture2D::createExplicit(Image2D::create(image));
        material_[1]->color_ambient  = glm::vec3(0.0, 0.0, 0.5);
    }
    if (auto file = file::FileManager::instance().getNativeFile("/resources/assets/texture3.dds", std::ios::binary))
    {
        render::Image image;
        serializer::Serializer::deserialize(file, image);

        material_[2]->texture_albedo = Texture2D::createExplicit(Image2D::create(image));
        material_[2]->color_ambient  = glm::vec3(0.0, 0.5, 0.0);
    }
    if (auto file = file::FileManager::instance().getNativeFile("/resources/assets/texture4.dds", std::ios::binary))
    {
        render::Image image;
        serializer::Serializer::deserialize(file, image);

        material_[3]->texture_albedo = Texture2D::createExplicit(Image2D::create(image));
        material_[3]->color_ambient  = glm::vec3(0.5, 0.0, 0.0);
    }
    if (auto file = file::FileManager::instance().getNativeFile("/resources/assets/texture5.dds", std::ios::binary))
    {
        render::Image image;
        serializer::Serializer::deserialize(file, image);

        material_[4]->texture_albedo = Texture2D::createExplicit(Image2D::create(image));
        material_[4]->color_ambient  = glm::vec3(1.0, 1.0, 1.0);
    }
    {
        Vertices vertices{3U};

        vertices[0].position = {0.0f, -0.25f, 2.f};
        vertices[1].position = {-0.2165f, 0.125f, 2.f};
        vertices[2].position = {0.2165f, 0.125f, 2.f};

        vertices[0].uv_coordinates = {0.5f, 0.0f}; // нижняя вершина (центр по X)
        vertices[1].uv_coordinates = {0.0f, 1.0f}; // левая верхняя вершина
        vertices[2].uv_coordinates = {1.0f, 1.0f}; // правая верхняя вершина

        Indices indicies{0, 1, 2};

        drawable_1_ = Drawable::create<Drawable>();
        drawable_1_->addVertices(vertices);
        drawable_1_->addIndices(indicies);
        drawable_1_->setMaterial(material_[0]);
    }
    {
        Vertices vertices{3U};
        vertices[0].position = {0.0f, 0.0f, 2.f};
        vertices[1].position = {-0.2165f, -0.375f, 2.f};
        vertices[2].position = {0.2165f, -0.375f, 2.f};

        vertices[0].uv_coordinates = {0.5f, 0.0f}; // нижняя вершина (центр по X)
        vertices[1].uv_coordinates = {0.0f, 1.0f}; // левая верхняя вершина
        vertices[2].uv_coordinates = {1.0f, 1.0f}; // правая верхняя вершина

        Indices indicies{0, 1, 2};

        drawable_2_ = Drawable::create<Drawable>();
        drawable_2_->addVertices(vertices);
        drawable_2_->addIndices(indicies);
        drawable_2_->setMaterial(material_[1]);
    }

    main_camera_ = Camera::create<Camera>();
}

void vshade::render::MainSceneRenderer::onUpdate(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer,
                                                 std::uint32_t const frame_index)
{
    {
        glm::mat4 transform{glm::identity<glm::mat4>()};

        transform[3] = glm::vec4{0.6f, 0.8f, 0.0f, 3.0f};

        vshade::render::Render::instance().submit(main_render_pipeline_, drawable_2_, material_[0], transform);

        transform[3] = glm::vec4{0.2f, 0.8f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(main_render_pipeline_, drawable_2_, material_[1], transform);

        transform[3] = glm::vec4{-0.2f, 0.8f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(main_render_pipeline_, drawable_2_, material_[2], transform);

        transform[3] = glm::vec4{-0.6f, 0.6f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(main_render_pipeline_, drawable_1_, material_[3], transform);
    }

    {
        glm::mat4 transform{glm::identity<glm::mat4>()};

        transform[3] = glm::vec4{0.6f, 0.4f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(off_render_pipeline_, drawable_2_, material_[3], transform);

        transform[3] = glm::vec4{0.2f, 0.4f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(off_render_pipeline_, drawable_2_, material_[1], transform);

        transform[3] = glm::vec4{-0.2f, 0.2f, 0.0f, 1.0f};

        vshade::render::Render::instance().submit(off_render_pipeline_, drawable_1_, material_[4], transform);
    }
}

void vshade::render::MainSceneRenderer::onRenderBegin(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
    render::Render::instance().beginScene(main_camera_, main_render_command_buffer_);
}

void vshade::render::MainSceneRenderer::onRender(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer,
                                                 std::uint32_t const frame_index)
{
    bool is_clear = !render::Render::instance().executeSubmittedRenderPipeline(main_render_pipeline_, main_render_command_buffer_, true);
    is_clear += !render::Render::instance().executeSubmittedRenderPipeline(off_render_pipeline_, main_render_command_buffer_, is_clear);
}

void vshade::render::MainSceneRenderer::onRenderEnd(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
    render::Render::instance().endScene(main_render_command_buffer_);

    render::Render::instance().resolveIntoSwapChain(main_render_pipeline_->getSpecification().frame_buffer->getColorAttachment(0));
}

void vshade::render::MainSceneRenderer::basicDrawCallback(std::shared_ptr<RenderPipeline>      render_pipeline,
                                                          std::shared_ptr<RenderCommandBuffer> render_command_buffer,
                                                          data::DrawableMaterialMap const&     drawable_material_map,
                                                          data::SubmittedFrameData const&      submitted_frame_fata, std::uint32_t const,
                                                          bool const                           is_force_clear)
{
    render::Render::instance().beginRender(render_command_buffer, render_pipeline, is_force_clear, 1U);

    for (auto& [instance, materials_per_models] : drawable_material_map)
    {
        Render::instance().setSubmittedTransforms(render_command_buffer, render_pipeline, instance);

        Render::instance().setSubmittedMaterials(render_command_buffer, render_pipeline, instance);

        Render::instance().drawSubmitedInstance(render_command_buffer, render_pipeline, instance);
    }

    render::Render::instance().endRender(render_command_buffer);
}
