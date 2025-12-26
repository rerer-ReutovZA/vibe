#include "logger.h"
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : minLevel_(LogLevel::DEBUG)
    , initialized_(false) {
}

Logger::~Logger() {
    close();
}

bool Logger::initialize(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }

    logFile_.open(filePath, std::ios::out | std::ios::app);
    if (!logFile_.is_open()) {
        return false;
    }

    initialized_ = true;
    log(LogLevel::INFO, "Logger initialized successfully");
    
    return true;
}

void Logger::setLogLevel(LogLevel level) {
    minLevel_ = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!initialized_ || level < minLevel_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    std::string timestamp = getTimestamp();
    std::string levelStr = getLevelString(level);
    std::string logMsg = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (logFile_.is_open()) {
        logFile_ << logMsg << std::endl;
        logFile_.flush();
    }
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
    log(LogLevel::DEBUG, message);
}

void Logger::flush() {
    if (initialized_) {
        std::lock_guard<std::mutex> lock(mutex_);
        logFile_.flush();
    }
}

void Logger::close() {
    if (initialized_) {
        std::lock_guard<std::mutex> lock(mutex_);
        log(LogLevel::INFO, "Logger shutting down");
        logFile_.close();
        initialized_ = false;
    }
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::DEBUG:    return "DEBUG";
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
