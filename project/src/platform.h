// Adventure Pants Recomp - the few bits that talk to Windows directly.
// Kept in their own file so <windows.h> macros (small, DrawText, ...) can't
// leak into the rest of the code.
#pragma once

#include <string>

namespace adventure_pants {

// Native file/folder picker. Returns a UTF-8 path, or an empty string if cancelled.
std::string BrowseForCopy(bool folder);

// Writes to stdout even from a windowed (GUI) program, for RecompLauncher's setup step.
void WriteStdout(const std::string& text);

}  // namespace adventure_pants
