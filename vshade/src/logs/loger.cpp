#include "engine/core/logs/loger.h"

vshade::Logger::Logger()
{
	// Параметры ротации
	auto max_size  = 1024 * 1024 * 5; // 5 MB
	auto max_files = 3;

	// ==== Синк для консоли ====
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::trace);
	console_sink->set_pattern("%^[%T] [%n] %v%$");
	
	// ==== Синк с ротацией для core ====
	auto core_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/core.log", max_size, max_files);
	core_file_sink->set_level(spdlog::level::trace);

	// ==== Синк с ротацией для application ====
	auto app_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/app.log", max_size, max_files);
	app_file_sink->set_level(spdlog::level::trace);

	// ==== Core Logger ====
	core_logger_ = std::make_shared<spdlog::logger>("CORE", spdlog::sinks_init_list{console_sink, core_file_sink});
	core_logger_->set_level(spdlog::level::trace);
	spdlog::register_logger(core_logger_);

	// ==== Client/Application Logger ====
	client_logger_ = std::make_shared<spdlog::logger>("APPLICATION", spdlog::sinks_init_list{console_sink, app_file_sink});
	client_logger_->set_level(spdlog::level::trace);
	spdlog::register_logger(client_logger_);
}