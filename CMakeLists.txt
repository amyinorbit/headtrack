cmake_minimum_required(VERSION 3.12)
project(htrack VERSION 2011.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(ccore REQUIRED)

if(APPLE)
    link_directories(/usr/local/lib)
endif()

option(XPLANE_DIR "X-Plane's root directory" "/xp11")

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(PLUGIN_SRC
    src/main.c
    src/htrack.c
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRC})
include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/XPLM"
    "SDK/CHeaders/Widgets"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/Wrappers"
)

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -pedantic -Werror)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
    add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
elseif(APPLE)
    add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
else()
    add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()

target_link_libraries(${PROJECT_NAME} m ccore)

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
