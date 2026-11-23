#include "PluginEditor.h"
#include "Icons.h"

using namespace juce;

Font uiFont (float h, bool bold)
{
    auto o = FontOptions (h);
    return Font (bold ? o.withStyle ("Bold") : o);
}

//==============================================================================
ProRLook::ProRLook()
{
    setColour (PopupMenu::backgroundColourId, Pal::panel);
    setColour (PopupMenu::textColourId, Pal::text);
    setColour (PopupMenu::highlightedBackgroundColourId, Pal::blue.withAlpha (0.35f));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (ComboBox::textColourId, Pal::text);
    setColour (ComboBox::backgroundColourId, Pal::panel);
    setColour (ComboBox::outlineColourId, Pal::panelEdge);
    setColour (ComboBox::arrowColourId, Pal::dim);
    setColour (TextButton::textColourOffId, Pal::text);
    setColour (ToggleButton::textColourId, Pal::dim);
}

void ProRLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float start, float end, Slider& s)
{
    const bool big = (bool) s.getProperties()["big"];
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float r = jmin (b.getWidth(), b.getHeight()) * 0.5f * 0.78f;
    const auto c = b.getCentre();
    const float angle = start + pos * (end - start);

    // tick marks
    g.setColour (Pal::faint);
    const int ticks = big ? 21 : 11;
    for (int i = 0; i < ticks; ++i)
    {
        const float a = start + (float) i / (float) (ticks - 1) * (end - start);
        const auto p = c.getPointOnCircumference (r * 1.2f, a);
        g.fillEllipse (p.x - 1.2f * uiScale, p.y - 1.2f * uiScale, 2.4f * uiScale, 2.4f * uiScale);
    }

    // value arc
    {
        Path arc;
        arc.addCentredArc (c.x, c.y, r * 1.08f, r * 1.08f, 0.0f, start, angle, true);
        g.setColour (Pal::blue.withAlpha (s.isMouseOverOrDragging() ? 1.0f : 0.75f));
        g.strokePath (arc, PathStrokeType (2.0f * uiScale, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // body
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillEllipse (c.x - r, c.y - r + 2.0f * uiScale, r * 2, r * 2);
    ColourGradient grad (Pal::knobTop, c.x, c.y - r, Pal::knobBottom, c.x, c.y + r, false);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
    g.setColour (Colours::white.withAlpha (0.12f));
    g.drawEllipse (c.x - r + 0.5f, c.y - r + 0.5f, r * 2 - 1, r * 2 - 1, 1.0f);
    if (big)
    {
        const float ri = r * 0.82f;
        g.setColour (Colour (0xff131c26));
        g.fillEllipse (c.x - ri, c.y - ri, ri * 2, ri * 2);
        g.setColour (Pal::text);
        g.setFont (uiFont (r * 0.34f, true));
        g.drawText (s.getTextFromValue (s.getValue()), Rectangle<float> (c.x - ri, c.y - r * 0.2f, ri * 2, r * 0.4f), Justification::centred);
    }

    // pointer
    const auto p1 = c.getPointOnCircumference (big ? r * 0.86f : r * 0.3f, angle);
    const auto p2 = c.getPointOnCircumference (r * 0.95f, angle);
    g.setColour (Colours::white);
    g.drawLine ({ p1, p2 }, (big ? 3.0f : 2.4f) * uiScale);
}

void ProRLook::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float pos, float, float, Slider::SliderStyle, Slider& s)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float cxp = b.getCentreX();
    g.setColour (Pal::faint);
    g.fillRoundedRectangle (cxp - 2.0f * uiScale, b.getY(), 4.0f * uiScale, b.getHeight(), 2.0f * uiScale);

    // ear-level marker
    const float mid = b.getCentreY();
    g.setColour (Pal::dim);
    g.drawLine (cxp - 9 * uiScale, mid, cxp + 9 * uiScale, mid, 1.0f);

    g.setColour (Pal::blue.withAlpha (s.isMouseOverOrDragging() ? 1.0f : 0.8f));
    g.fillRoundedRectangle (cxp - 2.0f * uiScale, jmin (pos, mid), 4.0f * uiScale, std::abs (mid - pos), 2.0f * uiScale);

    const float tw = 22.0f * uiScale, th = 9.0f * uiScale;
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (cxp - tw * 0.5f, pos - th * 0.5f + 1.5f, tw, th, th * 0.5f);
    g.setColour (Pal::gold);
    g.fillRoundedRectangle (cxp - tw * 0.5f, pos - th * 0.5f, tw, th, th * 0.5f);
}

void ProRLook::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? Pal::blue.withAlpha (0.25f) : (over ? Colour (0xff1a2633) : Pal::panel));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (over ? Pal::blue.withAlpha (0.7f) : Pal::panelEdge);
    g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.0f);
}

void ProRLook::drawButtonText (Graphics& g, TextButton& b, bool, bool)
{
    g.setColour (Pal::text);
    g.setFont (uiFont (13.0f * uiScale, true));
    g.drawText (b.getButtonText(), b.getLocalBounds().reduced ((int) (12 * uiScale), 0), Justification::centred);
}

