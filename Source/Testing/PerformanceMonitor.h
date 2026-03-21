#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Core/ErrorLogger.h"
#include <chrono>
#include <map>

class PerformanceMonitor
{
public:
    static PerformanceMonitor& getInstance()
    {
        static PerformanceMonitor instance;
        return instance;
    }
    
    struct PerformanceMetrics
    {
        double averageCPU = 0.0;
        double peakCPU = 0.0;
        int voiceCount = 0;
        double bufferFillPercentage = 0.0;
        int xruns = 0;
        double latencyMs = 0.0;
        size_t memoryUsageMB = 0;
    };
    
    void startFrame()
    {
        frameStartTime = std::chrono::high_resolution_clock::now();
    }
    
    void endFrame()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - frameStartTime);
        
        double cpuUsage = (duration.count() / 1000.0) / (blockSize / sampleRate * 1000.0) * 100.0;
        
        currentMetrics.averageCPU = currentMetrics.averageCPU * 0.95 + cpuUsage * 0.05;
        currentMetrics.peakCPU = juce::jmax(currentMetrics.peakCPU, cpuUsage);
        
        if (cpuUsage > 80.0)
        {
            LOG_WARNING("High CPU usage: " + juce::String(cpuUsage, 1) + "%");
        }
        
        frameCount++;
    }
    
    void reportXRun()
    {
        currentMetrics.xruns++;
        LOG_ERROR("Audio buffer underrun detected (xrun #" + juce::String(currentMetrics.xruns) + ")");
    }
    
    void setVoiceCount(int count)
    {
        currentMetrics.voiceCount = count;
    }
    
    void setBufferFill(double percentage)
    {
        currentMetrics.bufferFillPercentage = percentage;
    }
    
    void setLatency(double latencyMs)
    {
        currentMetrics.latencyMs = latencyMs;
    }
    
    void updateMemoryUsage()
    {
        currentMetrics.memoryUsageMB = juce::SystemStats::getMemorySizeInMegabytes();
    }
    
    void prepare(double sr, int bs)
    {
        sampleRate = sr;
        blockSize = bs;
        reset();
    }
    
    void reset()
    {
        currentMetrics = PerformanceMetrics();
        frameCount = 0;
    }
    
    PerformanceMetrics getMetrics() const
    {
        return currentMetrics;
    }
    
    juce::String getPerformanceReport() const
    {
        juce::String report;
        report << "=== Performance Report ===\n";
        report << "Average CPU: " << juce::String(currentMetrics.averageCPU, 2) << "%\n";
        report << "Peak CPU: " << juce::String(currentMetrics.peakCPU, 2) << "%\n";
        report << "Active Voices: " << currentMetrics.voiceCount << "\n";
        report << "Buffer Fill: " << juce::String(currentMetrics.bufferFillPercentage, 1) << "%\n";
        report << "XRuns: " << currentMetrics.xruns << "\n";
        report << "Latency: " << juce::String(currentMetrics.latencyMs, 2) << " ms\n";
        report << "Memory: " << currentMetrics.memoryUsageMB << " MB\n";
        report << "Frames Processed: " << frameCount << "\n";
        return report;
    }
    
    void logPerformanceWarnings()
    {
        if (currentMetrics.averageCPU > 70.0)
            LOG_WARNING("High average CPU usage: " + juce::String(currentMetrics.averageCPU, 1) + "%");
        
        if (currentMetrics.peakCPU > 90.0)
            LOG_WARNING("Peak CPU usage critical: " + juce::String(currentMetrics.peakCPU, 1) + "%");
        
        if (currentMetrics.xruns > 0)
            LOG_ERROR("Audio dropouts detected: " + juce::String(currentMetrics.xruns) + " xruns");
        
        if (currentMetrics.latencyMs > 20.0)
            LOG_WARNING("High latency: " + juce::String(currentMetrics.latencyMs, 1) + " ms");
    }
    
private:
    PerformanceMonitor() = default;
    ~PerformanceMonitor() = default;
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
    
    PerformanceMetrics currentMetrics;
    std::chrono::high_resolution_clock::time_point frameStartTime;
    
    double sampleRate = 44100.0;
    int blockSize = 512;
    int frameCount = 0;
};
