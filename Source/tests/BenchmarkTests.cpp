#include "../PluginTests.h"
#include "../TestUtilities.h"

struct BenchmarkTest : public PluginTest
{
    BenchmarkTest() : PluginTest ("Benchmark", 1,
                                        {Requirements::Thread::backgroundThread, Requirements::GUI::requiresGUI}) {}

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

        while (--numBlocks >= 0)
        {
            /*for (auto parameter : parameters)
            {
                ut.logVerboseMessage (juce::String ("\nTesting parameter: ") + juce::String (parameter->getParameterIndex()) + " - " + parameter->getName (512));
                for (auto parameter : parameters)
                    parameter->setValue (r.nextFloat());
            }
            */

            fillNoise (ab);
            instance.processBlock (ab, mb);
            mb.clear();

            ut.resetTimeout();
            juce::Thread::sleep (10);
        }
    }
};

static BenchmarkTest benchmarkTest;