void ProRLook::drawToggleButton (Graphics& g, ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const float h = 14.0f * uiScale, w = 26.0f * uiScale;
    auto sw = Rectangle<float> (r.getX(), r.getCentreY() - h * 0.5f, w, h);
    const bool on = b.getToggleState();
    g.setColour (on ? Pal::blue.withAlpha (0.85f) : Pal::faint);
    g.fillRoundedRectangle (sw, h * 0.5f);
    const float d = h - 4.0f;
    g.setColour (Colours::white);
    g.fillEllipse (on ? sw.getRight() - d - 2.0f : sw.getX() + 2.0f, sw.getY() + 2.0f, d, d);
    g.setColour (over ? Pal::text : Pal::dim);
    g.setFont (uiFont (12.0f * uiScale, true));
    g.drawText (b.getButtonText().toUpperCase(), r.withTrimmedLeft (w + 8.0f * uiScale), Justification::centredLeft);
}

void ProRLook::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box)
{
    auto r = Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (box.isMouseOver (true) ? Colour (0xff1a2633) : Pal::panel);
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (box.isMouseOver (true) ? Pal::blue.withAlpha (0.7f) : Pal::panelEdge);
    g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.0f);
    Path arrow;
    const float ax = (float) w - 16.0f * uiScale, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4 * uiScale, ay - 2 * uiScale, ax + 4 * uiScale, ay - 2 * uiScale, ax, ay + 3 * uiScale);
    g.setColour (Pal::dim);
    g.fillPath (arrow);
}

Font ProRLook::getComboBoxFont (ComboBox&) { return uiFont (13.0f * uiScale, true); }
Font ProRLook::getPopupMenuFont() { return uiFont (14.0f * uiScale); }
void ProRLook::positionComboBoxText (ComboBox& box, Label& l)
{
    l.setBounds ((int) (12 * uiScale), 0, box.getWidth() - (int) (30 * uiScale), box.getHeight());
    l.setFont (getComboBoxFont (box));
}

//==============================================================================
Knob::Knob (StageDeckProcessor& p, const String& paramId, const String& t, const String& l, const String& h, bool isBig)
    : title (t), lo (l), hi (h), big (isBig)
{
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
    slider.getProperties().set ("big", big);
    slider.setMouseDragSensitivity (big ? 260 : 200);
    addAndMakeVisible (slider);
    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, paramId, slider);
    if (auto* param = p.apvts.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
    slider.addListener (this);
    slider.addMouseListener (this, false);
}

Knob::~Knob() { slider.removeListener (this); }

void Knob::resized()
{
    auto b = getLocalBounds();
    const int labelH = jmax (14, getHeight() / 6);
    b.removeFromBottom (labelH);
    slider.setBounds (b);
}

void Knob::paint (Graphics& g)
{
    const int labelH = jmax (14, getHeight() / 6);
    auto lab = getLocalBounds().removeFromBottom (labelH).toFloat();
    const bool active = slider.isMouseOverOrDragging();
    const float fh = labelH * 0.72f;

    if (active && ! big)
    {
        g.setColour (Pal::blue);
        g.setFont (uiFont (fh, true));
        g.drawText (slider.getTextFromValue (slider.getValue()), lab, Justification::centred);
    }
    else
    {
        g.setColour (Pal::text);
        g.setFont (uiFont (fh, true));
        g.drawText (title, lab, Justification::centred);
    }

    // range labels at the ends of the knob's travel
    auto sb = slider.getBounds().toFloat();
    const float r = jmin (sb.getWidth(), sb.getHeight()) * 0.5f * 0.78f;
    const auto c = sb.getCentre();
    g.setColour (Pal::dim);
    g.setFont (uiFont (fh * 0.72f, true));
    const float yy = c.y + r * 1.0f;
    g.drawText (lo, Rectangle<float> (c.x - r * 2.2f, yy, r * 1.6f, fh), Justification::centredRight);
    g.drawText (hi, Rectangle<float> (c.x + r * 0.6f, yy, r * 1.6f, fh), Justification::centredLeft);
}

//==============================================================================
StageView::StageView (StageDeckProcessor& p) : proc (p) {}

void StageView::resized()
{
    area = getLocalBounds().toFloat();
    cx = area.getCentreX();
    frontY = area.getBottom() - 62.0f * uiScale;
    const float top = area.getY() + 16.0f * uiScale;
    const float h = frontY - top;
    const float s1 = scaleAt (1.0f);
    const float backFloorY = frontY - 0.42f * h;
    vpY = frontY - 0.42f * h / (1.0f - s1);
    wallH = (backFloorY - top) / s1;
    halfW = area.getWidth() * 0.43f;
}

Point<float> StageView::project (float x, float d, float h) const
{
    const float s = scaleAt (d);
    return { cx + x * halfW * s, vpY + (frontY - vpY) * s - h * wallH * s };
}

float StageView::visualHeight (float heightParam) const
{
    const float H = stage::roomHeightFor (proc.getParam (ParamIDs::roomSize));
    return stage::heightToZ (heightParam, H) / H;
}

float StageView::heightFromVisual (float v) const
{
    const float H = stage::roomHeightFor (proc.getParam (ParamIDs::roomSize));
    return stage::zToHeight (jlimit (0.0f, 1.0f, v) * H, H);
}

bool StageView::floorFromScreen (Point<float> p, float& x, float& d) const
{
    float s = (p.y - vpY) / (frontY - vpY);
    s = jlimit (scaleAt (1.0f), 1.0f, s);
    d = jlimit (0.0f, 1.0f, (1.0f / s - 1.0f) / k);
    x = jlimit (-1.0f, 1.0f, (p.x - cx) / (halfW * s));
    return true;
}

