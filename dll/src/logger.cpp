#include "logger.h"
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!logFile_.is_open()) {
        logFile_.open("dll_log.txt", std::ios::out | std::ios::app);
    }
    
    std::string timestamp = getTimestamp();
    std::string levelStr = getLevelString(level);
    std::string logMsg = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (logFile_.is_open()) {
        logFile_ << logMsg << std::endl;
        logFile_.flush();
    }
    
    std::cout << logMsg << std::endl;
}

void Logger::logInfo(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::logWarning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::logError(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::logDebug(const std::string& message) {
    log(LogLevel::DBG, message);
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::DBG:      return "DBG";
        default:                 return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    std::tm tm_buf;
    
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif

    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}