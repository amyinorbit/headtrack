# Import acfutils libraries and create an imported target
function(find_acfutils dir)
    if(WIN32)
        set(ARCH "mingw64")
    elseif(APPLE)
        set(ARCH "mac64")
    else()
        set(ARCH "lin64")
    endif()
    
    set(ACFLIB "${dir}/libacfutils-redist/${ARCH}/lib/libacfutils.a")
    
    set(ACFLIBROOT "${dir}/libacfutils-redist/${ARCH}/lib")
    set(ACFLIBS
        "${ACFLIBROOT}/libiconv.a"
        "${ACFLIBROOT}/libcairo.a"
        "${ACFLIBROOT}/libpixman-1.a"
        "${ACFLIBROOT}/libfreetype.a"
        "${ACFLIBROOT}/libopusurl.a"
        "${ACFLIBROOT}/libopusfile.a"
        "${ACFLIBROOT}/libopus.a"
        "${ACFLIBROOT}/libogg.a"
        "${ACFLIBROOT}/libpcre2-8.a"
        "${ACFLIBROOT}/libproj.a"
        "${ACFLIBROOT}/libxml2.a"
        "${ACFLIBROOT}/libcurl.a"
        "${ACFLIBROOT}/libshp.a"
        "${ACFLIBROOT}/libcharset.a"
        "${ACFLIBROOT}/libpng16.a"
        "${ACFLIBROOT}/liblzma.a"
        "${ACFLIBROOT}/libz.a"
        "${ACFLIBROOT}/libssl.a"
        "${ACFLIBROOT}/libcrypto.a"
    )
    
    if(WIN32)
        list(APPEND ACFLIBS "${ACFLIBROOT}/libglew32mx.a")
    else()
        list(APPEND ACFLIBS "${ACFLIBROOT}/libGLEWmx.a")
    endif()
    
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
    
    add_library(acfutils STATIC IMPORTED GLOBAL)
    target_include_directories(acfutils INTERFACE
        "${dir}/libacfutils-redist/include"
        "${dir}/libacfutils-redist/${ARCH}/include"
        "${dir}/src/jsmn")
    set_target_properties(acfutils PROPERTIES IMPORTED_LOCATION "${ACFLIB}")
    
    
    if(APPLE)
        target_compile_definitions(acfutils INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
    elseif(WIN32)
        target_compile_definitions(acfutils INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
    else()
        target_compile_definitions(acfutils INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
    endif()
    target_link_libraries(acfutils INTERFACE ${ACFLIBS} ${OPENGL_LIBRARIES})
    
endfunction(find_acfutils)

# Import the X-Plane SDK and create targets for it
function(find_xplane_sdk SDK_ROOT SDK_VERSION)
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
    
    if(WIN32)
        add_library(xplm SHARED IMPORTED GLOBAL)
        add_library(xpwidgets SHARED IMPORTED GLOBAL)
        
        set_target_properties(xplm PROPERTIES IMPORTED_IMPLIB "${XPLM_LIBRARY}")
        set_target_properties(xpwidgets PROPERTIES IMPORTED_IMPLIB "${XPWIDGETS_LIBRARY}")
        
        target_compile_definitions(xplm INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=0 -DIBM=1 -DLIN=0)
    elseif(APPLE)
        add_library(xplm SHARED IMPORTED GLOBAL)
        add_library(xpwidgets SHARED IMPORTED GLOBAL)
        
        set_target_properties(xplm PROPERTIES IMPORTED_LOCATION "${XPLM_LIBRARY}/XPLM")
        set_target_properties(xpwidgets PROPERTIES IMPORTED_LOCATION "${XPWIDGETS_LIBRARY}/XPWidgets")
        
        target_compile_definitions(xplm INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=1 -DIBM=0 -DLIN=0)
    else()
        add_library(xplm INTERFACE)
        add_library(xpwidgets INTERFACE)
        
        target_compile_definitions(xplm INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
        target_compile_definitions(xpwidgets INTERFACE -DAPL=0 -DIBM=0 -DLIN=1)
    endif()
    
    if(SDK_VERSION EQUAL 300)
        target_compile_definitions(xplm
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1)
        target_compile_definitions(xpwidgets
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
    elseif(SDK_VERSION EQUAL 301)
        target_compile_definitions(xplm
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
        target_compile_definitions(xpwidgets
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
    elseif(SDK_VERSION EQUAL 400)
        target_compile_definitions(xplm
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1)
        target_compile_definitions(xpwidgets
            INTERFACE -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1)
    else()
        message(FATAL_ERROR "Library version one of: 300, 301, 400")
    endif()
    
    target_include_directories(xplm INTERFACE "${SDK_ROOT}/CHeaders/XPLM")
    target_include_directories(xpwidgets INTERFACE
        "${SDK_ROOT}/CHeaders/XPLM" "${SDK_ROOT}/CHeaders/Widgets")
    
    set(XPLM_INCLUDE_DIR
        "${SDK_ROOT}/CHeaders/XPLM"
        "${SDK_ROOT}/CHeaders/Widgets"
        "${SDK_ROOT}/CHeaders/Wrappers"
        CACHE INTERNAL "XPLM SDK INCLUDE DIRECTORIES"
    )
endfunction(find_xplane_sdk)

function(add_xplane_plugin lib_name ...)
    set(SRC ${ARGV})
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
        target_link_libraries(${lib_name} PUBLIC ${CocoaLib})
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/mac_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/mac_x64")
    elseif(WIN32)
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=1 -DLIN=0)
        target_compile_definitions(${lib_name} PUBLIC -D_WIN32_WINNT=0x0600)
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/win_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/win_x64"
            CMAKE_C_FLAGS
                "${CMAKE_C_FLAGS} -static-libgcc"
            CMAKE_CXX_FLAGS
                "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++"
            CMAKE_SHARED_LIBRARY_LINK_C_FLAGS
                "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s"
            CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
                "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s"
        )
        
    else()
        target_compile_definitions(${lib_name} PUBLIC -DAPL=0 -DIBM=0 -DLIN=1)
    	set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/lin_x64"
            RUNTIME_OUTPUT_DIRECTORY
                "${PROJECT_SOURCE_DIR}/${lib_name}/lin_x64")
    endif()
    
    target_link_libraries(${lib_name} PUBLIC xplm xpwidgets)
    
    set_target_properties(${lib_name} PROPERTIES
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )
    set_target_properties(${lib_name} PROPERTIES LINK_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector")
    
endfunction(add_xplane_plugin)