Array<StageView::Item> StageView::collectItems() const
{
    Array<Item> items;
    const int ownId = proc.getRegistryId();
    if (proc.showOthers)
        for (auto& e : StageRegistry::get().snapshot())
            if (e.id != ownId)
                items.add ({ e.id, false, e.name, e.trackColour, e.icon, e.x, e.depth, e.height });

    items.add ({ ownId, true, proc.getDisplayName(), proc.getTrackColour(), proc.iconIndex.load(),
                 proc.getParam (ParamIDs::posX), proc.getParam (ParamIDs::depth), proc.getParam (ParamIDs::height) });

    // far first so near sounds are drawn on top
    std::stable_sort (items.begin(), items.end(), [] (const Item& a, const Item& b) { return a.d > b.d; });
    return items;
}

int StageView::hitTestItem (Point<float> p, Item& out) const
{
    auto items = collectItems();
    // own first, then from front to back
    for (auto& it : items)
        if (it.own && project (it.x, it.d, visualHeight (it.h)).getDistanceFrom (p) <= badgeRadius (it.d) * 1.1f) { out = it; return 1; }
    for (int i = items.size(); --i >= 0;)
    {
        auto& it = items.getReference (i);
        if (! it.own && project (it.x, it.d, visualHeight (it.h)).getDistanceFrom (p) <= badgeRadius (it.d)) { out = it; return 1; }
    }
    return 0;
}

