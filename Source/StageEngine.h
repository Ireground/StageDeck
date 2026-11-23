#pragma once
// StageEngine — JUCE-independent DSP for StageDeck.
// A source position in a virtual room (left/right, depth, height) is turned into:
//   * direct sound: panning, distance level, air absorption, proximity, elevation/pinna cues, head shadow
//   * early reflections: 26 image sources (walls, floor, ceiling, up to 3rd order) — timing follows the geometry
//   * late reverb: 8-line feedback delay network with pre-delay derived from the room and the position

#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>

namespace stage
{
constexpr float kPi = 3.14159265358979f;
constexpr float kSpeedOfSound = 343.0f;
constexpr int   kNumTaps = 26;

inline float lerp (float a, float b, float t) { return a + (b - a) * t; }
inline float clamp01 (float v) { return std::min (1.0f, std::max (0.0f, v)); }
inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g) { return 20.0f * std::log10 (std::max (g, 1.0e-6f)); }

//==============================================================================
struct BiquadCoeffs
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

    // RBJ cookbook designs
    static BiquadCoeffs lowPass (double fs, double f, double q)
    {
        f = std::min (f, fs * 0.45);
        const double w = 2.0 * 3.141592653589793 * f / fs, c = std::cos (w), s = std::sin (w), al = s / (2.0 * q);
        const double a0 = 1.0 + al;
        return make ((1.0 - c) * 0.5 / a0, (1.0 - c) / a0, (1.0 - c) * 0.5 / a0, -2.0 * c / a0, (1.0 - al) / a0);
    }
    static BiquadCoeffs peak (double fs, double f, double q, double db)
    {
        f = std::min (f, fs * 0.45);
        const double A = std::pow (10.0, db / 40.0);
        const double w = 2.0 * 3.141592653589793 * f / fs, c = std::cos (w), s = std::sin (w), al = s / (2.0 * q);
        const double a0 = 1.0 + al / A;
        return make ((1.0 + al * A) / a0, -2.0 * c / a0, (1.0 - al * A) / a0, -2.0 * c / a0, (1.0 - al / A) / a0);
    }
    static BiquadCoeffs shelf (double fs, double f, double db, bool high)
    {
        f = std::min (f, fs * 0.45);
        const double A = std::pow (10.0, db / 40.0);
        const double w = 2.0 * 3.141592653589793 * f / fs, c = std::cos (w), s = std::sin (w);
        const double al = s / 2.0 * std::sqrt (2.0); // S = 1
        const double sa = 2.0 * std::sqrt (A) * al;
        if (high)
        {
            const double a0 = (A + 1) - (A - 1) * c + sa;
            return make (A * ((A + 1) + (A - 1) * c + sa) / a0, -2 * A * ((A - 1) + (A + 1) * c) / a0,
                         A * ((A + 1) + (A - 1) * c - sa) / a0, 2 * ((A - 1) - (A + 1) * c) / a0,
                         ((A + 1) - (A - 1) * c - sa) / a0);
        }
        const double a0 = (A + 1) + (A - 1) * c + sa;
        return make (A * ((A + 1) - (A - 1) * c + sa) / a0, 2 * A * ((A - 1) - (A + 1) * c) / a0,
                     A * ((A + 1) - (A - 1) * c - sa) / a0, -2 * ((A - 1) + (A + 1) * c) / a0,
                     ((A + 1) + (A - 1) * c - sa) / a0);
    }

    // magnitude in dB at frequency f (for the GUI curve)
    float magnitudeDb (double fs, double f) const
    {
        const double w = 2.0 * 3.141592653589793 * f / fs;
        const double c1 = std::cos (w), s1 = std::sin (w), c2 = std::cos (2 * w), s2 = std::sin (2 * w);
        const double nr = b0 + b1 * c1 + b2 * c2, ni = -(b1 * s1 + b2 * s2);
        const double dr = 1.0 + a1 * c1 + a2 * c2, di = -(a1 * s1 + a2 * s2);
        const double m = std::sqrt ((nr * nr + ni * ni) / std::max (1e-20, dr * dr + di * di));
        return (float) (20.0 * std::log10 (std::max (m, 1e-9)));
    }

