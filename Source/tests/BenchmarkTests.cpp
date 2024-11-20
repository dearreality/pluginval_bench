#include <chrono>

#include <benchmark/benchmark.h>

#include "../PluginTests.h"
#include "../TestUtilities.h"

struct BenchmarkTest : public PluginTest
{
    BenchmarkTest() : PluginTest ("Benchmark", 1, {Requirements::Thread::backgroundThread, Requirements::GUI::requiresGUI}) {}

    void runTest (PluginTests& ut, juce::AudioPluginInstance& instance) override
    {
        ut.logMessage ("\nPlugin name: " + instance.getName());
        ut.logMessage ("Alternative names: " + instance.getAlternateDisplayNames().joinIntoString ("|"));
        ut.logMessage ("SupportsDoublePrecision: " + juce::String (instance.supportsDoublePrecisionProcessing() ? "yes" : "no"));
        ut.logMessage ("Reported latency: " + juce::String (instance.getLatencySamples()));
        ut.logMessage ("Reported taillength: " + juce::String (instance.getTailLengthSeconds()));

        const ScopedEditorShower editor (instance);

        int numBlocks = 1000;
        const int  blockSize = 512;
        const double sampleRate = 44100.0;
        const int numChannelsRequired = juce::jmax (instance.getTotalNumInputChannels(), instance.getTotalNumOutputChannels());
        juce::AudioBuffer<float> ab (numChannelsRequired, blockSize);
        juce::MidiBuffer mb;
        auto r = ut.getRandom();

        const auto& parameters = instance.getParameters();

        callPrepareToPlayOnMessageThreadIfVST3 (instance, sampleRate, blockSize);

        struct timespec start, end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        uint64_t totalCPUTime = 0;
        
        while (--numBlocks >= 0)
        {
            fillNoise (ab);
            
            struct timespec blockStart, blockEnd;
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &blockStart);
            
            instance.processBlock (ab, mb);
            
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &blockEnd);
            
            // Calculate block CPU time in nanoseconds
            uint64_t blockCPUTime = (blockEnd.tv_sec - blockStart.tv_sec) * 1000000000ULL 
                                   + (blockEnd.tv_nsec - blockStart.tv_nsec);
            totalCPUTime += blockCPUTime;
            
            mb.clear();
            ut.resetTimeout();
            juce::Thread::sleep (10);
        }

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

        // Calculate total process CPU time
        uint64_t processCPUTime = (end.tv_sec - start.tv_sec) * 1000000000ULL 
                                 + (end.tv_nsec - start.tv_nsec);

        // Calculate CPU usage percentage
        double cpuUsagePercentage = (totalCPUTime * 100.0) / processCPUTime;

        // Log results
        ut.logMessage ("Total Process CPU Time: " + juce::String(processCPUTime) + " nanoseconds");
        ut.logMessage ("Total Block CPU Time: " + juce::String(totalCPUTime) + " nanoseconds");
        ut.logMessage ("Estimated CPU Usage: " + juce::String(cpuUsagePercentage) + "%");
    }
};

static BenchmarkTest benchmarkTest;