void StageView::paint (Graphics& g)
{
    // panel
    g.setColour (Pal::panel);
    g.fillRoundedRectangle (area, 8.0f * uiScale);

    const auto fl = project (-1, 0, 0), fr = project (1, 0, 0), bl = project (-1, 1, 0), br = project (1, 1, 0);
    const auto blT = project (-1, 1, 1), brT = project (1, 1, 1), flT = project (-1, 0, 1), frT = project (1, 0, 1);

    Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (area.toNearestInt());

    // walls
    auto quad = [] (Point<float> a, Point<float> b, Point<float> c, Point<float> d)
    { Path p; p.startNewSubPath (a); p.lineTo (b); p.lineTo (c); p.lineTo (d); p.closeSubPath(); return p; };

    g.setGradientFill (ColourGradient (Colour (0xff1c2a39), cx, blT.y, Colour (0xff15202b), cx, bl.y, false));
    g.fillPath (quad (blT, brT, br, bl));
    g.setGradientFill (ColourGradient (Colour (0xff172330), fl.x, 0, Colour (0xff1d2b3a), bl.x, 0, false));
    g.fillPath (quad (flT, blT, bl, fl));
    g.setGradientFill (ColourGradient (Colour (0xff172330), fr.x, 0, Colour (0xff1d2b3a), br.x, 0, false));
    g.fillPath (quad (brT, frT, fr, br));

    // floor
    g.setGradientFill (ColourGradient (Colour (0xff1f3042), cx, bl.y, Colour (0xff2a4057), cx, fl.y, false));
    g.fillPath (quad (bl, br, fr, fl));

    // grid
    const auto sp = proc.currentSceneParams();
    for (int i = 0; i <= 8; ++i)
    {
        const float x = -1.0f + i * 0.25f;
        g.setColour (Pal::blue.withAlpha (i == 4 ? 0.18f : 0.08f));
        g.drawLine (Line<float> (project (x, 0, 0), project (x, 1, 0)), 1.0f);
    }
    g.setFont (uiFont (10.5f * uiScale, true));
    for (int i = 0; i <= 8; ++i)
    {
        const float d = i / 8.0f;
        g.setColour (Pal::blue.withAlpha (0.08f));
        g.drawLine (Line<float> (project (-1, d, 0), project (1, d, 0)), 1.0f);
        if (i % 2 == 0)
        {
            auto p = sp; p.x = 0; p.depth = d; p.height = 0.5f;
            stage::Scene s; stage::computeScene (p, s);
            const auto pt = project (-1, d, 0);
            g.setColour (Pal::dim.withAlpha (0.8f));
            const float m = s.sy - s.ly;
            g.drawText ((m < 10.0f ? String (m, 1) : String (roundToInt (m))) + " m",
                        Rectangle<float> (pt.x + 6 * uiScale, pt.y - 14 * uiScale, 60 * uiScale, 12 * uiScale), Justification::centredLeft);
        }
    }

    // edges
    g.setColour (Pal::blue.withAlpha (0.55f));
    for (auto l : { Line<float> (fl, bl), Line<float> (fr, br), Line<float> (bl, br), Line<float> (bl, blT), Line<float> (br, brT),
                    Line<float> (blT, brT), Line<float> (blT, flT), Line<float> (brT, frT) })
        g.drawLine (l, 1.4f * uiScale);
    g.setColour (Pal::blue.withAlpha (0.3f));
    g.drawLine (Line<float> (fl, fr), 1.0f);

    // listener
    const auto lp = Point<float> (cx, frontY + 34.0f * uiScale);
    {
        const float hr = 11.0f * uiScale;
        g.setColour (Pal::text.withAlpha (0.85f));
        Path nose; nose.addTriangle (lp.x - 4 * uiScale, lp.y - hr + 2, lp.x + 4 * uiScale, lp.y - hr + 2, lp.x, lp.y - hr - 6 * uiScale);
        g.fillPath (nose);
        g.fillEllipse (lp.x - hr - 4 * uiScale, lp.y - 4 * uiScale, 6 * uiScale, 8 * uiScale);
        g.fillEllipse (lp.x + hr - 2 * uiScale, lp.y - 4 * uiScale, 6 * uiScale, 8 * uiScale);
        g.fillEllipse (lp.x - hr, lp.y - hr, hr * 2, hr * 2);
        g.setColour (Pal::dim);
        g.setFont (uiFont (10.0f * uiScale, true));
        g.drawText ("YOU", Rectangle<float> (lp.x + 18 * uiScale, lp.y - 6 * uiScale, 40 * uiScale, 12 * uiScale), Justification::centredLeft);
    }

    // room size readout
    {
        stage::Scene s; stage::computeScene (sp, s);
        g.setColour (Pal::dim);
        g.setFont (uiFont (11.0f * uiScale, true));
        g.drawText ("ROOM  " + String (s.roomW, 1) + " x " + String (s.roomD, 1) + " x " + String (s.roomH, 1) + " m",
                    area.reduced (12 * uiScale, 8 * uiScale).removeFromTop (14 * uiScale), Justification::topLeft);
    }

    // hints
    g.setColour (Pal::dim.withAlpha (0.7f));
    g.setFont (uiFont (10.5f * uiScale));
    {
        auto hb = area.reduced (12 * uiScale, 8 * uiScale);
        g.drawText ("Drag = move", hb.removeFromBottom (13 * uiScale), Justification::bottomRight);
        g.drawText ("Shift+drag / wheel = height", hb.removeFromBottom (13 * uiScale), Justification::bottomRight);
        g.drawText ("Double-click = reset  |  Right-click = icon", hb.removeFromBottom (13 * uiScale), Justification::bottomRight);
    }

    // sounds
    for (auto& it : collectItems())
    {
        const auto fp = project (it.x, it.d, 0);
        const auto ip = project (it.x, it.d, visualHeight (it.h));
        const float r = badgeRadius (it.d);
        const float alpha = it.own ? 1.0f : 0.55f;
        const auto col = Icons::getColour (it.icon);

        if (it.own)
        {
            g.setColour (Pal::blue.withAlpha (0.22f));
            g.drawLine (Line<float> (lp, fp), 1.2f * uiScale);
        }

        // shadow + height stem
        g.setColour (Colours::black.withAlpha (0.35f * alpha));
        g.fillEllipse (fp.x - r * 0.9f, fp.y - r * 0.28f, r * 1.8f, r * 0.56f);
        if (std::abs (ip.y - fp.y) > 2.0f)
        {
            Path stem; stem.startNewSubPath (fp); stem.lineTo (ip);
            Path dashed; const float dl[] = { 4.0f * uiScale, 3.0f * uiScale };
            PathStrokeType (1.2f * uiScale).createDashedStroke (dashed, stem, dl, 2);
            g.setColour ((it.own ? Pal::gold : Pal::dim).withAlpha (0.6f * alpha));
            g.fillPath (dashed);
            g.fillEllipse (fp.x - 2.5f * uiScale, fp.y - 2.5f * uiScale, 5 * uiScale, 5 * uiScale);
        }

        // badge
        if (it.own)
        {
            for (int i = 3; i > 0; --i)
            {
                g.setColour (Pal::gold.withAlpha (0.06f * (float) i));
                const float gr = r + (float) (4 - i) * 4.0f * uiScale;
                g.fillEllipse (ip.x - gr, ip.y - gr, gr * 2, gr * 2);
            }
        }
        g.setColour (Pal::badge.withAlpha (it.own ? 1.0f : 0.85f));
        g.fillEllipse (ip.x - r, ip.y - r, r * 2, r * 2);
        g.setColour (it.own ? Pal::gold : Pal::dim.withAlpha (0.6f));
        g.drawEllipse (ip.x - r, ip.y - r, r * 2, r * 2, (it.own ? 2.2f : 1.2f) * uiScale);

        Icons::draw (g, it.icon, Rectangle<float> (ip.x - r * 0.62f, ip.y - r * 0.62f, r * 1.24f, r * 1.24f),
                     col.withAlpha (alpha), Pal::badge.withAlpha (it.own ? 1.0f : 0.85f));

        if (! it.trackCol.isTransparent())
        {
            const auto dp = ip.translated (r * 0.72f, -r * 0.72f);
            g.setColour (it.trackCol);
            g.fillEllipse (dp.x - 4 * uiScale, dp.y - 4 * uiScale, 8 * uiScale, 8 * uiScale);
        }

        g.setFont (uiFont ((it.own ? 12.5f : 11.0f) * uiScale, true));
        g.setColour (it.own ? Pal::text : Pal::dim.withAlpha (0.9f));
        g.drawText (it.name.toUpperCase(), Rectangle<float> (ip.x - 80 * uiScale, ip.y + r + 3 * uiScale, 160 * uiScale, 15 * uiScale),
                    Justification::centredTop, true);
    }
}

