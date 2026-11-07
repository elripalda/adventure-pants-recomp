// Adventure Pants Recomp - app

#pragma once

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/rex_app.h>
#include <rex/ui/imgui_dialog.h>
#include <rex/ui/imgui_drawer.h>
#include <rex/ui/keybinds.h>
#include <rex/ui/ui_event.h>
#include <rex/ui/window.h>

#include <imgui.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

#include "fps_counter.h"
#include "setup.h"

namespace adventure_pants {

constexpr const char* kTitle = "Adventure Pants Recomp";

// Small frame counter in the top-left corner (show_fps in AdventurePantsRecomp.toml).
class FpsOverlay : public rex::ui::ImGuiDialog {
 public:
  explicit FpsOverlay(rex::ui::ImGuiDrawer* drawer) : rex::ui::ImGuiDialog(drawer) {}

 protected:
  void OnDraw(ImGuiIO& io) override {
    (void)io;
    if (!FpsCounterEnabled()) return;
    auto stats = GetFrameStats();
    ImGui::SetNextWindowPos(ImVec2(12, 12));
    ImGui::SetNextWindowBgAlpha(0.45f);
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##adventure_pants_fps", nullptr, flags)) {
      if (stats.frame_count > 0)
        ImGui::Text("%.0f FPS  %.1f ms", stats.fps, stats.frame_time_ms);
      else
        ImGui::Text("-- FPS");
    }
    ImGui::End();
  }
};

}  // namespace adventure_pants

class AdventurePantsApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(rex::ui::WindowedAppContext& ctx);

  // Game files live in a "game" folder next to the exe.
  void OnConfigurePaths(rex::PathConfig& paths) override {
    if (paths.game_data_root.empty() && adventure_pants::GameInstalled())
      paths.game_data_root = adventure_pants::GameDir();
    // Shader cache next to the exe, so the Documents folder holds only saves
    // (that's the folder RecompLauncher backs up).
    if (paths.cache_root == paths.user_data_root / "cache")
      paths.cache_root = rex::filesystem::GetExecutableFolder() / "cache";
  }

  // Runs after the settings file is read, before the game starts.
  void OnPostInitLogging() override {
    using rex::cvar::Source;
    // Defaults that only apply when the settings file doesn't say otherwise.
    if (rex::cvar::GetFlagSource("gpu_plugin") == Source::kDefault)
      rex::cvar::SetFlagByName("gpu_plugin", "xenos");
    // Started from RecompLauncher's Big Picture mode: always full screen.
    if (const char* mode = std::getenv("RECOMPLAUNCHER_MODE"); mode && std::string(mode) == "bigpicture")
      rex::cvar::SetFlagByName("fullscreen", "true");
  }

  // Segoe UI for the setup screen (the debug overlays keep the SDK's font).
  void OnConfigureFonts(ImFontAtlas* atlas) override { adventure_pants::LoadSetupFonts(atlas); }

  // A package or folder dropped on the window during setup.
  void OnFileDrop(rex::ui::FileDropEvent& e) override {
    if (setup_dialog_ && !setup_dialog_->busy()) {
      auto p = e.filename().u8string();
      setup_dialog_->Start(std::string(p.begin(), p.end()));
    }
  }

  std::optional<rex::PathConfig> OnFinalizePaths(
      const rex::PathConfig& defaults, std::function<void(rex::PathConfig)> resume) override;

  void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {
    fps_overlay_ = std::make_unique<adventure_pants::FpsOverlay>(drawer);
    SetGuestFrameStats([] { return adventure_pants::GetFrameStats(); });
    // Escape switches between full screen and a window.
    rex::ui::RegisterBind("bind_toggle_window", "Escape", "Switch between fullscreen and windowed",
                          [this] {
                            if (auto* w = window()) w->SetFullscreen(!w->IsFullscreen());
                          });
  }

  void OnShutdown() override {
    rex::ui::UnregisterBind("bind_toggle_window");
    setup_dialog_.reset();
    fps_overlay_.reset();
  }

 private:
  std::unique_ptr<adventure_pants::FpsOverlay> fps_overlay_;
  std::unique_ptr<adventure_pants::SetupDialog> setup_dialog_;
};
