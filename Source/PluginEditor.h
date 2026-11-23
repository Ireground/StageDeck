#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

namespace Pal
{
    const juce::Colour bgTop { 0xff22303f }, bgBottom { 0xff111a23 }, panel { 0xff0e161f }, panelEdge { 0xff2a3a4b },
        text { 0xffe9eef3 }, dim { 0xff8394a6 }, faint { 0xff3b4d60 }, blue { 0xff5ab4e8 }, gold { 0xfff2c14e },
        knobTop { 0xff3c4c5e }, knobBottom { 0xff17212b }, badge { 0xff0d151d };
}

juce::Font uiFont (float h, bool bold = false);

//==============================================================================
class ProRLook : public juce::LookAndFeel_V4
{
public:
    ProRLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    float uiScale = 1.0f;
};

//==============================================================================
class Knob : public juce::Component, private juce::Slider::Listener
{
public:
    Knob (StageDeckProcessor&, const juce::String& paramId, const juce::String& title,
          const juce::String& lo, const juce::String& hi, bool big = false);
    ~Knob() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseUp (const juce::MouseEvent&) override { repaint(); }

    juce::Slider slider;
private:
    void sliderValueChanged (juce::Slider*) override { repaint(); }
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String title, lo, hi;
    bool big;
};

//==============================================================================
class StageView : public juce::Component
{
public:
    explicit StageView (StageDeckProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    std::function<void()> onIconMenu;
    float uiScale = 1.0f;

private:
    struct Item { int id; bool own; juce::String name; juce::Colour trackCol; int icon; float x, d, h; };
    juce::Array<Item> collectItems() const;

    float scaleAt (float d) const { return 1.0f / (1.0f + k * d); }
    juce::Point<float> project (float x, float d, float h) const;
    float visualHeight (float heightParam) const;   // 0..1 of the wall height, in real metres
    float heightFromVisual (float v) const;
    bool floorFromScreen (juce::Point<float> p, float& x, float& d) const;
    float badgeRadius (float d) const { return 30.0f * uiScale * (0.55f + 0.45f * scaleAt (d) / scaleAt (0.0f)); }
    int hitTestItem (juce::Point<float> p, Item& out) const;

    StageDeckProcessor& proc;
    float cx = 0, frontY = 0, halfW = 0, vpY = 0, wallH = 0, k = 0.85f;
    juce::Rectangle<float> area;

    // drag state
    enum class Drag { none, own, other } drag = Drag::none;
    int dragId = 0;
    juce::Point<float> grabOffset, lastMouse;
    float dragX = 0, dragD = 0, dragH = 0;
    bool heightMode = false;
};

//==============================================================================
class InfoPanel : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void setScene (const stage::Scene& s, double fs) { scene = s; sampleRate = fs; repaint(); }
    float uiScale = 1.0f;
private:
    stage::Scene scene;
    double sampleRate = 48000.0;
};

//==============================================================================
class StageDeckEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit StageDeckEditor (StageDeckProcessor&);
    ~StageDeckEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void showIconMenu();
    void applyRoomPreset (int index);

    StageDeckProcessor& proc;
    ProRLook look;

    Knob brightness, roomSize, reverb, decay, distLevel, width, mix, output;
    StageView stage;
    InfoPanel info;
    juce::Slider heightSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> heightAttachment;
    juce::TextButton iconButton;
    juce::ComboBox roomPresets;
    juce::ToggleButton showOthers { "Show other StageDeck tracks" };

    juce::Rectangle<int> headerArea, knobArea, middleArea, bottomArea, heightArea;
    float sc = 1.0f;
    int lastIcon = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StageDeckEditor)
};