void StageView::mouseDown (const MouseEvent& e)
{
    const auto p = e.position;
    lastMouse = p;
    heightMode = e.mods.isShiftDown() || e.mods.isAltDown();
    Item it;
    const bool hit = hitTestItem (p, it) != 0;

    if (e.mods.isPopupMenu())
    {
        if (hit && it.own && onIconMenu) onIconMenu();
        return;
    }

    if (hit && ! it.own)
    {
        drag = Drag::other; dragId = it.id;
        dragX = it.x; dragD = it.d; dragH = it.h;
        grabOffset = project (it.x, it.d, 0) - p;
        StageRegistry::get().setGesture (dragId, true);
        return;
    }

    drag = Drag::own;
    for (auto* id : { &ParamIDs::posX, &ParamIDs::depth, &ParamIDs::height }) proc.beginGesture (*id);
    dragX = proc.getParam (ParamIDs::posX); dragD = proc.getParam (ParamIDs::depth); dragH = proc.getParam (ParamIDs::height);

    if (hit)
        grabOffset = project (dragX, dragD, 0) - p;
    else if (! heightMode)
    {
        // click on empty floor: jump there (keeping the height)
        grabOffset = {};
        floorFromScreen (p, dragX, dragD);
        proc.setParam (ParamIDs::posX, dragX);
        proc.setParam (ParamIDs::depth, dragD);
    }
    repaint();
}

void StageView::mouseDrag (const MouseEvent& e)
{
    if (drag == Drag::none) return;
    const auto p = e.position;
    const bool hMode = e.mods.isShiftDown() || e.mods.isAltDown();

    if (hMode)
    {
        const float s = scaleAt (dragD);
        dragH = heightFromVisual (visualHeight (dragH) - (p.y - lastMouse.y) / (wallH * s));
    }
    else
    {
        if (heightMode) grabOffset = project (dragX, dragD, 0) - p; // switching back from height mode: no jump
        floorFromScreen (p + grabOffset, dragX, dragD);
    }
    heightMode = hMode;
    lastMouse = p;

    if (drag == Drag::own)
    {
        proc.setParam (ParamIDs::posX, dragX);
        proc.setParam (ParamIDs::depth, dragD);
        proc.setParam (ParamIDs::height, dragH);
    }
    else
        StageRegistry::get().requestMove (dragId, dragX, dragD, dragH);
    repaint();
}

void StageView::mouseUp (const MouseEvent&)
{
    if (drag == Drag::own)
        for (auto* id : { &ParamIDs::posX, &ParamIDs::depth, &ParamIDs::height }) proc.endGesture (*id);
    else if (drag == Drag::other)
        StageRegistry::get().setGesture (dragId, false);
    drag = Drag::none;
}

void StageView::mouseDoubleClick (const MouseEvent& e)
{
    Item it;
    if (hitTestItem (e.position, it) && it.own)
    {
        for (auto* id : { &ParamIDs::posX, &ParamIDs::depth, &ParamIDs::height }) proc.beginGesture (*id);
        proc.setParam (ParamIDs::posX, 0.0f);
        proc.setParam (ParamIDs::depth, 0.3f);
        proc.setParam (ParamIDs::height, 0.5f);
        for (auto* id : { &ParamIDs::posX, &ParamIDs::depth, &ParamIDs::height }) proc.endGesture (*id);
        repaint();
    }
}

void StageView::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& w)
{
    const float h = jlimit (0.0f, 1.0f, proc.getParam (ParamIDs::height) + w.deltaY * (w.isReversed ? -0.08f : 0.08f));
    proc.beginGesture (ParamIDs::height);
    proc.setParam (ParamIDs::height, h);
    proc.endGesture (ParamIDs::height);
    repaint();
}

