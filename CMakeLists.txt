cmake_minimum_required(VERSION 3.12)
project(htrack VERSION 2209.1.0 LANGUAGES C CXX)

include(cmake/xplm.cmake)

add_definitions(
    "-DHTK_VERSION=\"${PROJECT_VERSION}\""
    "-DHTK_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}"
    "-DHTK_VERSION_MINOR=${PROJECT_VERSION_MINOR}"
    "-DHTK_VERSION_PATCH=${PROJECT_VERSION_PATCH}"
)

include(cmake/xplm.cmake)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

option(LIBACFUTILS "Directory where libacfutils is installed" "")

find_xplane_sdk("${LIBACFUTILS}/SDK" 301)
find_acfutils(${LIBACFUTILS})

add_xplane_plugin(htrack
    
    src/main.c
    src/htrack.c
    src/paths.c
    src/saving.c
    src/server.c
    src/settings.cpp
    
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/misc/cpp/imgui_stdlib.cpp
    lib/ImgWindow/ImgFontAtlas.cpp
    lib/ImgWindow/ImgWindow.cpp
)
target_compile_features(htrack PUBLIC c_std_11 cxx_std_17)
target_compile_options(htrack PUBLIC -Wall -Wextra -fno-exceptions)

target_include_directories(htrack PRIVATE "lib")
target_link_libraries(htrack PUBLIC m acfutils xplm ${OPENGL_LIBRARIES})
