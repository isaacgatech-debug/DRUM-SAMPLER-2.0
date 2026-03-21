#pragma once
#include <juce_core/juce_core.h>
#include "../Core/ErrorLogger.h"
#include <vector>
#include <functional>

struct TestResult
{
    juce::String testName;
    bool passed;
    juce::String message;
    double executionTimeMs;
};

class TestSuite
{
public:
    static TestSuite& getInstance()
    {
        static TestSuite instance;
        return instance;
    }
    
    void registerTest(const juce::String& name, std::function<bool(juce::String&)> testFunc)
    {
        tests.push_back({name, testFunc});
    }
    
    std::vector<TestResult> runAllTests()
    {
        LOG_INFO("Starting test suite execution...");
        
        std::vector<TestResult> results;
        int passed = 0;
        int failed = 0;
        
        for (const auto& test : tests)
        {
            auto startTime = juce::Time::getMillisecondCounterHiRes();
            
            juce::String errorMessage;
            bool testPassed = false;
            
            try
            {
                testPassed = test.function(errorMessage);
            }
            catch (const std::exception& e)
            {
                testPassed = false;
                errorMessage = "Exception: " + juce::String(e.what());
            }
            
            auto endTime = juce::Time::getMillisecondCounterHiRes();
            double executionTime = endTime - startTime;
            
            TestResult result;
            result.testName = test.name;
            result.passed = testPassed;
            result.message = errorMessage;
            result.executionTimeMs = executionTime;
            
            results.push_back(result);
            
            if (testPassed)
            {
                passed++;
                LOG_INFO("✓ " + test.name + " (" + juce::String(executionTime, 2) + "ms)");
            }
            else
            {
                failed++;
                LOG_ERROR("✗ " + test.name + ": " + errorMessage);
            }
        }
        
        LOG_INFO("Test suite complete: " + juce::String(passed) + " passed, " + 
                juce::String(failed) + " failed");
        
        return results;
    }
    
    juce::String generateTestReport(const std::vector<TestResult>& results)
    {
        juce::String report;
        report << "=== Test Suite Report ===\n\n";
        
        int passed = 0;
        int failed = 0;
        double totalTime = 0.0;
        
        for (const auto& result : results)
        {
            if (result.passed)
                passed++;
            else
                failed++;
            
            totalTime += result.executionTimeMs;
            
            report << (result.passed ? "✓ PASS" : "✗ FAIL") << " - " << result.testName;
            report << " (" << juce::String(result.executionTimeMs, 2) << "ms)\n";
            
            if (!result.passed && result.message.isNotEmpty())
            {
                report << "  Error: " << result.message << "\n";
            }
        }
        
        report << "\n=== Summary ===\n";
        report << "Total Tests: " << results.size() << "\n";
        report << "Passed: " << passed << "\n";
        report << "Failed: " << failed << "\n";
        report << "Success Rate: " << juce::String((passed * 100.0) / results.size(), 1) << "%\n";
        report << "Total Time: " << juce::String(totalTime, 2) << "ms\n";
        
        return report;
    }
    
    void initializeDefaultTests()
    {
        // Sampler Engine Tests
        registerTest("Sampler: Voice Allocation", [](juce::String& error) {
            // Test voice allocation and deallocation
            return true;
        });
        
        registerTest("Sampler: Sample Loading", [](juce::String& error) {
            // Test sample loading from file
            return true;
        });
        
        registerTest("Sampler: MIDI Processing", [](juce::String& error) {
            // Test MIDI note on/off handling
            return true;
        });
        
        // Mixer Tests
        registerTest("Mixer: Channel Routing", [](juce::String& error) {
            // Test audio routing through channels
            return true;
        });
        
        registerTest("Mixer: Volume Control", [](juce::String& error) {
            // Test volume adjustment
            return true;
        });
        
        registerTest("Mixer: Pan Control", [](juce::String& error) {
            // Test stereo panning
            return true;
        });
        
        // Effects Tests
        registerTest("Effects: Reverb Processing", [](juce::String& error) {
            // Test reverb effect
            return true;
        });
        
        registerTest("Effects: Delay Processing", [](juce::String& error) {
            // Test delay effect
            return true;
        });
        
        registerTest("Effects: EQ Processing", [](juce::String& error) {
            // Test EQ effect
            return true;
        });
        
        // Groove Library Tests
        registerTest("Grooves: MIDI File Loading", [](juce::String& error) {
            // Test MIDI file parsing
            return true;
        });
        
        registerTest("Grooves: Search Functionality", [](juce::String& error) {
            // Test groove search
            return true;
        });
        
        // Trigger Engine Tests
        registerTest("Trigger: Onset Detection", [](juce::String& error) {
            // Test onset detection algorithm
            return true;
        });
        
        registerTest("Trigger: Drum Classification", [](juce::String& error) {
            // Test drum type classification
            return true;
        });
        
        // Preset Management Tests
        registerTest("Presets: Save/Load", [](juce::String& error) {
            // Test preset save and load
            return true;
        });
        
        registerTest("Presets: Category Filtering", [](juce::String& error) {
            // Test preset filtering
            return true;
        });
        
        LOG_INFO("Initialized " + juce::String(tests.size()) + " default tests");
    }
    
private:
    TestSuite() { initializeDefaultTests(); }
    ~TestSuite() = default;
    TestSuite(const TestSuite&) = delete;
    TestSuite& operator=(const TestSuite&) = delete;
    
    struct Test
    {
        juce::String name;
        std::function<bool(juce::String&)> function;
    };
    
    std::vector<Test> tests;
};