private:
    static BiquadCoeffs make (double b0, double b1, double b2, double a1, double a2)
    {
        BiquadCoeffs c; c.b0 = (float) b0; c.b1 = (float) b1; c.b2 = (float) b2; c.a1 = (float) a1; c.a2 = (float) a2; return c;
    }
};

struct Biquad
{
    BiquadCoeffs c;
    float z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    inline float process (float x)
    {
        const float y = c.b0 * x + z1;
        z1 = c.b1 * x - c.a1 * y + z2;
        z2 = c.b2 * x - c.a2 * y;
        return y;
    }
};

//==============================================================================
struct SceneParams
{
    float x = 0.0f;          // -1 (left) .. +1 (right)
    float depth = 0.3f;      // 0 (close) .. 1 (back wall)
    float height = 0.5f;     // 0 (floor) .. 0.5 (ear level) .. 1 (ceiling)
    float roomSize = 0.4f;   // 0 (booth) .. 1 (arena)
    float decay = 1.6f;      // seconds (RT60 of the late reverb)
    float brightness = 0.5f; // 0 dark .. 1 bright (wall / reverb damping)
    float reverbDb = 0.0f;   // trim of early + late
    float distAmount = 0.5f; // 0 = level stays, 1 = physical inverse-distance law
    float width = 1.0f;      // stereo width of the source
    double fs = 48000.0;
};

struct Scene
{
    // geometry (metres)
    float roomW = 0, roomD = 0, roomH = 0;
    float lx = 0, ly = 0, lz = 0, sx = 0, sy = 0, sz = 0;
    float distance = 1, elevationDeg = 0, farness = 0, pan = 0;

    // direct path
    float directGain = 1, effWidth = 1;
    float lowShelfDb = 0, airLpHz = 20000, notchHz = 7000, notchDb = 0, highShelfDb = 0, shadowDbL = 0, shadowDbR = 0;
    BiquadCoeffs lowShelf, airLp, notch, highShelf, shadowL, shadowR;

    // reflections / reverb
    std::array<float, kNumTaps> tapDelaySec {}, tapGainL {}, tapGainR {};
    float firstReflMs = 0, latePredelayMs = 0, lateGain = 0, lateBalance = 0, erGainDb = -100;
    float wallLpHz = 8000, loopLpHz = 6000;

    static constexpr float shelfLowHz = 180.0f, shelfHighHz = 7500.0f, shadowHz = 2500.0f;
};

// height control <-> metres (0 = floor, 0.5 = ear level, 1 = ceiling)
inline float earHeight (float roomH) { return std::min (1.5f, roomH * 0.45f); }
inline float roomHeightFor (float roomSize) { return lerp (2.5f, 16.0f, std::pow (clamp01 (roomSize), 1.5f)); }
inline float heightToZ (float h, float roomH)
{
    const float zLow = 0.15f, zHigh = roomH - 0.25f, lz = earHeight (roomH);
    h = clamp01 (h);
    return h < 0.5f ? lerp (zLow, lz, h * 2.0f) : lerp (lz, zHigh, (h - 0.5f) * 2.0f);
}
inline float zToHeight (float z, float roomH)
{
    const float zLow = 0.15f, zHigh = roomH - 0.25f, lz = earHeight (roomH);
    if (z <= lz) return clamp01 (0.5f * (z - zLow) / (lz - zLow));
    return clamp01 (0.5f + 0.5f * (z - lz) / (zHigh - lz));
}

