#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Icons.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout StageDeckProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto pct = [] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; };

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::posX, 1 }, "Left / Right",
        NormalisableRange<float> (-1.0f, 1.0f), 0.0f, AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { const int n = roundToInt (std::abs (v) * 100.0f); return n == 0 ? String ("C") : String (n) + (v < 0 ? " L" : " R"); })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::depth, 1 }, "Distance",
        NormalisableRange<float> (0.0f, 1.0f), 0.3f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::height, 1 }, "Height",
        NormalisableRange<float> (0.0f, 1.0f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { const int n = roundToInt ((v - 0.5f) * 200.0f); return n == 0 ? String ("Ear level") : (n > 0 ? "+" : "") + String (n) + " %"; })));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::roomSize, 1 }, "Room Size",
        NormalisableRange<float> (0.0f, 1.0f), 0.4f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::decay, 1 }, "Decay",
        NormalisableRange<float> (0.2f, 10.0f, 0.0f, 0.4f), 1.6f, AudioParameterFloatAttributes().withLabel ("s")
            .withStringFromValueFunction ([] (float v, int) { return String (v, 2) + " s"; })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::brightness, 1 }, "Brightness",
        NormalisableRange<float> (0.0f, 1.0f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::reverb, 1 }, "Reverb",
        NormalisableRange<float> (-24.0f, 12.0f), 0.0f, AudioParameterFloatAttributes().withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int) { return (v > 0 ? "+" : "") + String (v, 1) + " dB"; })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::distLevel, 1 }, "Distance Level",
        NormalisableRange<float> (0.0f, 1.0f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::width, 1 }, "Source Width",
        NormalisableRange<float> (0.0f, 1.0f), 1.0f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::mix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 1.0f), 1.0f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::output, 1 }, "Output",
        NormalisableRange<float> (-24.0f, 12.0f), 0.0f, AudioParameterFloatAttributes().withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int) { return (v > 0 ? "+" : "") + String (v, 1) + " dB"; })));
    return { p.begin(), p.end() };
}

StageDeckProcessor::StageDeckProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "StageDeck", createLayout())
{
    pX = apvts.getRawParameterValue (ParamIDs::posX);
    pDepth = apvts.getRawParameterValue (ParamIDs::depth);
    pHeight = apvts.getRawParameterValue (ParamIDs::height);
    pRoom = apvts.getRawParameterValue (ParamIDs::roomSize);
    pDecay = apvts.getRawParameterValue (ParamIDs::decay);
    pBright = apvts.getRawParameterValue (ParamIDs::brightness);
    pReverb = apvts.getRawParameterValue (ParamIDs::reverb);
    pDist = apvts.getRawParameterValue (ParamIDs::distLevel);
    pWidth = apvts.getRawParameterValue (ParamIDs::width);
    pMix = apvts.getRawParameterValue (ParamIDs::mix);
    pOut = apvts.getRawParameterValue (ParamIDs::output);

    registryId = StageRegistry::get().add();
    timerCallback();
    startTimerHz (30);
}

StageDeckProcessor::~StageDeckProcessor()
{
    stopTimer();
    StageRegistry::get().remove (registryId);
}

bool StageDeckProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet(), out = layouts.getMainOutputChannelSet();
    if (out != AudioChannelSet::stereo()) return false;
    return in == AudioChannelSet::mono() || in == AudioChannelSet::stereo();
}

void StageDeckProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
}

stage::SceneParams StageDeckProcessor::currentSceneParams() const
{
    stage::SceneParams s;
    s.x = pX->load(); s.depth = pDepth->load(); s.height = pHeight->load();
    s.roomSize = pRoom->load(); s.decay = pDecay->load(); s.brightness = pBright->load();
    s.reverbDb = pReverb->load(); s.distAmount = pDist->load(); s.width = pWidth->load();
    s.fs = getSampleRate() > 0 ? getSampleRate() : 48000.0;
    return s;
}

void StageDeckProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    const int numIn = getTotalNumInputChannels(), numOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    if (numOut < 2 || n == 0) return;
    if (numIn == 1) buffer.copyFrom (1, 0, buffer, 0, 0, n);
    for (int ch = jmax (2, numIn); ch < numOut; ++ch) buffer.clear (ch, 0, n);

    stage::EngineParams ep;
    ep.scene = currentSceneParams();
    ep.mix = pMix->load();
    ep.outputDb = pOut->load();
    engine.setTargets (ep);
    engine.process (buffer.getWritePointer (0), buffer.getWritePointer (1), n);
}

void StageDeckProcessor::setParam (const String& id, float realValue)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (realValue));
}

String StageDeckProcessor::getDisplayName() const
{
    {
        const ScopedLock sl (nameLock);
        if (trackName.isNotEmpty()) return trackName;
    }
    return Icons::getName (iconIndex.load());
}

void StageDeckProcessor::updateTrackProperties (const TrackProperties& properties)
{
    const ScopedLock sl (nameLock);
    trackName = properties.name;
    trackColour = properties.colour;
}

void StageDeckProcessor::timerCallback()
{
    auto& reg = StageRegistry::get();
    float x, d, h; bool gesture = false;
    const bool hasMove = reg.takePendingMove (registryId, x, d, h, gesture);

    // another StageDeck window is dragging this sound: wrap the moves in one undo-able gesture
    if (gesture != foreignGestureActive)
    {
        for (auto* id : { &ParamIDs::posX, &ParamIDs::depth, &ParamIDs::height })
            gesture ? beginGesture (*id) : endGesture (*id);
        foreignGestureActive = gesture;
    }

    if (hasMove)
    {
        setParam (ParamIDs::posX, x);
        setParam (ParamIDs::depth, d);
        setParam (ParamIDs::height, h);
    }

    reg.publish (registryId, getDisplayName(), getTrackColour(), iconIndex.load(), pX->load(), pDepth->load(), pHeight->load());
}

void StageDeckProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("icon", iconIndex.load(), nullptr);
    state.setProperty ("editorW", editorWidth, nullptr);
    state.setProperty ("editorH", editorHeight, nullptr);
    state.setProperty ("showOthers", showOthers, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void StageDeckProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = ValueTree::fromXml (*xml);
            iconIndex = jlimit (0, Icons::count - 1, (int) state.getProperty ("icon", 0));
            editorWidth = (int) state.getProperty ("editorW", editorWidth);
            editorHeight = (int) state.getProperty ("editorH", editorHeight);
            showOthers = (bool) state.getProperty ("showOthers", true);
            apvts.replaceState (state);
        }
}

AudioProcessorEditor* StageDeckProcessor::createEditor() { return new StageDeckEditor (*this); }

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new StageDeckProcessor(); }
