// Adventure Pants Recomp - first-run setup
#pragma once

#include <rex/ui/imgui_dialog.h>
#include <rex/ui/immediate_drawer.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

struct ImFont;
struct ImFontAtlas;

namespace adventure_pants {

constexpr const char* kVersion = "v1.0.0";

// <folder next to the exe>/game
std::filesystem::path GameDir();
// True once the game has been unpacked (game/default.xex exists).
bool GameInstalled();

// Headless setup for RecompLauncher's "prepare" step:
//   AdventurePantsRecomp.exe --prepare --game <package or folder>
// Prints "progress <0..1> <text>" lines and returns the process exit code.
int RunPrepare(const std::string& game_path);

// Exit codes (also listed in recomplauncher.json)
constexpr int kExitNoPackage = 2;
constexpr int kExitUnpackFailed = 3;

// Segoe UI at the sizes the setup screen uses. Call from OnConfigureFonts.
void LoadSetupFonts(ImFontAtlas* atlas);

// First-run screen: asks for the player's copy, unpacks it, then calls on_ready.
class SetupDialog : public rex::ui::ImGuiDialog {
 public:
  using Post = std::function<void(std::function<void()>)>;

  SetupDialog(rex::ui::ImGuiDrawer* drawer, rex::ui::ImmediateDrawer* immediate, Post post_to_ui,
              std::function<void()> on_ready, std::function<void()> on_quit);
  ~SetupDialog() override;

  // Starts unpacking from a path (dropped on the window, picked, or --game).
  void Start(const std::string& path);
  bool busy() const { return state_ == State::kWorking || state_ == State::kDone; }

 protected:
  void OnDraw(ImGuiIO& io) override;

 private:
  enum class State { kPick, kWorking, kError, kDone };

  void Pick(bool folder);

  Post post_;
  std::function<void()> on_ready_;
  std::function<void()> on_quit_;
  std::unique_ptr<rex::ui::ImmediateTexture> icon_;
  std::thread worker_;
  std::atomic<State> state_{State::kPick};
  std::atomic<uint64_t> done_bytes_{0};
  std::atomic<uint64_t> total_bytes_{0};
  std::mutex mutex_;
  std::string current_file_;
  std::string source_name_;
  std::string error_;
  bool ready_sent_ = false;
};

}  // namespace adventure_pants
