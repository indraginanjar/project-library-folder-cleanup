#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <filesystem>

class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger& instance();
    
    void log(Level level, const std::string& message);
    void set_log_file(const std::filesystem::path& path);
    void close_log_file();
    void set_console_output(bool enabled);
    void set_min_level(Level level);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream log_file_;
    std::mutex mutex_;
    Level min_level_;
    bool console_output_;
    
    std::string level_to_string(Level level) const;
    std::string get_timestamp() const;
};
