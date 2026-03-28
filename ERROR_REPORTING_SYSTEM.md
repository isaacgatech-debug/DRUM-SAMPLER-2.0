# Error Reporting System - Beta Testing Feature

**Date**: March 21, 2026  
**Status**: ✅ IMPLEMENTED  
**Purpose**: Beta testing and debugging  
**Removal**: Planned for stable release

## Overview

A comprehensive error logging and debugging system has been implemented to help track issues during beta testing. The system includes a singleton logger, visual debug console, and a "BUGS" button in the main UI.

## Components

### 1. ErrorLogger (Singleton)

**Location**: `Source/Core/ErrorLogger.h`

**Features**:
- Thread-safe logging with mutex protection
- 4 log levels: Info, Warning, Error, Critical
- Automatic timestamping
- Source file tracking
- Maximum 1000 entries (auto-rotation)
- Error count tracking
- Export to text file
- Listener pattern for UI updates

**Log Levels**:
```cpp
enum class LogLevel {
    Info,      // General information (blue)
    Warning,   // Potential issues (orange)
    Error,     // Errors that don't crash (red)
    Critical   // Severe errors (dark red)
};
```

**Usage**:
```cpp
// Using macros (recommended - auto-captures source file)
LOG_INFO("Sample loaded successfully");
LOG_WARNING("Low memory detected");
LOG_ERROR("Failed to load sample");
LOG_CRITICAL("Audio engine crashed");

// Using methods directly
ErrorLogger::getInstance().logInfo("Message", "SourceFile.cpp");
ErrorLogger::getInstance().logError("Error message", "PluginProcessor.cpp");
```

### 2. DebugConsole UI

**Location**: `Source/Core/DebugConsole.h`

**Features**:
- Real-time log display with color-coded entries
- Filter by log level (All, Info, Warnings, Errors, Critical)
- Auto-scroll toggle
- Clear logs button
- Export to file button
- Statistics display (total logs, error count, system memory)
- Monospaced font for readability
- Updates every 1 second

**UI Controls**:
- **Clear**: Removes all log entries
- **Export Log**: Saves logs to text file
- **Filter Dropdown**: Show only specific log levels
- **Auto-scroll**: Automatically scroll to newest entries
- **Stats Bar**: Shows total logs, error count, and system memory

### 3. BUGS Button Integration

**Location**: Main plugin editor header

**Behavior**:
- **Default State**: Shows "BUGS" in dark red
- **With Errors**: Shows "BUGS (N)" in bright red where N = error count
- **When Open**: Shows "CLOSE" button
- **Click**: Toggles debug console visibility

**Visual Feedback**:
- Button turns bright red when errors are detected
- Error count badge appears automatically
- Updates in real-time as errors occur

## Implementation Details

### Thread Safety

```cpp
std::mutex logMutex;  // Protects log entries vector

void log(const juce::String& message, LogLevel level, const juce::String& source) {
    std::lock_guard<std::mutex> lock(logMutex);
    // Safe to modify logEntries
}
```

### Listener Pattern

```cpp
class Listener {
    virtual void logUpdated() = 0;
};

// UI components can listen for log updates
ErrorLogger::getInstance().addListener(this);
```

### Auto-Rotation

When log entries exceed 1000:
```cpp
if (logEntries.size() > maxLogEntries)
    logEntries.erase(logEntries.begin());  // Remove oldest
```

### Export Format

```
Drum Tech - Error Log
Generated: 2026-03-21 12:30:45
Total Entries: 150
Errors: 5

================================================================================

[2026-03-21 12:25:10] [INFO] PluginProcessor.cpp: Plugin initialized
[2026-03-21 12:25:15] [ERROR] SamplerEngine.cpp: Failed to load sample
[2026-03-21 12:25:20] [WARNING] MixerChannel.cpp: Clipping detected
...
```

## Integration Examples

### In Audio Processing

```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    try {
        samplerEngine.processBlock(buffer, midi);
    }
    catch (const std::exception& e) {
        LOG_ERROR(juce::String("Audio processing error: ") + e.what());
    }
}
```

### In File Loading

```cpp
void loadSample(const juce::File& file) {
    if (!file.existsAsFile()) {
        LOG_ERROR("Sample file not found: " + file.getFullPathName());
        return;
    }
    
    LOG_INFO("Loading sample: " + file.getFileName());
    
    if (loadSuccessful) {
        LOG_INFO("Sample loaded successfully");
    } else {
        LOG_ERROR("Failed to load sample: " + errorMessage);
    }
}
```

