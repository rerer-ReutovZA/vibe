#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger {
public:
    static Logger& getInstance();
    
    bool initialize(const std::string& filePath);
    void setLogLevel(LogLevel level);
    
    void log(LogLevel level, const std::string& message);
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    void logDebug(const std::string& message);
    
    void flush();
    void close();
    
    ~Logger();
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    
    std::string getLevelString(LogLevel level);
    std::string getTimestamp();
    
    std::ofstream logFile_;
    std::mutex mutex_;
    LogLevel minLevel_;
    bool initialized_;
};

#endif // LOGGER_H