//==============================================================================
void InfoPanel::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Pal::panel);
    g.fillRoundedRectangle (b, 8.0f * uiScale);

    auto r = b.reduced (14.0f * uiScale, 12.0f * uiScale);
    g.setColour (Pal::text);
    g.setFont (uiFont (12.0f * uiScale, true));
    g.drawText ("AUTO-ADJUSTED", r.removeFromTop (16 * uiScale), Justification::centredLeft);
    g.setColour (Pal::gold);
    g.fillEllipse (r.getRight() - 7 * uiScale, r.getY() - 12 * uiScale, 7 * uiScale, 7 * uiScale);
    r.removeFromTop (6 * uiScale);

    const auto& s = scene;
    auto fmtHz = [] (float hz) { return hz >= 1000.0f ? String (hz / 1000.0f, 1) + " kHz" : String (roundToInt (hz)) + " Hz"; };
    auto fmtDb = [] (float db) { return (db > 0.05f ? "+" : "") + String (db, 1) + " dB"; };
    const int panPct = roundToInt (std::abs (s.pan) * 100.0f);
    const float directDb = stage::gainToDb (s.directGain);
    const float lateDb = stage::gainToDb (s.lateGain);

    const std::pair<String, String> rows[] = {
        { "PAN", panPct == 0 ? String ("Centre") : String (panPct) + (s.pan < 0 ? " % L" : " % R") },
        { "DISTANCE", String (s.distance, 1) + " m" },
        { "ELEVATION", (s.elevationDeg > 0.5f ? "+" : "") + String (roundToInt (s.elevationDeg)) + String (CharPointer_UTF8 ("\xc2\xb0")) },
        { "DIRECT LEVEL", fmtDb (directDb) },
        { "AIR (HIGH CUT)", s.airLpHz > 19000.0f ? String ("Off") : fmtHz (s.airLpHz) },
        { "PROXIMITY LOWS", fmtDb (s.lowShelfDb) },
        { "HEIGHT TONE", fmtDb (s.highShelfDb) + "  @ " + fmtHz (stage::Scene::shelfHighHz) },
        { "EAR NOTCH", std::abs (s.notchDb) < 0.1f ? String ("Off") : fmtHz (s.notchHz) + "  " + fmtDb (s.notchDb) },
        { "SOURCE WIDTH", String (roundToInt (s.effWidth * 100.0f)) + " %" },
        { "1ST REFLECTION", String (s.firstReflMs, 1) + " ms" },
        { "PRE-DELAY", String (s.latePredelayMs, 1) + " ms" },
        { "EARLY REFL.", fmtDb (s.erGainDb) },
        { sendMode ? "REVERB SEND" : "REVERB TAIL", fmtDb (sendMode ? lateDb + stage::gainToDb (stage::kSendScale) : lateDb) },
        { "DIRECT / REVERB", fmtDb (directDb - lateDb) },
    };

    const float rowH = jmin (19.0f * uiScale, (r.getHeight() * 0.62f) / (float) std::size (rows));
    for (auto& row : rows)
    {
        auto line = r.removeFromTop (rowH);
        g.setColour (Pal::dim);
        g.setFont (uiFont (rowH * 0.56f, true));
        g.drawText (row.first, line, Justification::centredLeft);
        g.setColour (Pal::blue);
        g.setFont (uiFont (rowH * 0.62f, true));
        g.drawText (row.second, line, Justification::centredRight);
        g.setColour (Pal::faint.withAlpha (0.35f));
        g.drawLine (line.getX(), line.getBottom(), line.getRight(), line.getBottom(), 0.6f);
    }

    // direct-sound EQ curve (L blue, R gold)
    r.removeFromTop (10 * uiScale);
    g.setColour (Pal::text);
    g.setFont (uiFont (11.0f * uiScale, true));
    g.drawText ("DIRECT SOUND EQ", r.removeFromTop (14 * uiScale), Justification::centredLeft);
    r.removeFromTop (4 * uiScale);
    auto eq = r;
    g.setColour (Colour (0xff0a1118));
    g.fillRoundedRectangle (eq, 5.0f * uiScale);

    const float dbTop = 8.0f, dbBot = -26.0f;
    auto yFor = [&] (float db) { return jmap (jlimit (dbBot, dbTop, db), dbTop, dbBot, eq.getY() + 4, eq.getBottom() - 4); };
    auto xFor = [&] (float hz) { return jmap (std::log10 (hz), std::log10 (20.0f), std::log10 (20000.0f), eq.getX(), eq.getRight()); };

    g.setFont (uiFont (9.0f * uiScale, true));
    for (float db : { 0.0f, -12.0f, -24.0f })
    {
        g.setColour (Pal::faint.withAlpha (db == 0.0f ? 0.7f : 0.35f));
        g.drawHorizontalLine ((int) yFor (db), eq.getX(), eq.getRight());
        g.setColour (Pal::dim.withAlpha (0.7f));
        g.drawText (String ((int) db), Rectangle<float> (eq.getX() + 3, yFor (db) - 11, 30, 10), Justification::centredLeft);
    }
    for (float hz : { 100.0f, 1000.0f, 10000.0f })
    {
        g.setColour (Pal::faint.withAlpha (0.35f));
        g.drawVerticalLine ((int) xFor (hz), eq.getY(), eq.getBottom());
        g.setColour (Pal::dim.withAlpha (0.7f));
        g.drawText (hz >= 1000 ? String ((int) (hz / 1000)) + "k" : String ((int) hz), Rectangle<float> (xFor (hz) + 2, eq.getBottom() - 12, 30, 10), Justification::centredLeft);
    }

    for (int ch = 1; ch >= 0; --ch)
    {
        Path curve;
        const int N = 90;
        for (int i = 0; i <= N; ++i)
        {
            const float hz = 20.0f * std::pow (1000.0f, (float) i / N);
            float db = s.lowShelf.magnitudeDb (sampleRate, hz) + s.airLp.magnitudeDb (sampleRate, hz)
                     + s.notch.magnitudeDb (sampleRate, hz) + s.highShelf.magnitudeDb (sampleRate, hz)
                     + (ch == 0 ? s.shadowL : s.shadowR).magnitudeDb (sampleRate, hz);
            const Point<float> pt (xFor (hz), yFor (db));
            if (i == 0) curve.startNewSubPath (pt); else curve.lineTo (pt);
        }
        const auto col = ch == 0 ? Pal::blue : Pal::gold;
        Path fill (curve);
        fill.lineTo (eq.getRight(), yFor (0)); fill.lineTo (eq.getX(), yFor (0)); fill.closeSubPath();
        g.setColour (col.withAlpha (0.08f));
        g.fillPath (fill);
        g.setColour (col);
        g.strokePath (curve, PathStrokeType (1.8f * uiScale));
    }
    g.setFont (uiFont (9.5f * uiScale, true));
    g.setColour (Pal::blue); g.drawText ("L", Rectangle<float> (eq.getRight() - 30 * uiScale, eq.getY() + 4, 12 * uiScale, 10 * uiScale), Justification::centred);
    g.setColour (Pal::gold); g.drawText ("R", Rectangle<float> (eq.getRight() - 16 * uiScale, eq.getY() + 4, 12 * uiScale, 10 * uiScale), Justification::centred);
}