inline void computeScene (const SceneParams& p, Scene& s)
{
    const float rs = clamp01 (p.roomSize);
    s.roomW = lerp (3.5f, 32.0f, std::pow (rs, 1.4f));
    s.roomD = lerp (4.5f, 42.0f, std::pow (rs, 1.4f));
    s.roomH = roomHeightFor (rs);

    s.lx = 0.0f; s.ly = 0.6f; s.lz = earHeight (s.roomH);

    const float x = std::max (-1.0f, std::min (1.0f, p.x));
    const float depth = clamp01 (p.depth), h = clamp01 (p.height);

    s.sx = x * (s.roomW * 0.5f - 0.4f);
    const float nearD = 0.8f, farD = std::max (nearD + 0.5f, s.roomD - 0.4f - s.ly);
    s.sy = s.ly + nearD * std::pow (farD / nearD, depth);
    s.sz = heightToZ (h, s.roomH);

    const float dx = s.sx - s.lx, dy = s.sy - s.ly, dz = s.sz - s.lz;
    const float horiz = std::sqrt (dx * dx + dy * dy);
    s.distance = std::max (0.3f, std::sqrt (horiz * horiz + dz * dz));
    const float elev = std::atan2 (dz, horiz);
    s.elevationDeg = elev * 180.0f / kPi;
    const float e = std::sin (elev);

    // perceptual "farness" mixes the depth control with the true distance so a big room sounds further away
    s.farness = clamp01 (0.65f * depth + 0.35f * std::log (s.distance / nearD) / std::log (50.0f));
    const float f = s.farness;

    s.pan = x * (1.0f - 0.25f * depth);

    // level: physical gain^amount; the same normalisation is applied to reflections so the
    // direct/reverb ratio stays natural even when "Distance Level" is turned down
    const float gp = std::min (1.0f, nearD / s.distance);
    const float amt = clamp01 (p.distAmount);
    const float norm = std::pow (gp, amt - 1.0f);
    s.directGain = std::max (dbToGain (-30.0f), std::pow (gp, amt));

    s.effWidth = clamp01 (p.width) * (1.0f - 0.65f * f) * (1.0f - 0.85f * std::abs (s.pan));

    // tone of the direct sound
    s.airLpHz    = 20000.0f * std::pow (4500.0f / 20000.0f, std::pow (f, 1.1f));
    s.lowShelfDb = 3.5f * std::pow (1.0f - f, 3.0f) - 2.5f * f;          // proximity up close, thinner far away
    s.highShelfDb = 4.0f * std::max (e, 0.0f) - 4.5f * std::max (-e, 0.0f); // above = airy, below = darker
    s.notchHz = 6500.0f * std::pow (2.0f, e * 0.8f);                      // pinna notch moves with elevation
    s.notchDb = -6.0f * std::abs (e);
    s.shadowDbL = -4.0f * std::pow (std::max (s.pan, 0.0f), 1.5f);        // head shadow on the far ear
    s.shadowDbR = -4.0f * std::pow (std::max (-s.pan, 0.0f), 1.5f);

    const double fs = p.fs;
    s.lowShelf  = BiquadCoeffs::shelf (fs, Scene::shelfLowHz, s.lowShelfDb, false);
    s.airLp     = BiquadCoeffs::lowPass (fs, s.airLpHz, 0.7071);
    s.notch     = BiquadCoeffs::peak (fs, s.notchHz, 3.0, s.notchDb);
    s.highShelf = BiquadCoeffs::shelf (fs, Scene::shelfHighHz, s.highShelfDb, true);
    s.shadowL   = BiquadCoeffs::shelf (fs, Scene::shadowHz, s.shadowDbL, true);
    s.shadowR   = BiquadCoeffs::shelf (fs, Scene::shadowHz, s.shadowDbR, true);

    // early reflections from image sources
    const float trim = dbToGain (p.reverbDb);
    const float W = s.roomW, D = s.roomD, H = s.roomH;
    int t = 0;
    float firstRefl = 1.0e9f, erEnergy = 0.0f;
    for (int i = -1; i <= 1; ++i)
        for (int j = -1; j <= 1; ++j)
            for (int k = -1; k <= 1; ++k)
            {
                if (i == 0 && j == 0 && k == 0) continue;
                const float ix = i == 0 ? s.sx : (i < 0 ? -W - s.sx : W - s.sx);
                const float iy = j == 0 ? s.sy : (j < 0 ? -s.sy : 2.0f * D - s.sy);
                const float iz = k == 0 ? s.sz : (k < 0 ? -s.sz : 2.0f * H - s.sz);
                const float ex = ix - s.lx, ey = iy - s.ly, ez = iz - s.lz;
                const float L = std::sqrt (ex * ex + ey * ey + ez * ez);
                float refl = 1.0f;
                if (i != 0) refl *= 0.72f;
                if (j != 0) refl *= (j < 0 ? 0.6f : 0.72f);
                if (k != 0) refl *= (k < 0 ? 0.8f : 0.68f);
                const float g = refl * (nearD / L) * norm * trim * 0.9f;
                const float az = std::atan2 (ex, ey);
                const float tp = std::sin (az) * 0.9f;               // -1..1
                const float ang = (tp + 1.0f) * kPi * 0.25f;
                s.tapDelaySec[(size_t) t] = std::max (0.0f, (L - s.distance) / kSpeedOfSound);
                s.tapGainL[(size_t) t] = g * std::cos (ang);
                s.tapGainR[(size_t) t] = g * std::sin (ang);
                firstRefl = std::min (firstRefl, s.tapDelaySec[(size_t) t]);
                erEnergy += g * g;
                ++t;
            }
    s.firstReflMs = firstRefl * 1000.0f;
    s.erGainDb = gainToDb (std::sqrt (erEnergy));

    const float V = W * D * H, S = 2.0f * (W * D + W * H + D * H);
    const float mfp = 4.0f * V / S;
    s.latePredelayMs = s.firstReflMs + 1000.0f * mfp / kSpeedOfSound * 0.5f;
    s.lateGain = 0.22f * norm * trim;
    s.lateBalance = s.pan * 0.35f;

    const float b = clamp01 (p.brightness);
    s.wallLpHz = 3000.0f * std::pow (16000.0f / 3000.0f, b);
    s.loopLpHz = 1500.0f * std::pow (14000.0f / 1500.0f, b);
}