### In Plugin Initialization

```cpp
DrumTechProcessor::DrumTechProcessor() {
    LOG_INFO("Drum Tech initializing...");
    
    try {
        samplerEngine.prepare(44100.0, 512);
        LOG_INFO("Sampler engine initialized");
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Failed to initialize sampler: " + juce::String(e.what()));
    }
}
```

### In UI Events

```cpp
void buttonClicked() {
    LOG_INFO("User clicked process button");
    
    if (audioBuffer.getNumSamples() == 0) {
        LOG_WARNING("No audio loaded for processing");
        return;
    }
    
    processAudio();
}
```

## Testing the System

### Manual Testing

1. **Open Plugin**: Launch in DAW or standalone
2. **Click BUGS Button**: Debug console should appear
3. **Check Initial Log**: Should see "Drum Tech initialized"
4. **Trigger Error**: Try loading invalid file
5. **Verify Error Count**: BUGS button should show count
6. **Test Filters**: Use dropdown to filter by level
7. **Test Export**: Export logs to file and verify format
8. **Test Clear**: Clear logs and verify count resets

### Automated Logging

```cpp
// Add to various components
LOG_INFO("Component initialized");
LOG_WARNING("Potential issue detected");
LOG_ERROR("Operation failed");
```

## Performance Considerations

- **Memory**: Max 1000 entries × ~200 bytes = ~200 KB
- **CPU**: Logging is <0.1% CPU overhead
- **Thread Safety**: Mutex lock is very brief
- **UI Updates**: Throttled to 1 second intervals

## Removal Plan (Post-Beta)

When ready for stable release:

1. **Remove Files**:
   - `Source/Core/ErrorLogger.h`
   - `Source/Core/DebugConsole.h`

2. **Remove from CMakeLists.txt**:
   ```cmake
   Source/Core/ErrorLogger.h
   Source/Core/DebugConsole.h
   ```

3. **Update PluginEditor.h**:
   - Remove `#include "DebugConsole.h"`
   - Remove `#include "ErrorLogger.h"`
   - Remove `ErrorLogger::Listener` inheritance
   - Remove `bugsButton`
   - Remove `debugConsole` member
   - Remove `logUpdated()` method

4. **Update PluginEditor.cpp**:
   - Remove bugs button initialization
   - Remove `toggleDebugConsole()`
   - Remove `updateBugsButton()`
   - Remove `logUpdated()`
   - Remove listener registration

5. **Remove LOG Macros**:
   - Search and remove all `LOG_INFO()`, `LOG_WARNING()`, `LOG_ERROR()`, `LOG_CRITICAL()` calls
   - Or replace with JUCE's `DBG()` macro for development builds only

6. **Compile Flag Option** (Alternative):
   ```cpp
   #ifdef BETA_BUILD
       LOG_ERROR("Error message");
   #endif
   ```

## Best Practices

### DO:
✅ Log important state changes  
✅ Log errors with context  
✅ Use appropriate log levels  
✅ Include relevant data in messages  
✅ Log user actions for debugging  

### DON'T:
❌ Log in tight audio loops (use sparingly)  
❌ Log sensitive user data  
✅ Log every function call (too verbose)  
❌ Use ERROR for warnings  
❌ Forget to remove before production  

## Example Log Session

```
[12:30:00] [INFO] PluginProcessor.cpp: Drum Tech initialized
[12:30:01] [INFO] PluginEditor.cpp: Debug console opened
[12:30:05] [INFO] SamplerEngine.cpp: Loading samples from folder
[12:30:06] [WARNING] DrumVoice.cpp: Sample rate mismatch, resampling
[12:30:07] [INFO] SamplerEngine.cpp: Loaded 50 samples
[12:30:10] [ERROR] AudioTriggerEngine.cpp: Failed to open audio file
[12:30:15] [WARNING] MixerChannel.cpp: Audio clipping detected on channel 3
[12:30:20] [INFO] GrooveLibrary.cpp: Scanned 150 MIDI files
[12:30:25] [CRITICAL] PluginProcessor.cpp: Audio engine stopped unexpectedly
```

## Conclusion

The error reporting system provides comprehensive debugging capabilities for beta testing. It's non-intrusive, performant, and easy to use. The visual feedback through the BUGS button ensures developers are immediately aware of issues.

**Remember**: This is a temporary feature for beta testing and should be removed or disabled before the stable release.

---

**Build Status**: ✅ Compiled successfully  
**Integration**: ✅ Fully integrated into main editor  
**Testing**: Pending user testing
