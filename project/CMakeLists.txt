# Adventure Pants Recomp - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake. The CMake target keeps its
# original name (FancyPants) because the generated code is named after it; the
# exe is AdventurePantsRecomp.exe.

cmake_minimum_required(VERSION 3.25)
project(FancyPants LANGUAGES CXX)
if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

set(FANCYPANTS_SOURCES
    src/main.cpp
    src/adventure_pants_app.cpp
    src/setup.cpp
    src/stfs.cpp
    src/fps_counter.cpp
    src/platform_win.cpp
)

if(WIN32)
    add_executable(FancyPants WIN32 ${FANCYPANTS_SOURCES} res/app.rc)
    target_link_libraries(FancyPants PRIVATE ole32 shell32 uuid)
else()
    add_executable(FancyPants ${FANCYPANTS_SOURCES})
endif()

set_target_properties(FancyPants PROPERTIES OUTPUT_NAME AdventurePantsRecomp)

rexglue_setup_target(FancyPants)
