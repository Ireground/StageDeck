#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "StageEngine.h"
#include "StageRegistry.h"

#ifndef STAGEDECK_SEND
 #define STAGEDECK_SEND 0
#endif

namespace ParamIDs
{
    static const juce::String posX = "posX", depth = "depth", height = "height",
        roomSize = "roomSize", decay = "decay", brightness = "brightness", reverb = "reverb",
        distLevel = "distLevel", width = "width", mix = "mix", output = "output",
        tailMode = "tailMode", linkRoom = "linkRoom";
}

class StageDeckProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    StageDeckProcessor();
    ~StageDeckProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return STAGEDECK_SEND ? "StageDeck Send" : "StageDeck"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateTrackProperties (const TrackProperties& properties) override;

    // helpers for the GUI
    stage::SceneParams currentSceneParams() const;
    void setParam (const juce::String& id, float realValue);
    void setParamAsGesture (const juce::String& id, float realValue);
    void beginGesture (const juce::String& id) { if (auto* p = apvts.getParameter (id)) p->beginChangeGesture(); }
    void endGesture (const juce::String& id)   { if (auto* p = apvts.getParameter (id)) p->endChangeGesture(); }
    float getParam (const juce::String& id) const { return apvts.getRawParameterValue (id)->load(); }

    juce::String getDisplayName() const;
    juce::Colour getTrackColour() const { const juce::ScopedLock sl (nameLock); return trackColour; }
    int getRegistryId() const { return registryId; }
    bool isSendMode() const { return STAGEDECK_SEND && pTail != nullptr && pTail->load() > 0.5f; }

    juce::AudioProcessorValueTreeState apvts;

    // GUI state (saved with the project)
    std::atomic<int> iconIndex { 0 };
    int editorWidth = 1100, editorHeight = 720;
    bool showOthers = true;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    void timerCallback() override;

    stage::StageEngine engine;
    int registryId = 0;
    bool foreignGestureActive = false;

    juce::CriticalSection nameLock;
    juce::String trackName;
    juce::Colour trackColour { juce::Colours::transparentBlack };

    std::atomic<float>* pX = nullptr; std::atomic<float>* pDepth = nullptr; std::atomic<float>* pHeight = nullptr;
    std::atomic<float>* pRoom = nullptr; std::atomic<float>* pDecay = nullptr; std::atomic<float>* pBright = nullptr;
    std::atomic<float>* pReverb = nullptr; std::atomic<float>* pDist = nullptr; std::atomic<float>* pWidth = nullptr;
    std::atomic<float>* pMix = nullptr; std::atomic<float>* pOut = nullptr;
    std::atomic<float>* pTail = nullptr; std::atomic<float>* pLink = nullptr;

    void syncLinkedRoom();
    bool wasLinked = false;
    int seenRoomVersion = 0;
    float lastRoom[3] { -1.0f, -1.0f, -1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StageDeckProcessor)
};
