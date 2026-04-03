#ifndef ENGINE_CORE_RENDER_BUFFERS_COMMON_BUFER_H
#define ENGINE_CORE_RENDER_BUFFERS_COMMON_BUFER_H

#include <cstdint>

namespace vshade
{
    namespace render
    {
        enum class BufferUsage : std::uint8_t
        {
            _GPU_,
            _CPU_GPU_
        };
    }
}

#endif // ENGINE_CORE_RENDER_BUFFERS_COMMON_BUFER_H