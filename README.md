# StageDeck

Place a sound in a 3D room by dragging its icon. StageDeck sets the panning, EQ, level, early reflections and reverb automatically.

## Using it
- **Drag** the icon to move left/right and front/back. Clicking empty floor moves the sound there.
- **Shift+drag**, the **mouse wheel** or the **HIGH/LOW strip** change the height.
- **Double-click** the icon to reset it. **Right-click** it, or use the top-right button, to choose an instrument icon.
- Every StageDeck on other tracks shows up on the same stage, labelled with its track name and colour. You can drag those icons too, so you can arrange the whole mix from one window. Turn this off with *Show other StageDeck tracks*.

## What moving it does
| Move | Result |
|---|---|
| Left / right | Constant-power pan, a little high-frequency shadow on the far ear, and stereo sources get narrower as they move to the sides |
| Further back | Lower level (how much is set by *Distance Level*), high-frequency loss from air, less proximity bass, a narrower source, early reflections closer behind the direct sound, shorter pre-delay and more reverb compared with the direct sound |
| Up / down | Higher sounds get more air plus an ear-notch cue that moves up; lower sounds get darker. Floor and ceiling reflections come from the real position |

The early reflections come from 26 mirror-image sources (walls, floor and ceiling, up to 3rd order), so their timing follows the room. The tail is an 8-line feedback delay network.

## Knobs
Brightness · Room Size · Reverb · **Decay** · Distance Level · Source Width · Mix · Output. *Room Presets* (bottom left) sets Room Size, Decay and Brightness together.

## Build
GitHub Actions builds the Windows VST3 on every push: **Actions → latest run → StageDeck-VST3-Windows**. Copy `StageDeck.vst3` to `C:\Program Files\Common Files\VST3`.

---

# StageDeck Send (for big sessions)
StageDeck Send is the same plugin with two extras. It builds as a separate VST3.

## 1. Send the reverb tail to your own reverb bus
Set **TAIL** to **SEND TO BUS (OUT 3/4)**. Each track then keeps its panning, EQ and early reflections, and sends the reverb on plugin outputs **3/4**. That send is already **pre-delayed** and **level-scaled by distance**, so far sounds still get more reverb.

REAPER setup (once per track):
1. Set the track to **4 channels**: click the **Route** button → *Track channels: 4*.
2. In the same window, add a **send** to your reverb bus with *Audio: 3/4 → 1/2*.
3. Turn off the track's **master/parent send** for channels 3/4, or just leave the send as the only path for 3/4. Only 1/2 goes to the parent by default.
4. On the reverb bus, use any reverb you like, set **100 % wet** and **0 ms pre-delay**.

Switch TAIL back to **BUILT-IN REVERB** whenever you want the self-contained version.

## 2. Link Room
With **LINK ROOM** on, Room Size, Decay and Brightness are shared by every StageDeck Send that also has Link Room on. Change them on any track and the others follow. Turn it off on a track to give that one its own room.

Note: StageDeck and StageDeck Send are separate plugins, so they don't appear on each other's stage. Use one or the other in a project.
