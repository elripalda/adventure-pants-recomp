// Adventure Pants Recomp - app

#include "generated/default/FancyPants_init.h"

#include "adventure_pants_app.h"

#include <rex/ui/windowed_app_context.h>

REXCVAR_DEFINE_BOOL(prepare, false, "AdventurePants",
                    "Unpack the game from --game and exit (used by RecompLauncher's setup)");
REXCVAR_DEFINE_STRING(game, "", "AdventurePants",
                      "Your copy of the game: its Xbox Live Arcade package or the folder with it");

std::unique_ptr<rex::ui::WindowedApp> AdventurePantsApp::Create(rex::ui::WindowedAppContext& ctx) {
  // Headless setup step: no window, just unpack and report progress.
  if (REXCVAR_GET(prepare)) {
    std::exit(adventure_pants::RunPrepare(REXCVAR_GET(game)));
  }
  return std::unique_ptr<AdventurePantsApp>(
      new AdventurePantsApp(ctx, "AdventurePantsRecomp", PPCImageConfig));
}

std::optional<rex::PathConfig> AdventurePantsApp::OnFinalizePaths(
    const rex::PathConfig& defaults, std::function<void(rex::PathConfig)> resume) {
  if (auto* w = window()) w->SetTitle(adventure_pants::kTitle);

  rex::PathConfig paths = defaults;
  if (adventure_pants::GameInstalled()) {
    paths.game_data_root = adventure_pants::GameDir();
    return paths;
  }

  // First start: show the setup screen in a window, then carry on once the game is unpacked.
  bool want_fullscreen = window() && window()->IsFullscreen();
  if (want_fullscreen) window()->SetFullscreen(false);

  auto post = [this](std::function<void()> fn) { app_context().CallInUIThreadDeferred(std::move(fn)); };
  auto on_ready = [this, paths, resume, want_fullscreen]() mutable {
    paths.game_data_root = adventure_pants::GameDir();
    if (want_fullscreen && window()) window()->SetFullscreen(true);
    // Drop the setup screen on the next UI tick, after it has finished drawing.
    app_context().CallInUIThreadDeferred([this] { setup_dialog_.reset(); });
    resume(paths);
  };
  auto on_quit = [this] { app_context().QuitFromUIThread(); };

  setup_dialog_ = std::make_unique<adventure_pants::SetupDialog>(
      imgui_drawer(), immediate_drawer(), post, on_ready, on_quit);
  std::string from_launcher = REXCVAR_GET(game);
  if (!from_launcher.empty()) setup_dialog_->Start(from_launcher);
  return std::nullopt;
}
