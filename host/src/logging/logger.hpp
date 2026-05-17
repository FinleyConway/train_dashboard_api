#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace host {
    class logger_t {
    public:
        static void init() {
            spdlog::set_pattern("%^[%T] %n: %v%$");

            s_logger = spdlog::stderr_color_mt("Server");
            s_logger->set_level(spdlog::level::trace);
        }

        static std::shared_ptr<spdlog::logger>& get_logger() {
            return s_logger;
        }

    private:
        inline static std::shared_ptr<spdlog::logger> s_logger;
    };
}

#define LOG_TRACE(...)    ::host::logger_t::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::host::logger_t::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::host::logger_t::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::host::logger_t::get_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::host::logger_t::get_logger()->critical(__VA_ARGS__)