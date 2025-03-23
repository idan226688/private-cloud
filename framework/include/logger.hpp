#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>
#include <thread_pool.hpp>

#include "singleton.hpp"

class Logger
{
public:
        enum Level
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL
        };
        void Log(Level level, const std::string& message);
        void SetLoggerLevel(Level level);
        ~Logger() noexcept;

private:
        ThreadPool* m_thread_pool;
        Level m_min_level;
        explicit Logger(const std::string& log_file, Level min_level = TRACE);
        friend class Singleton<Logger, std::string>;
        friend class Singleton<Logger, std::string, Level>;
        std::string m_log_file;

}; // Class Logger

#endif //__LOGGER_HPP__

