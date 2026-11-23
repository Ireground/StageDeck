#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

// Every StageDeck instance in the same host process publishes its position here, so each
// editor can show (and move) all the sounds on one shared stage. Message thread only.
struct StageEntry
{
    int id = 0;
    juce::String name;
    juce::Colour trackColour;
    int icon = 0;
    float x = 0, depth = 0.0f, height = 0.5f;

    // a move requested by another instance's editor; the owner applies it on its timer
    bool pendingMove = false;
    float reqX = 0, reqDepth = 0, reqHeight = 0;
    bool gestureActive = false; // true while another editor is dragging this entry
};

class StageRegistry
{
public:
    static StageRegistry& get()
    {
        static StageRegistry r;
        return r;
    }

    int add()
    {
        const juce::ScopedLock sl (lock);
        StageEntry e; e.id = ++nextId;
        entries.add (e);
        return e.id;
    }

    void remove (int id)
    {
        const juce::ScopedLock sl (lock);
        for (int i = entries.size(); --i >= 0;)
            if (entries.getReference (i).id == id) entries.remove (i);
    }

    // owner publishes its current state (returns a pending move to apply first, if any)
    bool takePendingMove (int id, float& x, float& d, float& h, bool& gestureActive)
    {
        const juce::ScopedLock sl (lock);
        for (auto& e : entries)
            if (e.id == id)
            {
                gestureActive = e.gestureActive;
                if (! e.pendingMove) return false;
                e.pendingMove = false;
                x = e.reqX; d = e.reqDepth; h = e.reqHeight;
                return true;
            }
        return false;
    }

    void publish (int id, const juce::String& name, juce::Colour col, int icon, float x, float d, float h)
    {
        const juce::ScopedLock sl (lock);
        for (auto& e : entries)
            if (e.id == id)
            {
                e.name = name; e.trackColour = col; e.icon = icon;
                if (! e.pendingMove) { e.x = x; e.depth = d; e.height = h; }
            }
    }

    // called by a foreign editor
    void requestMove (int id, float x, float d, float h)
    {
        const juce::ScopedLock sl (lock);
        for (auto& e : entries)
            if (e.id == id)
            {
                e.pendingMove = true; e.reqX = x; e.reqDepth = d; e.reqHeight = h;
                e.x = x; e.depth = d; e.height = h; // show immediately
            }
    }

    void setGesture (int id, bool active)
    {
        const juce::ScopedLock sl (lock);
        for (auto& e : entries)
            if (e.id == id) e.gestureActive = active;
    }

    juce::Array<StageEntry> snapshot() const
    {
        const juce::ScopedLock sl (lock);
        return entries;
    }

    // linked room settings (StageDeck Send)
    struct Room { int version = 0; float size = 0.4f, decay = 1.6f, bright = 0.5f; };
    Room getRoom() const { const juce::ScopedLock sl (lock); return room; }
    int publishRoom (float size, float decay, float bright)
    {
        const juce::ScopedLock sl (lock);
        room.size = size; room.decay = decay; room.bright = bright;
        return ++room.version;
    }

private:
    Room room;
    juce::CriticalSection lock;
    juce::Array<StageEntry> entries;
    int nextId = 0;
};