//==============================================================================
StageDeckEditor::StageDeckEditor (StageDeckProcessor& p)
    : AudioProcessorEditor (p), proc (p),
      brightness (p, ParamIDs::brightness, "BRIGHTNESS", "DARK", "BRIGHT"),
      roomSize (p, ParamIDs::roomSize, "ROOM SIZE", "SMALL", "LARGE"),
      reverb (p, ParamIDs::reverb, "REVERB", "DRY", "WET"),
      decay (p, ParamIDs::decay, "DECAY", "SHORT", "LONG", true),
      distLevel (p, ParamIDs::distLevel, "DISTANCE LEVEL", "FIXED", "REAL"),
      width (p, ParamIDs::width, "SOURCE WIDTH", "MONO", "WIDE"),
      mix (p, ParamIDs::mix, "MIX", "0%", "100%"),
      output (p, ParamIDs::output, "OUTPUT", "-24", "+12"),
      stage (p)
{
    setLookAndFeel (&look);
    for (auto* k : { &brightness, &roomSize, &reverb, &decay, &distLevel, &width, &mix, &output })
        addAndMakeVisible (k);
    addAndMakeVisible (stage);
    addAndMakeVisible (info);

    heightSlider.setSliderStyle (Slider::LinearVertical);
    heightSlider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (heightSlider);
    heightAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, ParamIDs::height, heightSlider);
    heightSlider.setDoubleClickReturnValue (true, 0.5);

    iconButton.onClick = [this] { showIconMenu(); };
    addAndMakeVisible (iconButton);
    stage.onIconMenu = [this] { showIconMenu(); };

    roomPresets.setTextWhenNothingSelected ("ROOM PRESETS");
    roomPresets.addItemList ({ "Vocal Booth", "Studio Room", "Live Room", "Club", "Concert Hall", "Church", "Cathedral", "Arena" }, 1);
    roomPresets.onChange = [this]
    {
        const int i = roomPresets.getSelectedItemIndex();
        if (i >= 0) applyRoomPreset (i);
        roomPresets.setSelectedId (0, dontSendNotification);
    };
    addAndMakeVisible (roomPresets);

   #if STAGEDECK_SEND
    tailMode.addItemList ({ "TAIL: BUILT-IN REVERB", "TAIL: SEND TO BUS (OUT 3/4)" }, 1);
    tailAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, ParamIDs::tailMode, tailMode);
    addAndMakeVisible (tailMode);
    linkAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::linkRoom, linkRoom);
    linkRoom.setTooltip ("Room size, decay and brightness are shared by every StageDeck Send with Link Room on");
    addAndMakeVisible (linkRoom);
   #endif
    showOthers.setToggleState (proc.showOthers, dontSendNotification);
    showOthers.onClick = [this] { proc.showOthers = showOthers.getToggleState(); stage.repaint(); };
    addAndMakeVisible (showOthers);

    // read the remembered size first: setResizeLimits() triggers resized(), which stores the size
    const int savedW = jlimit (860, 2600, proc.editorWidth), savedH = jlimit (580, 1700, proc.editorHeight);
    setResizable (true, true);
    setResizeLimits (860, 580, 2600, 1700);
    setSize (savedW, savedH);
    startTimerHz (30);
}

StageDeckEditor::~StageDeckEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void StageDeckEditor::applyRoomPreset (int index)
{
    struct Preset { float size, decaySec, bright; };
    static const Preset presets[] = { { 0.0f, 0.3f, 0.4f }, { 0.15f, 0.6f, 0.55f }, { 0.3f, 0.9f, 0.6f }, { 0.45f, 1.3f, 0.5f },
                                      { 0.72f, 2.3f, 0.5f }, { 0.6f, 3.2f, 0.42f }, { 0.92f, 6.0f, 0.38f }, { 1.0f, 3.8f, 0.6f } };
    const auto& pr = presets[jlimit (0, 7, index)];
    for (auto [id, v] : { std::pair<const String*, float> { &ParamIDs::roomSize, pr.size }, { &ParamIDs::decay, pr.decaySec }, { &ParamIDs::brightness, pr.bright } })
    {
        proc.beginGesture (*id);
        proc.setParam (*id, v);
        proc.endGesture (*id);
    }
}

void StageDeckEditor::showIconMenu()
{
    PopupMenu m;
    for (int i = 0; i < Icons::count; ++i)
    {
        Image img (Image::ARGB, 64, 64, true);
        {
            Graphics g (img);
            Icons::draw (g, i, { 6, 6, 52, 52 }, Icons::getColour (i), Pal::panel);
        }
        PopupMenu::Item item (Icons::getName (i));
        item.itemID = i + 1;
        item.isTicked = proc.iconIndex.load() == i;
        auto d = std::make_unique<DrawableImage>();
        d->setImage (img);
        item.image = std::move (d);
        m.addItem (std::move (item));
    }
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&iconButton),
                     [safe = Component::SafePointer<StageDeckEditor> (this)] (int r)
                     {
                         if (safe != nullptr && r > 0) { safe->proc.iconIndex = r - 1; safe->stage.repaint(); }
                     });
}

