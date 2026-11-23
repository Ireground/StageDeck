#pragma once
#include <juce_graphics/juce_graphics.h>

// Vector instrument icons drawn in a unit box. "fill" is drawn in the icon colour,
// "cut" is drawn on top in the badge colour to make holes / details.
namespace Icons
{
static constexpr int count = 13;

inline juce::String getName (int i)
{
    static const char* names[count] = { "Vocal", "Drums", "Keys", "Guitar", "Bass", "Synth", "Strings",
                                        "Brass", "Pad", "FX", "Percussion", "Choir", "Speaker" };
    return names[juce::jlimit (0, count - 1, i)];
}

inline juce::Colour getColour (int i)
{
    // Pro-R inspired palette: blues, golds and soft accents
    static const juce::uint32 cols[count] = { 0xfff2c14e, 0xff5ab4e8, 0xff7fd1c7, 0xffef8f5a, 0xffb48cf0, 0xff5ab4e8,
                                              0xffe8a2c0, 0xfff2c14e, 0xff8fb8ff, 0xff7fe0a0, 0xffef8f5a, 0xfff2d27a, 0xffaab8c8 };
    return juce::Colour (cols[juce::jlimit (0, count - 1, i)]);
}

struct Shape { juce::Path fill, cut; };

inline void stroke (juce::Path& dst, const juce::Path& src, float w)
{
    juce::Path s;
    juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath (s, src);
    dst.addPath (s);
}
inline void line (juce::Path& dst, float x1, float y1, float x2, float y2, float w)
{
    juce::Path p; p.startNewSubPath (x1, y1); p.lineTo (x2, y2); stroke (dst, p, w);
}

inline Shape makeUnit (int index)
{
    using juce::Path;
    Shape s; auto& f = s.fill; auto& c = s.cut;
    switch (index)
    {
        case 0: // vocal mic
        {
            f.addRoundedRectangle (0.36f, 0.06f, 0.28f, 0.44f, 0.14f);
            Path stand; stand.startNewSubPath (0.24f, 0.32f);
            stand.cubicTo (0.24f, 0.62f, 0.76f, 0.62f, 0.76f, 0.32f);
            stroke (f, stand, 0.055f);
            line (f, 0.5f, 0.58f, 0.5f, 0.86f, 0.06f);
            line (f, 0.33f, 0.9f, 0.67f, 0.9f, 0.07f);
            for (int i = 0; i < 4; ++i) line (c, 0.4f, 0.15f + i * 0.075f, 0.6f, 0.15f + i * 0.075f, 0.025f);
            break;
        }
        case 1: // drum
        {
            f.addRectangle (0.14f, 0.5f, 0.72f, 0.28f);
            f.addEllipse (0.14f, 0.68f, 0.72f, 0.2f);
            f.addEllipse (0.14f, 0.4f, 0.72f, 0.2f);
            c.addEllipse (0.19f, 0.43f, 0.62f, 0.14f);
            for (int i = 0; i < 4; ++i) line (c, 0.24f + i * 0.17f, 0.62f, 0.32f + i * 0.17f, 0.82f, 0.025f);
            line (f, 0.18f, 0.06f, 0.46f, 0.42f, 0.055f);
            line (f, 0.82f, 0.06f, 0.56f, 0.42f, 0.055f);
            f.addEllipse (0.13f, 0.02f, 0.1f, 0.1f);
            f.addEllipse (0.77f, 0.02f, 0.1f, 0.1f);
            break;
        }
        case 2: // keys
        {
            f.addRoundedRectangle (0.05f, 0.24f, 0.9f, 0.54f, 0.05f);
            c.addRectangle (0.09f, 0.37f, 0.82f, 0.025f);
            const float kw = 0.82f / 7.0f;
            for (int i = 1; i < 7; ++i) c.addRectangle (0.09f + i * kw - 0.01f, 0.4f, 0.02f, 0.34f);
            const int black[] = { 1, 2, 4, 5, 6 };
            for (int b : black) c.addRectangle (0.09f + b * kw - 0.032f, 0.4f, 0.064f, 0.2f);
            for (int i = 0; i < 4; ++i) c.addEllipse (0.12f + i * 0.07f, 0.285f, 0.04f, 0.04f);
            break;
        }
        case 3: // guitar
        case 4: // bass
        {
            const bool bass = index == 4;
            const float lr = bass ? 0.17f : 0.2f, ur = bass ? 0.13f : 0.155f;
            const float ly = bass ? 0.76f : 0.72f, uy = bass ? 0.58f : 0.52f;
            f.addEllipse (0.5f - lr, ly - lr, lr * 2, lr * 2);
            f.addEllipse (0.5f - ur, uy - ur, ur * 2, ur * 2);
            f.addRectangle (0.465f, bass ? 0.05f : 0.1f, 0.07f, 0.4f);
            f.addRoundedRectangle (0.43f, bass ? 0.01f : 0.04f, 0.14f, 0.09f, 0.02f);
            if (! bass) c.addEllipse (0.44f, 0.56f, 0.12f, 0.12f);
            else line (c, 0.42f, 0.7f, 0.58f, 0.7f, 0.03f);
            line (c, 0.42f, ly + lr * 0.45f, 0.58f, ly + lr * 0.45f, 0.035f);
            break;
        }
        case 5: // synth
        {
            f.addRoundedRectangle (0.06f, 0.2f, 0.88f, 0.6f, 0.08f);
            Path w; w.startNewSubPath (0.16f, 0.5f);
            for (int i = 1; i <= 40; ++i)
            {
                const float t = (float) i / 40.0f;
                w.lineTo (0.16f + t * 0.68f, 0.5f - 0.17f * std::sin (t * juce::MathConstants<float>::twoPi * 1.5f));
            }
            stroke (c, w, 0.06f);
            break;
        }
        case 6: // strings (violin + bow)
        {
            f.addEllipse (0.33f, 0.52f, 0.34f, 0.36f);
            f.addEllipse (0.36f, 0.3f, 0.28f, 0.3f);
            f.addRectangle (0.47f, 0.06f, 0.06f, 0.3f);
            f.addEllipse (0.45f, 0.02f, 0.1f, 0.08f);
            line (c, 0.42f, 0.55f, 0.42f, 0.7f, 0.025f);
            line (c, 0.58f, 0.55f, 0.58f, 0.7f, 0.025f);
            line (f, 0.1f, 0.86f, 0.9f, 0.24f, 0.035f);
            break;
        }
        case 7: // brass
        {
            Path bell; bell.startNewSubPath (0.58f, 0.44f);
            bell.quadraticTo (0.78f, 0.42f, 0.94f, 0.22f); bell.lineTo (0.94f, 0.78f);
            bell.quadraticTo (0.78f, 0.58f, 0.58f, 0.56f); bell.closeSubPath();
            f.addPath (bell);
            f.addRoundedRectangle (0.1f, 0.44f, 0.52f, 0.12f, 0.04f);
            f.addRoundedRectangle (0.04f, 0.46f, 0.08f, 0.08f, 0.02f);
            for (int i = 0; i < 3; ++i) f.addRoundedRectangle (0.26f + i * 0.11f, 0.26f, 0.07f, 0.2f, 0.02f);
            Path loop; loop.addRoundedRectangle (0.2f, 0.56f, 0.38f, 0.16f, 0.08f);
            stroke (f, loop, 0.05f);
            break;
        }
        case 8: // pad (cloud)
        {
            f.addEllipse (0.12f, 0.42f, 0.3f, 0.3f);
            f.addEllipse (0.28f, 0.24f, 0.4f, 0.4f);
            f.addEllipse (0.55f, 0.36f, 0.32f, 0.32f);
            f.addRoundedRectangle (0.27f, 0.5f, 0.45f, 0.22f, 0.1f);
            Path w; w.startNewSubPath (0.18f, 0.84f); w.quadraticTo (0.34f, 0.76f, 0.5f, 0.84f); w.quadraticTo (0.66f, 0.92f, 0.82f, 0.84f);
            stroke (f, w, 0.045f);
            break;
        }
        case 9: // fx sparkle
        {
            auto star = [&f] (float cx, float cy, float r)
            {
                Path p; p.startNewSubPath (cx, cy - r);
                p.quadraticTo (cx, cy, cx + r, cy); p.quadraticTo (cx, cy, cx, cy + r);
                p.quadraticTo (cx, cy, cx - r, cy); p.quadraticTo (cx, cy, cx, cy - r); p.closeSubPath();
                f.addPath (p);
            };
            star (0.42f, 0.55f, 0.36f); star (0.76f, 0.22f, 0.16f); star (0.8f, 0.78f, 0.1f);
            break;
        }
        case 10: // percussion (shaker)
        {
            f.addEllipse (0.12f, 0.1f, 0.5f, 0.5f);
            line (f, 0.52f, 0.5f, 0.86f, 0.88f, 0.1f);
            for (int i = 0; i < 3; ++i) line (c, 0.22f + i * 0.1f, 0.3f - i * 0.06f, 0.3f + i * 0.1f, 0.42f - i * 0.06f, 0.035f);
            break;
        }
        case 11: // choir
        {
            f.addEllipse (0.14f, 0.2f, 0.26f, 0.26f);
            f.addEllipse (0.6f, 0.2f, 0.26f, 0.26f);
            f.addEllipse (0.36f, 0.08f, 0.28f, 0.28f);
            Path b1; b1.addRoundedRectangle (0.06f, 0.52f, 0.42f, 0.4f, 0.16f); f.addPath (b1);
            Path b2; b2.addRoundedRectangle (0.52f, 0.52f, 0.42f, 0.4f, 0.16f); f.addPath (b2);
            Path b3; b3.addRoundedRectangle (0.27f, 0.4f, 0.46f, 0.52f, 0.18f); f.addPath (b3);
            c.addEllipse (0.44f, 0.18f, 0.12f, 0.08f);
            break;
        }
        default: // speaker
        {
            f.addRoundedRectangle (0.2f, 0.05f, 0.6f, 0.9f, 0.06f);
            c.addEllipse (0.3f, 0.43f, 0.4f, 0.4f);
            c.addEllipse (0.41f, 0.14f, 0.18f, 0.18f);
            f.addEllipse (0.43f, 0.56f, 0.14f, 0.14f);
            break;
        }
    }
    return s;
}

inline void draw (juce::Graphics& g, int index, juce::Rectangle<float> box, juce::Colour fg, juce::Colour bg)
{
    auto s = makeUnit (index);
    const auto t = juce::AffineTransform::scale (box.getWidth(), box.getHeight()).translated (box.getX(), box.getY());
    s.fill.applyTransform (t); s.cut.applyTransform (t);
    g.setColour (fg); g.fillPath (s.fill);
    g.setColour (bg); g.fillPath (s.cut);
}
} // namespace Icons
