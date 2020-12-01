# https://stackoverflow.com/questions/33086122/
# Prevents from getting cmake errors when importing library from multiple deps
macro(project_guarded name)
    if(DEFINED ${name}_GUARD)
        if(NOT ${name}_GUARD STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
            return() # return() *doesn't* terminate a macro!
        endif()
    else()
        set(${name}_GUARD ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "${name} guard")
    endif()
    project(${name})
endmacro()
