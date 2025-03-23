//include <ctime> 
#define __INIT_SYMBOL
#include <fstream> //file stream
#include <chrono> // time
#include <iomanip> // format time

#include "logger.hpp" //logger
//g++ -std=c++17 -fPIC -Iinclude -c ./src/Logger.cpp -D__INIT_SYMBOL -o Logger.o

Logger::Logger(const std::string& log_file, Level min_level) 
    : m_log_file(log_file), m_min_level(min_level)
{
    m_thread_pool = Singleton<ThreadPool>::GetInstance();
}

void Logger::Log(Level level, const std::string &message)
{
    class log_task : public ThreadPool::ITask
    {
    public:
        log_task(Level level, const std::string &message, const std::string &log_file) 
            : m_level(level), m_message(message), m_log_file(log_file) {}

        void Run() override
        {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm* now_tm = std::localtime(&now_time);

            std::ostringstream oss;
            oss << std::put_time(now_tm, "%d%m%Y: %H:%M:%S");
            std::string time_str = oss.str();

            std::ofstream file;
            file.open(m_log_file, std::ios_base::app);
            file << time_str << " " << m_level << " : " << m_message << "\n";
            file.close();
        }

    private:
        Level m_level;
        const std::string m_message;
        const std::string m_log_file;
    };

    if (level >= m_min_level)
    {
        m_thread_pool->Add(std::make_shared<log_task>(level, message, m_log_file), ThreadPool::HIGH);
    }
}

void Logger::SetLoggerLevel(Level level)
{
    m_min_level = level;
}

Logger::~Logger() noexcept
{
}
