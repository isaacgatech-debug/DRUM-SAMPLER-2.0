#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <mutex>

enum class LogLevel
{
    Info,
    Warning,
    Error,
    Critical
};

struct LogEntry
{
    juce::String message;
    LogLevel level;
    juce::String timestamp;
    juce::String source;
    
    juce::String getLevelString() const
    {
        switch (level)
        {
            case LogLevel::Info:     return "INFO";
            case LogLevel::Warning:  return "WARN";
            case LogLevel::Error:    return "ERROR";
            case LogLevel::Critical: return "CRITICAL";
            default:                 return "UNKNOWN";
        }
    }
    
    juce::Colour getLevelColour() const
    {
        switch (level)
        {
            case LogLevel::Info:     return juce::Colours::lightblue;
            case LogLevel::Warning:  return juce::Colours::orange;
            case LogLevel::Error:    return juce::Colours::red;
            case LogLevel::Critical: return juce::Colours::darkred;
            default:                 return juce::Colours::white;
        }
    }
};

class ErrorLogger
{
public:
    static ErrorLogger& getInstance()
    {
        static ErrorLogger instance;
        return instance;
    }
    
    void log(const juce::String& message, LogLevel level = LogLevel::Info, 
             const juce::String& source = "")
    {
        std::lock_guard<std::mutex> lock(logMutex);
        
        LogEntry entry;
        entry.message = message;
        entry.level = level;
        entry.source = source;
        entry.timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
        
        logEntries.push_back(entry);
        
        if (logEntries.size() > static_cast<size_t>(maxLogEntries))
            logEntries.erase(logEntries.begin());
        
        if (level == LogLevel::Error || level == LogLevel::Critical)
            errorCount++;
        
        DBG("[" << entry.getLevelString() << "] " << entry.source << ": " << entry.message);
        
        notifyListeners();
    }
    
    void logInfo(const juce::String& message, const juce::String& source = "")
    {
        log(message, LogLevel::Info, source);
    }
    
    void logWarning(const juce::String& message, const juce::String& source = "")
    {
        log(message, LogLevel::Warning, source);
    }
    
    void logError(const juce::String& message, const juce::String& source = "")
    {
        log(message, LogLevel::Error, source);
    }
    
    void logCritical(const juce::String& message, const juce::String& source = "")
    {
        log(message, LogLevel::Critical, source);
    }
    
    std::vector<LogEntry> getLogEntries() const
    {
        std::lock_guard<std::mutex> lock(logMutex);
        return logEntries;
    }
    
    void clearLogs()
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logEntries.clear();
        errorCount = 0;
        notifyListeners();
    }
    
    int getErrorCount() const { return errorCount; }
    
    void exportToFile(const juce::File& file)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        
        juce::String output;
        output << "Drum Tech - Error Log\n";
        output << "Generated: " << juce::Time::getCurrentTime().toString(true, true, true, true) << "\n";
        output << "Total Entries: " << logEntries.size() << "\n";
        output << "Errors: " << errorCount << "\n";
        output << "\n" << juce::String::repeatedString("=", 80) << "\n\n";
        
        for (const auto& entry : logEntries)
        {
            output << "[" << entry.timestamp << "] ";
            output << "[" << entry.getLevelString() << "] ";
            if (entry.source.isNotEmpty())
                output << entry.source << ": ";
            output << entry.message << "\n";
        }
        
        file.replaceWithText(output);
    }
    
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void logUpdated() = 0;
    };
    
    void addListener(Listener* listener)
    {
        listeners.add(listener);
    }
    
    void removeListener(Listener* listener)
    {
        listeners.remove(listener);
    }
    
private:
    ErrorLogger() = default;
    ~ErrorLogger() = default;
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;
    
    void notifyListeners()
    {
        listeners.call([](Listener& l) { l.logUpdated(); });
    }
    
    std::vector<LogEntry> logEntries;
    mutable std::mutex logMutex;
    int maxLogEntries = 1000;
    int errorCount = 0;
    
    juce::ListenerList<Listener> listeners;
};

#define LOG_INFO(message) ErrorLogger::getInstance().logInfo(message, __FILE__)
#define LOG_WARNING(message) ErrorLogger::getInstance().logWarning(message, __FILE__)
#define LOG_ERROR(message) ErrorLogger::getInstance().logError(message, __FILE__)
#define LOG_CRITICAL(message) ErrorLogger::getInstance().logCritical(message, __FILE__)
