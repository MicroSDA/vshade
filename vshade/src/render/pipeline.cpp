#include "engine/core/render/pipeline.h"
#include <engine/platforms/render/vulkan/vulkan_compute_pipeline.h>
#include <engine/platforms/render/vulkan/vulkan_render_pipeline.h>

vshade::render::Pipeline::Pipeline(Specification const& specification) : specification_{specification}
{
}

vshade::render::RenderPipeline::RenderPipeline(Specification const& specification) : Pipeline(specification)
{
}

std::shared_ptr<vshade::render::RenderPipeline> vshade::render::RenderPipeline::create(Specification const& specification)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<RenderPipeline>::create<VulkanRenderPipeline>(specification);
    }
}

vshade::render::ComputePipeline::ComputePipeline(Specification const& specification) : Pipeline(specification)
{
}

std::shared_ptr<vshade::render::ComputePipeline> vshade::render::ComputePipeline::create(Specification const& specification)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<ComputePipeline>::create<VulkanComputePipeline>(specification);
    }
}
