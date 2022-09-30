
function(find_acfutils dir)
    if(WIN32)
        set(ARCH "mingw32")
    elseif(APPLE)
        set(ARCH "mac64")
    else()
        set(ARCH "lin64")
    endif()
    
    
    file(GLOB ACFLIBS ${dir}/libacfutils-redist/${ARCH}/lib/*.a)
    if(WIN32)
        list(APPEND ACFLIBS crypt32 ws2_32 gdi32 dbghelp psapi winmm)
    else()
        list(APPEND ACFLIBS pthread)
    endif()

    set(LIBACFUTILS_INCLUDE_DIR
        "${dir}/libacfutils-redist/include"
        "${dir}/libacfutils-redist/${ARCH}/include"
        "${dir}/src/jsmn"
        CACHE INTERNAL "acfutils include dir"
    )
    set(LIBACFUTILS_LIBS ${ACFLIBS} CACHE INTERNAL "acfutils libs")
endfunction(find_acfutils)

function(find_xplane_sdk SDK_ROOT)
    if(APPLE)
        find_library(XPLM_LIBRARY
            NAMES XPLM
            PATHS ${SDK_ROOT}/Libraries/Mac/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(XPWIDGETS_LIBRARY
            XPWidgets
            ${SDK_ROOT}/Libraries/Mac/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(CocoaLib Cocoa)
    elseif(WIN32)
        find_library(XPLM_LIBRARY
            NAMES XPLM_64
            PATHS ${SDK_ROOT}/Libraries/Win/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(XPWIDGETS_LIBRARY
            NAMES XPWidgets_64
            PATHS ${SDK_ROOT}/Libraries/Win/
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )
    endif()
    
    message("${XPLM_LIBRARY}")
    
    set(XPLM_INCLUDE_DIR
        "${SDK_ROOT}/CHeaders/XPLM"
        "${SDK_ROOT}/CHeaders/Widgets"
        "${SDK_ROOT}/CHeaders/Wrappers"
        CACHE INTERNAL "XPLM SDK INCLUDE DIRECTORIES"
    )
endfunction(find_xplane_sdk)

function(add_xplane_sdk_definitions library_name library_version)

    if(library_version EQUAL 300)
        target_compile_definitions(${library_name}
            PUBLIC -DXPLM200=1 -DXPLM210=1 -DXPLM300=1)
    elseif(library_version EQUAL 301)
        target_compile_definitions(${library_name}
            PUBLIC -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
    elseif(library_version EQUAL 400)
        target_compile_definitions(${library_name}
            PUBLIC -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1)
    else()
        message(FATAL_ERROR "Library version one of: 300, 301, 400")
    endif()
endfunction(add_xplane_sdk_definitions)

function(add_xplane_plugin lib_name lib_version ...)
    set(SRC ${ARGV})
    list(REMOVE_AT SRC 0)
    list(REMOVE_AT SRC 0)
    
    add_library(${lib_name} SHARED ${SRC})
    set_target_properties(${lib_name} PROPERTIES PREFIX "")
    set_target_properties(${lib_name} PROPERTIES SUFFIX "")
    set_target_properties(${lib_name} PROPERTIES OUTPUT_NAME "${lib_name}.xpl")
    include_directories(${lib_name} PUBLIC ${XPLM_INCLUDE_DIR})
    
    # Link libraries
    if(APPLE)
        target_compile_definitions(${lib_name} PUBLIC -DAPL=1 -DIBM=0 -DLIN=0)
        target_compile_definitions(${lib_name} PUBLIC -DGL_SILENCE_DEPRECATION)
        target_link_libraries(${lib_name} PUBLIC ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY} ${CocoaLib})
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${lib_name}/mac_x64" )
    elseif(WIN32)
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=1 -DLIN=0)
        target_compile_definitions(${lib_name} PUBLIC -D_WIN32_WINNT=0x0600)
        target_link_libraries(${lib_name} PUBLIC ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/win_x64"
            CMAKE_C_FLAGS
                "${CMAKE_C_FLAGS} -static-libgcc"
            CMAKE_CXX_FLAGS
                "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++"
            CMAKE_SHARED_LIBRARY_LINK_C_FLAGS
                "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s"
            CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
                "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s"
        )
    else()
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=0 -DLIN=1)
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${lib_name}/lin_x64" )
    endif()
    add_xplane_sdk_definitions(${lib_name} ${lib_version})
    
    set_target_properties(${lib_name} PROPERTIES LINK_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector -fPIC -fvisibility=hidden")
    
endfunction(add_xplane_plugin)

