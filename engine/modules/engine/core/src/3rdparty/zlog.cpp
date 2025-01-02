#include "zlog.h"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
namespace zlog {
    zloger::zloger()
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S][%s:%#] %-8l %^%v%$");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/zengine.log", true);
        file_sink->set_level(spdlog::level::trace);
        //file_sink->set_pattern("[%Y-%m-%d %H:%M:%S][%s:%#] %-8l %^%v%$");

        const spdlog::sinks_init_list sink_list = { console_sink, file_sink };

        spdlog::init_thread_pool(8192, 1);

        m_logger = std::make_shared<spdlog::async_logger>("zlogger",
            sink_list.begin(),
            sink_list.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
        m_logger->set_level(spdlog::level::trace);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S][%s:%#] %-8l %^%v%$");
        spdlog::register_logger(m_logger);
    }
    zloger::~zloger() noexcept
    {
        m_logger->flush();
        //spdlog::drop_all();
        //spdlog::shutdown();
    }
}
