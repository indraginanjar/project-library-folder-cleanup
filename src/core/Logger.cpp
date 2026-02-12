#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : min_level_(Level::Info)
    , console_output_(true) {
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (level < min_level_) {
        return;
    }
    
    std::string log_entry = get_timestamp() + " [" + level_to_string(level) + "] " + message;
    
    if (console_output_) {
        std::cout << log_entry << std::endl;
    }
    
    if (log_file_.is_open()) {
        log_file_ << log_entry << std::endl;
        log_file_.flush();
    }
}

void Logger::set_log_file(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    log_file_.open(path, std::ios::app);
}

void Logger::close_log_file() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::set_console_output(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    console_output_ = enabled;
}

void Logger::set_min_level(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

std::string Logger::level_to_string(Level level) const {
    switch (level) {
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warning: return "WARNING";
        case Level::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::get_timestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
