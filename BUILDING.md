# Building Adventure Pants Recomp

Windows 10 or 11. You need your own copy of the game: nothing from it is stored here.

1. Download the [ReXGlue SDK 0.10.1](https://github.com/rexglue/rexglue-sdk/releases/tag/v0.10.1) for Windows (`win-amd64`) and unzip it to your Desktop as `rexglue-sdk-0.10.1-win-amd64`.
2. Put your Xbox Live Arcade dump on your Desktop as `Fancy Pants Adventures, The (World) (XBLA)`, or change the `SDK` and `PKG` lines in `tools\env.bat`.
3. Run these in order:

| Script | Does |
|---|---|
| `1_install_tools.bat` | Installs LLVM (clang), CMake, Ninja, Python and the Visual Studio 2022 C++ tools. Once. |
| `2_recompile.bat` | Unpacks your copy and recompiles `default.xex` to C++ with ReXGlue. |
| `3_build.bat` | Builds `AdventurePantsRecomp.exe` into the `AdventurePantsRecomp` folder. |
| `4_package.bat` | Makes the release zip and the source zip in `release\`. |

After changing the app code (`project/src`), run `3_build.bat`. After changing the function list (`project/fancypants_functions.toml`), run `REBUILD.bat`.

## What's where
- `project/src`: the app. The setup screen, the package unpacker, the frame counter and the RecompLauncher setup step.
- `project/fancypants_functions.toml`: function boundaries the recompiler needs help with. `tools/find_missing_functions.py` finds more of them from the generated code.
- `project/recomplauncher.json`: how RecompLauncher installs, sets up and shows settings for the game.
- `tools/stfs_extract.py`: the same unpacker as the exe's, for building.