//==============================================================================
class DelayLine
{
public:
    void prepare (int size) { buf.assign ((size_t) size + 4, 0.0f); w = 0; mask = (int) buf.size(); }
    void reset() { std::fill (buf.begin(), buf.end(), 0.0f); w = 0; }
    inline void push (float x) { buf[(size_t) w] = x; if (++w >= mask) w = 0; }
    // delay in samples, >= 1 means "one sample ago" is the latest pushed sample
    inline float read (float d) const
    {
        d = std::max (1.0f, std::min (d, (float) (mask - 3)));
        const int di = (int) d; const float fr = d - (float) di;
        int i0 = w - di; if (i0 < 0) i0 += mask;
        int i1 = i0 - 1; if (i1 < 0) i1 += mask;
        return buf[(size_t) i0] + fr * (buf[(size_t) i1] - buf[(size_t) i0]);
    }
private:
    std::vector<float> buf; int w = 0, mask = 1;
};

struct Allpass
{
    DelayLine dl; float delay = 100, g = 0.6f;
    inline float process (float x)
    {
        const float d = dl.read (delay);
        const float v = x + g * d;
        dl.push (v);
        return d - g * v;
    }
};

struct OnePoleLP
{
    float a = 0.5f, z = 0;
    void set (double fs, double hz) { a = (float) (1.0 - std::exp (-2.0 * 3.141592653589793 * std::min (hz, fs * 0.45) / fs)); }
    inline float process (float x) { z += a * (x - z); return z; }
};

//==============================================================================
struct EngineParams
{
    SceneParams scene;
    float mix = 1.0f;
    float outputDb = 0.0f;
};

