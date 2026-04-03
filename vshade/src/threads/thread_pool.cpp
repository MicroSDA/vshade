#include "engine/core/threads/thread_pool.h"


vshade::thread::ThreadPool::ThreadPool(std::size_t threads_count)
    : threads_count_{threads_count > 0 ? threads_count : std::thread::hardware_concurrency()} 
{
    initialize();
}
vshade::thread::ThreadPool::~ThreadPool()
{
    shutDown();
}
 
void vshade::thread::ThreadPool::shutDown() noexcept
{
    {
        std::unique_lock<std::mutex> lock{mutex_};
        quit_ = true;
    }

    event_.notify_all();

    for (auto& thread : threads_)
    {
        if (thread.joinable())
            thread.join();
    }
}

void vshade::thread::ThreadPool::initialize() 
{
    for (std::size_t i{0U}; i < threads_count_; ++i)
    {
        threads_.emplace_back(
            [this]
            {
                {
                    // std::unique_lock<std::mutex> lock{mutex_};
                    // threads_id_.emplace(std::hash<std::thread::id>{}(std::this_thread::get_id()), static_cast<ThreadName>(threads_id_.size()));
                }

                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock{mutex_};

                        event_.wait(lock, [this] { return quit_ || !tasks_.empty(); });

                        if (quit_ && tasks_.empty())
                        {
                            break;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    // Async part
                    task();
                    // !Async part
                }
            });
    }
}