void StageDeckEditor::timerCallback()
{
    stage::Scene s;
    stage::computeScene (proc.currentSceneParams(), s);
    info.setScene (s, proc.currentSceneParams().fs, proc.isSendMode());
    stage.repaint();
    const int icon = proc.iconIndex.load();
    const String label = Icons::getName (icon).toUpperCase() + "   |   " + proc.getDisplayName();
    if (iconButton.getButtonText() != label) iconButton.setButtonText (label);
    if (icon != lastIcon) { lastIcon = icon; repaint (headerArea); }
}

void StageDeckEditor::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Pal::bgTop, 0, 0, Pal::bgBottom, 0, (float) getHeight(), false));
    g.fillAll();

    // header
    auto h = headerArea.toFloat().reduced (16 * sc, 0);
    g.setFont (uiFont (22.0f * sc, true));
    const String a = "Stage", b = "Deck";
    const float aw = GlyphArrangement::getStringWidth (uiFont (22.0f * sc, true), a);
    g.setColour (Pal::text);
    g.drawText (a, h, Justification::centredLeft);
    g.setColour (Pal::blue);
    g.drawText (b, h.withTrimmedLeft (aw), Justification::centredLeft);
    float tx = aw + GlyphArrangement::getStringWidth (uiFont (22.0f * sc, true), b) + 10 * sc;
   #if STAGEDECK_SEND
    {
        const float pw = 46 * sc, ph = 18 * sc;
        auto pill = Rectangle<float> (h.getX() + tx, h.getCentreY() - ph * 0.5f, pw, ph);
        g.setColour (Pal::gold);
        g.fillRoundedRectangle (pill, ph * 0.5f);
        g.setColour (Pal::badge);
        g.setFont (uiFont (11.0f * sc, true));
        g.drawText ("SEND", pill, Justification::centred);
        tx += pw + 10 * sc;
    }
   #endif
    g.setColour (Pal::dim);
    g.setFont (uiFont (11.0f * sc, true));
    g.drawText ("SPATIAL PLACEMENT", h.withTrimmedLeft (tx + 4 * sc), Justification::centredLeft);
    g.setColour (Colours::black.withAlpha (0.35f));
    g.drawHorizontalLine (headerArea.getBottom(), 0.0f, (float) getWidth());

    // height strip labels
    auto ha = heightArea.toFloat();
    g.setColour (Pal::dim);
    g.setFont (uiFont (10.0f * sc, true));
    g.drawText ("HIGH", ha.removeFromTop (14 * sc), Justification::centred);
    g.drawText ("LOW", ha.removeFromBottom (14 * sc), Justification::centred);

}

void StageDeckEditor::resized()
{
    proc.editorWidth = getWidth();
    proc.editorHeight = getHeight();
    sc = jmin (getWidth() / 1100.0f, getHeight() / 720.0f);
    look.uiScale = sc;
    stage.uiScale = sc;
    info.uiScale = sc;

    auto r = getLocalBounds();
    headerArea = r.removeFromTop ((int) (46 * sc));
    bottomArea = r.removeFromBottom ((int) (40 * sc));
    knobArea = r.removeFromTop ((int) (132 * sc));
    middleArea = r.reduced ((int) (12 * sc), (int) (4 * sc));

    auto hd = headerArea.reduced ((int) (14 * sc), (int) (9 * sc));
    iconButton.setBounds (hd.removeFromRight ((int) (330 * sc)));

    // knob row: 3 | big | 4
    auto k = knobArea.reduced ((int) (20 * sc), (int) (8 * sc));
    const int bigW = (int) (150 * sc);
    const int small = (k.getWidth() - bigW) / 7;
    for (auto* kn : { &brightness, &roomSize, &reverb })
        kn->setBounds (k.removeFromLeft (small).reduced ((int) (4 * sc), (int) (6 * sc)));
    decay.setBounds (k.removeFromLeft (bigW).withSizeKeepingCentre (bigW, k.getHeight() + (int) (6 * sc)));
    for (auto* kn : { &distLevel, &width, &mix, &output })
        kn->setBounds (k.removeFromLeft (small).reduced ((int) (4 * sc), (int) (6 * sc)));

    auto m = middleArea;
    info.setBounds (m.removeFromRight ((int) (285 * sc)));
    m.removeFromRight ((int) (8 * sc));
    heightArea = m.removeFromRight ((int) (38 * sc));
    heightSlider.setBounds (heightArea.reduced (0, (int) (18 * sc)));
    m.removeFromRight ((int) (4 * sc));
    stage.setBounds (m);

    auto bb = bottomArea.reduced ((int) (14 * sc), (int) (7 * sc));
    roomPresets.setBounds (bb.removeFromLeft ((int) (190 * sc)));
    bb.removeFromLeft ((int) (16 * sc));
   #if STAGEDECK_SEND
    tailMode.setBounds (bb.removeFromLeft ((int) (270 * sc)));
    bb.removeFromLeft ((int) (18 * sc));
    linkRoom.setBounds (bb.removeFromLeft ((int) (140 * sc)));
    bb.removeFromLeft ((int) (6 * sc));
    showOthers.setBounds (bb.removeFromLeft ((int) (200 * sc)));
   #else
    showOthers.setBounds (bb.removeFromLeft ((int) (260 * sc)));
   #endif
}