class StageEngine
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        const int maxDelay = (int) (fs * 0.6) + 8;
        erLine.prepare (maxDelay);
        preLine.prepare (maxDelay);
        static const float apMs[4] = { 4.7f, 3.6f, 12.7f, 9.3f };
        for (int i = 0; i < 4; ++i) { diff[i].dl.prepare ((int) (fs * 0.05) + 8); diff[i].delay = apMs[i] * 0.001f * (float) fs; diff[i].g = 0.62f; }
        for (auto& l : fdn) l.prepare ((int) (fs * 0.4) + 8);
        initialised = false;
        reset();
    }

    void reset()
    {
        erLine.reset(); preLine.reset();
        for (auto& d : diff) d.dl.reset();
        for (auto& l : fdn) l.reset();
        for (auto& lp : loopLp) lp.z = 0;
        erLpL.z = erLpR.z = 0;
        for (auto* f : { &dLs[0], &dLs[1], &dAir[0], &dAir[1], &dNotch[0], &dNotch[1], &dHs[0], &dHs[1], &dShadow[0], &dShadow[1] })
            f->reset();
        lfoPhase = 0;
        initialised = false;
    }

    // smoothing targets — call once per block
    void setTargets (const EngineParams& p) { target = p; }

    // in-place stereo processing; if right == nullptr the input is mono and only left is written.
    void process (float* left, float* right, int numSamples)
    {
        constexpr int sub = 32;
        if (! initialised) { cur = target; updateScene (true); initialised = true; }

        int pos = 0;
        while (pos < numSamples)
        {
            const int n = std::min (sub, numSamples - pos);
            smoothParams (n);
            updateScene (false);
            processChunk (left + pos, right != nullptr ? right + pos : nullptr, n);
            pos += n;
        }
    }

    const Scene& getScene() const { return scene; }

