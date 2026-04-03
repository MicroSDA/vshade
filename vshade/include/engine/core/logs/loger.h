#ifndef ENGINE_CORE_LOGS_LOGER_H
#define ENGINE_CORE_LOGS_LOGER_H

// #define SPDLOG_USE_STD_FORMAT
#define SPDLOG_COMPILED_LIB

#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <engine/core/utility/singleton.h>

#include <sstream>

namespace vshade
{
class VSHADE_API Logger final : public utility::CRTPSingleton<Logger>
{
    friend class utility::CRTPSingleton<Logger>;

public:
    virtual ~Logger()
    {
        spdlog::shutdown();
    };

    Logger(Logger const&)              = delete;
    Logger(Logger&&)                   = delete;
    Logger& operator=(Logger const&) & = delete;
    Logger& operator=(Logger&&) &      = delete;

    std::shared_ptr<spdlog::logger>& getCoreLogger()
    {
        return core_logger_;
    }
    std::shared_ptr<spdlog::logger>& getClientLogger()
    {
        return client_logger_;
    }
    std::ostringstream& getCoreStream()
    {
        return core_stream_;
    }
    std::ostringstream& getClientStream()
    {
        return client_stream_;
    }

protected:
    explicit Logger();

private:
    std::shared_ptr<spdlog::logger> core_logger_;
    std::shared_ptr<spdlog::logger> client_logger_;
    std::ostringstream              core_stream_;
    std::ostringstream              client_stream_;
};

#define VSHADE_CORE_ERROR(...)                                                                     \
    ::vshade::Logger::instance().getCoreLogger()->error(                                        \
        "{} [{}:{}] ", fmt::format(__VA_ARGS__), __FILE__, __LINE__);                              \
    ::vshade::Logger::destroy();                                                           \
    std::abort();

#define VSHADE_CORE_WARNING(...)                                                                   \
    ::vshade::Logger::instance().getCoreLogger()->warn("{} [{}:{}] ", fmt::format(__VA_ARGS__), \
                                                          __FILE__, __LINE__);
#define VSHADE_CORE_INFO(...) ::vshade::Logger::instance().getCoreLogger()->info(__VA_ARGS__);
#define VSHADE_CORE_TRACE(...)                                                                     \
    ::vshade::Logger::instance().getCoreLogger()->trace(                                        \
        "{} [{}:{}] ", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#ifdef _VSHADE_DEBUG_
    #define VSHADE_CORE_DEBUG(...) ::vshade::Logger::instance().getCoreLogger()->debug(__VA_ARGS__);
#else
    #define VSHADE_CORE_DEBUG(...) static_cast<void>(__VA_ARGS__);
#endif

#define VSHADE_ERROR(...)                                                                          \
    ::vshade::Logger::instance().getClientLogger()->error(                                      \
        "{} [{}:{}] ", fmt::format(__VA_ARGS__), __FILE__, __LINE__);                              \
    ::vshade::Logger::destroy();                                                           \
    std::abort();

#define VSHADE_WARNING(...)                                                                        \
    ::vshade::Logger::instance().getClientLogger()->warn(                                       \
        "{} [{}:{}] ", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
#define VSHADE_INFO(...) ::vshade::Logger::instance().getClientLogger()->info(__VA_ARGS__);
#define VSHADE_TRACE(...)                                                                          \
    ::vshade::Logger::instance().getClientLogger()->trace(                                      \
        "{} [{}:{}] ", fmt::format(__VA_ARGS__), __FILE__, __LINE__);
} // namespace vshade

#endif // ENGINE_CORE_LOGS_LOGER_H