#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>

namespace CS16Capture {

/**
 * @brief Log levels
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Thread-safe logger class
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger with a file path
     * @param filePath Path to the log file
     * @return true if initialization was successful
     */
    bool initialize(const std::string& filePath);

    /**
     * @brief Set the minimum log level
     * @param level Minimum log level to write
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Log a message
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Log a debug message
     */
    void debug(const std::string& message);

    /**
     * @brief Log an info message
     */
    void info(const std::string& message);

    /**
     * @brief Log a warning message
     */
    void warning(const std::string& message);

    /**
     * @brief Log an error message
     */
    void error(const std::string& message);

    /**
     * @brief Log a critical message
     */
    void critical(const std::string& message);

    /**
     * @brief Flush the log buffer
     */
    void flush();

    /**
     * @brief Close the logger
     */
    void close();

    /**
     * @brief Destructor
     */
    ~Logger();

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();

    std::string getCurrentTimestamp() const;
    std::string logLevelToString(LogLevel level) const;

    std::ofstream logFile_;
    std::mutex mutex_;
    LogLevel minLevel_;
    bool initialized_;
};

/**
 * @brief Helper macros for logging
 */
#define LOG_DEBUG(msg) CS16Capture::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) CS16Capture::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) CS16Capture::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) CS16Capture::Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) CS16Capture::Logger::getInstance().critical(msg)

} // namespace CS16Capture