private:
    void smoothParams (int n)
    {
        // ~70 ms time constant for positions, ~150 ms for room settings
        const float aPos  = 1.0f - std::exp (-(float) n / (0.07f * (float) fs));
        const float aRoom = 1.0f - std::exp (-(float) n / (0.15f * (float) fs));
        auto& c = cur.scene; const auto& t = target.scene;
        c.x += aPos * (t.x - c.x);
        c.depth += aPos * (t.depth - c.depth);
        c.height += aPos * (t.height - c.height);
        c.roomSize += aRoom * (t.roomSize - c.roomSize);
        c.decay += aRoom * (t.decay - c.decay);
        c.brightness += aRoom * (t.brightness - c.brightness);
        c.reverbDb += aPos * (t.reverbDb - c.reverbDb);
        c.distAmount += aPos * (t.distAmount - c.distAmount);
        c.width += aPos * (t.width - c.width);
        cur.mix += aPos * (target.mix - cur.mix);
        cur.outputDb += aPos * (target.outputDb - cur.outputDb);
    }

    void updateScene (bool snap)
    {
        cur.scene.fs = fs;
        const auto& a = cur.scene; const auto& b = lastComputed;
        const bool unchanged = std::abs (a.x - b.x) < 1e-5f && std::abs (a.depth - b.depth) < 1e-5f && std::abs (a.height - b.height) < 1e-5f
            && std::abs (a.roomSize - b.roomSize) < 1e-5f && std::abs (a.decay - b.decay) < 1e-4f && std::abs (a.brightness - b.brightness) < 1e-5f
            && std::abs (a.reverbDb - b.reverbDb) < 1e-4f && std::abs (a.distAmount - b.distAmount) < 1e-5f && std::abs (a.width - b.width) < 1e-5f;
        tgtMix = cur.mix;
        tgtOut = dbToGain (cur.outputDb);
        if (unchanged && ! snap) return;
        lastComputed = cur.scene;
        computeScene (cur.scene, scene);

        for (int c = 0; c < 2; ++c)
        {
            dLs[c].c = scene.lowShelf; dAir[c].c = scene.airLp; dNotch[c].c = scene.notch; dHs[c].c = scene.highShelf;
        }
        dShadow[0].c = scene.shadowL; dShadow[1].c = scene.shadowR;

        erLpL.set (fs, scene.wallLpHz); erLpR.set (fs, scene.wallLpHz);
        for (auto& lp : loopLp) lp.set (fs, scene.loopLpHz);

        // pan gains (constant power, unity at centre)
        const float ang = (scene.pan + 1.0f) * kPi * 0.25f;
        tgtPanL = std::sqrt (2.0f) * std::cos (ang);
        tgtPanR = std::sqrt (2.0f) * std::sin (ang);
        tgtDirect = scene.directGain;
        tgtWidth = scene.effWidth;
        for (int i = 0; i < kNumTaps; ++i)
        {
            tgtTapD[(size_t) i] = 1.0f + scene.tapDelaySec[(size_t) i] * (float) fs;
            tgtTapL[(size_t) i] = scene.tapGainL[(size_t) i];
            tgtTapR[(size_t) i] = scene.tapGainR[(size_t) i];
        }
        tgtPre = 1.0f + scene.latePredelayMs * 0.001f * (float) fs;
        const float lb = scene.lateBalance;
        const float la = (lb + 1.0f) * kPi * 0.25f;
        tgtLateL = scene.lateGain * std::sqrt (2.0f) * std::cos (la);
        tgtLateR = scene.lateGain * std::sqrt (2.0f) * std::sin (la);
        // FDN delay lengths scale with the room; feedback from RT60
        static const float baseMs[8] = { 31.7f, 37.3f, 41.9f, 47.3f, 53.1f, 61.3f, 67.7f, 79.1f };
        const float sizeScale = lerp (0.35f, 2.0f, std::pow (clamp01 (cur.scene.roomSize), 0.8f));
        const float rt = std::max (0.1f, cur.scene.decay);
        float gsum = 0;
        for (int i = 0; i < 8; ++i)
        {
            tgtFdnD[(size_t) i] = baseMs[i] * sizeScale * 0.001f * (float) fs;
            fdnG[(size_t) i] = std::pow (10.0f, -3.0f * (baseMs[i] * sizeScale * 0.001f) / rt);
            gsum += fdnG[(size_t) i] * fdnG[(size_t) i];
        }
        // partial energy compensation so long decays are not overwhelmingly loud
        fdnInScale = std::pow (std::max (0.02f, 1.0f - gsum / 8.0f), 0.4f);

        if (snap)
        {
            mixV = tgtMix; outV = tgtOut;
            panL = tgtPanL; panR = tgtPanR; direct = tgtDirect; width = tgtWidth;
            tapD = tgtTapD; tapL = tgtTapL; tapR = tgtTapR; pre = tgtPre;
            lateL = tgtLateL; lateR = tgtLateR; mixV = tgtMix; outV = tgtOut; fdnD = tgtFdnD;
        }
    }

    void processChunk (float* left, float* right, int n)
    {
        const float inv = 1.0f / (float) n;
        // per-sample ramp increments
        const float dPanL = (tgtPanL - panL) * inv, dPanR = (tgtPanR - panR) * inv, dDir = (tgtDirect - direct) * inv;
        const float dWid = (tgtWidth - width) * inv, dPre = (tgtPre - pre) * inv;
        const float dLL = (tgtLateL - lateL) * inv, dLR = (tgtLateR - lateR) * inv;
        const float dMix = (tgtMix - mixV) * inv, dOut = (tgtOut - outV) * inv;
        std::array<float, kNumTaps> dTD, dTL, dTR;
        for (size_t i = 0; i < (size_t) kNumTaps; ++i)
        {
            dTD[i] = (tgtTapD[i] - tapD[i]) * inv; dTL[i] = (tgtTapL[i] - tapL[i]) * inv; dTR[i] = (tgtTapR[i] - tapR[i]) * inv;
        }
        const float fdnSmooth = 1.0f - std::exp (-1.0f / (0.2f * (float) fs));
        const float lfoInc = 2.0f * kPi * 0.37f / (float) fs;

        for (int i = 0; i < n; ++i)
        {
            const float inL = left[i];
            const float inR = right != nullptr ? right[i] : inL;
            const float M = 0.5f * (inL + inR), S = 0.5f * (inL - inR);

            panL += dPanL; panR += dPanR; direct += dDir; width += dWid; pre += dPre;
            lateL += dLL; lateR += dLR; mixV += dMix; outV += dOut;

            // ---- direct
            float l = panL * (M + width * S);
            float r = panR * (M - width * S);
            l = dShadow[0].process (dHs[0].process (dNotch[0].process (dAir[0].process (dLs[0].process (l)))));
            r = dShadow[1].process (dHs[1].process (dNotch[1].process (dAir[1].process (dLs[1].process (r)))));
            l *= direct; r *= direct;

            // ---- early reflections
            erLine.push (M);
            float eL = 0, eR = 0;
            for (size_t t = 0; t < (size_t) kNumTaps; ++t)
            {
                tapD[t] += dTD[t]; tapL[t] += dTL[t]; tapR[t] += dTR[t];
                const float v = erLine.read (tapD[t]);
                eL += v * tapL[t]; eR += v * tapR[t];
            }
            eL = erLpL.process (eL); eR = erLpR.process (eR);

            // ---- late reverb (FDN)
            preLine.push (M);
            float x = preLine.read (pre);
            for (auto& d : diff) x = d.process (x);
            x *= fdnInScale;

            lfoPhase += lfoInc; if (lfoPhase > 2.0f * kPi) lfoPhase -= 2.0f * kPi;
            const float mod = std::sin (lfoPhase) * 0.0004f * (float) fs;
            std::array<float, 8> o;
            for (size_t k = 0; k < 8; ++k)
            {
                fdnD[k] += fdnSmooth * (tgtFdnD[k] - fdnD[k]);
                float d = fdnD[k];
                if (k == 1) d += mod; else if (k == 4) d -= mod * 0.7f; else if (k == 6) d += mod * 0.5f;
                o[k] = fdn[k].read (d);
            }
            const float outLate0 = (o[0] + o[2] + o[4] + o[6]) * 0.5f;
            const float outLate1 = (o[1] + o[3] + o[5] + o[7]) * 0.5f;
            // Hadamard mix
            std::array<float, 8> h = o;
            for (size_t len = 1; len < 8; len <<= 1)
                for (size_t a = 0; a < 8; a += len << 1)
                    for (size_t b = a; b < a + len; ++b) { const float u = h[b], v = h[b + len]; h[b] = u + v; h[b + len] = u - v; }
            const float hs = 0.35355339f;
            for (size_t k = 0; k < 8; ++k)
                fdn[k].push (loopLp[k].process (h[k] * hs * fdnG[k]) + x);

            const float wl = l + eL + outLate0 * lateL;
            const float wr = r + eR + outLate1 * lateR;

            const float oL = (inL * (1.0f - mixV) + wl * mixV) * outV;
            const float oR = (inR * (1.0f - mixV) + wr * mixV) * outV;
            if (right != nullptr) { left[i] = oL; right[i] = oR; }
            else left[i] = 0.5f * (oL + oR);
        }
    }

    double fs = 48000.0;
    bool initialised = false;
    EngineParams target, cur;
    SceneParams lastComputed { -9.0f };
    Scene scene;

    Biquad dLs[2], dAir[2], dNotch[2], dHs[2], dShadow[2];
    DelayLine erLine, preLine;
    Allpass diff[4];
    std::array<DelayLine, 8> fdn;
    std::array<OnePoleLP, 8> loopLp;
    OnePoleLP erLpL, erLpR;
    std::array<float, 8> fdnD {}, tgtFdnD {}, fdnG {};
    float fdnInScale = 1.0f, lfoPhase = 0.0f;

    float panL = 1, panR = 1, direct = 1, width = 1, pre = 1, lateL = 0, lateR = 0, mixV = 1, outV = 1;
    float tgtPanL = 1, tgtPanR = 1, tgtDirect = 1, tgtWidth = 1, tgtPre = 1, tgtLateL = 0, tgtLateR = 0, tgtMix = 1, tgtOut = 1;
    std::array<float, kNumTaps> tapD {}, tapL {}, tapR {}, tgtTapD {}, tgtTapL {}, tgtTapR {};
};
} // namespace stage
