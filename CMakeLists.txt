cmake_minimum_required(VERSION 3.12)
project(htrack VERSION 2209.1.2 LANGUAGES C CXX)

include(cmake/xplm.cmake)

add_definitions(
    "-DHTK_VERSION=\"${PROJECT_VERSION}\""
    "-DHTK_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}"
    "-DHTK_VERSION_MINOR=${PROJECT_VERSION_MINOR}"
    "-DHTK_VERSION_PATCH=${PROJECT_VERSION_PATCH}"
)

if(APPLE)
	add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)

    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")
elseif(WIN32)
	add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
else()
	add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()

option(LIBACFUTILS "Directory where libacfutils is installed" "")
find_package(OpenGL REQUIRED)

use_static_libc()
find_acfutils(${LIBACFUTILS})
find_xplane_sdk("${LIBACFUTILS}/SDK" 301)


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

message(${OPENGL_LIBRARIES})

target_compile_features(htrack PUBLIC c_std_11 cxx_std_17)
target_compile_options(htrack PUBLIC -Wall -Wextra -fno-exceptions)
target_link_libraries(htrack PUBLIC m acfutils xplm xpwidgets)
target_include_directories(htrack PRIVATE "lib")



# find_xplane_sdk("${LIBACFUTILS}/SDK" 301)
# find_acfutils(${LIBACFUTILS})
#
# message(${LIBACFUTILS_LIBS})
#
# add_xplane_plugin(htrack
#
#     src/main.c
#     src/htrack.c
#     src/paths.c
#     src/saving.c
#     src/server.c
#     src/settings.cpp
#
#     lib/imgui/imgui.cpp
#     lib/imgui/imgui_draw.cpp
#     lib/imgui/imgui_widgets.cpp
#     lib/imgui/misc/cpp/imgui_stdlib.cpp
#     lib/ImgWindow/ImgFontAtlas.cpp
#     lib/ImgWindow/ImgWindow.cpp
# )
# target_compile_features(htrack PUBLIC c_std_11 cxx_std_17)
# target_compile_options(htrack PUBLIC -Wall -Wextra -fno-exceptions)
#
# target_include_directories(htrack PRIVATE "lib")
# target_link_libraries(htrack PUBLIC m xplm acfutils)
