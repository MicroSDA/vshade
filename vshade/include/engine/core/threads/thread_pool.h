#ifndef ENGINE_CORE_THREADS_THREAD_POOL_H
#define ENGINE_CORE_THREADS_THREAD_POOL_H

#include <ankerl/unordered_dense.h>
#include <atomic>
#include <condition_variable>
#include <engine/core/utility/singleton.h>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace vshade
{
namespace thread
{
enum class ThreadName
{
    _MAIN_CPU_,
    _MAIN_RENDER_,
    _UNNAMED_1_,
    _UNNAMED_2_,
    _UNNAMED_3_,
    _UNNAMED_4_,
    _UNNAMED_5_,
    _UNNAMED_6_,
    _UNNAMED_7_,
    _UNNAMED_8_,
    _UNNAMED_9_,
    _UNNAMED_10_,
    _UNNAMED_11_,
    _UNNAMED_12_,
    _UNNAMED_13_,
    _UNNAMED_14_,
    _UNNAMED_15_,
    _UNNAMED_16_,
    _UNNAMED_17_,
    _UNNAMED_18_,
    _UNNAMED_19_,
    _UNNAMED_20_,
    _UNNAMED_21_,
    _UNNAMED_22_,
    _UNNAMED_23_,
    _UNNAMED_24_,
    _UNNAMED_25_,
    _UNNAMED_26_,
    _UNNAMED_27_,
    _UNNAMED_28_,
    _UNNAMED_29_,
    _UNNAMED_30_,
};

/// @brief ThreadPool class for managing tasks
class VSHADE_API ThreadPool final : public utility::CRTPSingleton<ThreadPool>
{
    friend class utility::CRTPSingleton<ThreadPool>;

public:
    ~ThreadPool();
    ThreadPool(ThreadPool const&)              = delete;
    ThreadPool(ThreadPool&&)                   = delete;
    ThreadPool& operator=(ThreadPool const&) & = delete;
    ThreadPool& operator=(ThreadPool&&) &      = delete;

    template <class T> inline auto emplace(T&& task, ThreadName const thread_id) -> auto
    {
        auto wrapper{std::make_shared<std::packaged_task<decltype(task())()>>(task)};
        {
            std::unique_lock<std::mutex> lock{mutex_};
            tasks_.emplace([=] { (*wrapper)(); });
        }

        event_.notify_one();
        return wrapper;
    }

private:
    explicit ThreadPool(std::size_t threads_count = std::thread::hardware_concurrency());
    void initialize();
    void shutDown() noexcept;

private:
    std::queue<std::function<void()>>                     tasks_;
    std::vector<std::thread>                              threads_;
    std::condition_variable                               event_;
    std::mutex                                            mutex_;
    std::atomic<std::size_t>                              threads_count_{0U};
    ankerl::unordered_dense::map<std::size_t, ThreadName> threads_id_;
    std::atomic<bool>                                     quit_{false};
};

} // namespace thread
} // namespace vshade

#endif // ENGINE_CORE_THREADS_THREAD_POOL_H
