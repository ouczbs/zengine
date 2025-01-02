#pragma once
#include "singleton.h"
#include "source_location.h"
#include <cstdint>
#include <stdexcept>
namespace zlog {
    using level_enum = spdlog::level::level_enum;
    class zloger {
    private:
        std::shared_ptr<spdlog::logger> m_logger;
    public:
        zloger();
        ~zloger() noexcept;
        template <typename... Args>
        void log(level_enum level, format& fmt, Args &&...args) {
            m_logger->log(fmt.loc, level, fmt::runtime(fmt.value), std::forward<Args>(args)...);
        }
        void flush() {
            m_logger->flush();
        }
    };
    UNIQUER_INLINE(zloger, zlog, "zlog::zlog")
    inline void flush() {
        UNIQUER_VAL(zlog).flush();
    }
    template <typename... Args>
    void info(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::info, fmt, std::forward<Args>(args)...);
    };
    template <typename... Args>
    void debug(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::debug, fmt, std::forward<Args>(args)...);
    };
    template <typename... Args>
    void warn(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::warn, fmt, std::forward<Args>(args)...);
    };
    template <typename... Args>
    void error(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::err, fmt, std::forward<Args>(args)...);
    };
    template <typename... Args>
    void errorf(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::err, fmt, std::forward<Args>(args)...);
        UNIQUER_VAL(zlog).flush();
    };
    template <typename... Args>
    void fatal(format fmt, Args &&...args) {
        UNIQUER_VAL(zlog).log(level_enum::critical, fmt, std::forward<Args>(args)...);
        UNIQUER_VAL(zlog).flush();
        throw std::runtime_error(std::string(fmt.value));
    };
    inline void fatal(format fmt) {
        UNIQUER_VAL(zlog).log(level_enum::critical, fmt);
        throw std::runtime_error(std::string(fmt.value));
    };
};