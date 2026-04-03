#include "engine/core/render/buffers/render_command_queue.h"

vshade::render::RenderCommandQueue::RenderCommandQueue()
{
    for (auto& queue : subqueues_)
    {
        queue.buffer = std::make_unique<std::byte[]>(BUFFER_SIZE);
        memset(queue.buffer.get(), 0, sizeof(std::byte) * BUFFER_SIZE);
        memset(queue.commands.data(), 0, sizeof(ICommand*) * MAX_COMMANDS);
    }
}

vshade::render::RenderCommandQueue::~RenderCommandQueue()
{
    clearQueues();
}

void vshade::render::RenderCommandQueue::clearQueues()
{
    for (auto& queue : subqueues_)
    {
        std::scoped_lock lock(queue.mutex);

        for (std::size_t i{0U}; i < queue.count; ++i)
        {
            queue.commands[i]->~ICommand();
        }

        queue.count  = 0U;
        queue.offset = 0U;

        // memset(queue.buffer.get(), 0, sizeof(std::byte) * BUFFER_SIZE);
        // memset(queue.commands.data(), 0, sizeof(ICommand*) * MAX_COMMANDS);
    }    
}

void vshade::render::RenderCommandQueue::executeAll(std::uint32_t const frame_index) 
{
    for (auto& queue : subqueues_)
    {
        std::scoped_lock lock{queue.mutex};

        for (std::size_t i{0U}; i < queue.count; ++i)
        {
            queue.commands[i]->execute(frame_index);
            queue.commands[i]->~ICommand();
        }

        queue.count  = 0U;
        queue.offset = 0U;
    }
}

void vshade::render::RenderCommandQueue::executeAllAsync(std::uint32_t const frame_index)
{
    // TODO: Refactor for thread pool !!
    std::vector<std::thread> threads;

    for (auto& queue : subqueues_)
    {
        threads.emplace_back(
            [&queue, frame_index]()
            {
                std::scoped_lock lock(queue.mutex);
                for (std::size_t i{0U}; i < queue.count; ++i)
                {
                    queue.commands[i]->execute(frame_index);
                    queue.commands[i]->~ICommand();
                }
                queue.count  = 0U;
                queue.offset = 0U;
            });
    }

    for (auto& t : threads)
    {
        t.join();
    }
}