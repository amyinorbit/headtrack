cmake_minimum_required(VERSION 3.12)
project(htrack VERSION 2012.3.2 LANGUAGES C CXX)

add_subdirectory(lib/ccore)
if(APPLE)
    link_directories(/usr/local/lib)
endif()

add_definitions(
    "-DHTK_VERSION=\"${PROJECT_VERSION}\""
    "-DHTK_VERSION_MAJOR=${PROJECT_MAJOR_VERSION}"
    "-DHTK_VERSION_MINOR=${PROJECT_MINOR_VERSION}"
    "-DHTK_VERSION_PATCH=${PROJECT_PATCH_VERSION}"
)


find_library(OPENGL_LIBRARY NAMES OpenGL)
find_package(Threads REQUIRED)
find_package(cJSON REQUIRED)
option(XPLANE_DIR "X-Plane's root directory" "/xp11")

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(PLUGIN_SRC
    src/main.c
    src/htrack.c
    src/settings.cpp
    src/saving.c
    src/paths.c
    src/server.c

    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/misc/cpp/imgui_stdlib.cpp
    lib/ImgWindow/ImgFontAtlas.cpp
    lib/ImgWindow/ImgWindow.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRC})
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11 cxx_std_17)
include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/XPLM"
    "SDK/CHeaders/Widgets"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/Wrappers"
    PRIVATE
    "lib"
)

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DGL_SILENCE_DEPRECATION)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -fno-exceptions)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32 opengl32)
    add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
elseif(APPLE)
    add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
    find_library(APPLICATION_SERVICES ApplicationServices)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} ${APPLICATION_SERVICES} ${CORE_FOUNDATION_LIBRARY} ${OpenGL_LIBRARY})
else()
    add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()

target_link_libraries(${PROJECT_NAME} m ccore cjson)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME}  PROPERTIES SUFFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.xpl")

if(APPLE)
    find_library(XPLM_LIBRARY XPLM ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Mac/)
    find_library(XPWIDGETS_LIBRARY XPWidgets ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Mac/)

    target_link_libraries(${PROJECT_NAME} ${XPLM_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${XPWIDGETS_LIBRARY})
elseif(WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/)
    find_library(XPLM_LIBRARY
        NAMES XPLM_64
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    find_library(XPWIDGETS_LIBRARY
        XPWidgets_64
        ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    target_link_libraries(${PROJECT_NAME} ${XPLM_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${XPWIDGETS_LIBRARY})
endif()

if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/win_x64" )
	set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/win_x64" )
    set(OUT_PLUGIN_PATH "Resources/plugins/${PROJECT_NAME}/win_x64")
elseif(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/mac_x64" )
	set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/mac_x64" )
    set(OUT_PLUGIN_PATH "Resources/plugins/${PROJECT_NAME}/mac_x64")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/lin_x64" )
	set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/${PROJECT_NAME}/lin_x64" )
    set(OUT_PLUGIN_PATH "Resources/plugins/${PROJECT_NAME}/lin_x64")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector -fPIC -fvisibility=hidden"
)

add_custom_target(patch
    COMMAND cp $<TARGET_FILE:${PROJECT_NAME}> ${XPLANE_DIR}/${OUT_PLUGIN_PATH}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
