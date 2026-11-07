// Adventure Pants Recomp - frame counter
#pragma once
#include <rex/ui/overlay/debug_overlay.h>

namespace adventure_pants {
// Called once per game frame (from the game's present function).
void OnGameFrame();
// Frames per second and frame time, averaged over the last half second.
rex::ui::FrameStats GetFrameStats();
// show_fps setting from FancyPants.toml
bool FpsCounterEnabled();
}  // namespace adventure_pants
