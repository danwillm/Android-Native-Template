cmake_minimum_required(VERSION 3.22.1)

project(mynativeapp)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_ANDROID_NDK)
    file(STRINGS "${CMAKE_ANDROID_NDK}/source.properties" NDK_PROPERTIES)
    foreach(_line ${NDK_PROPERTIES})
        if("${_line}" MATCHES
                "Pkg.Revision = ([0-9]+)[.]([0-9]+)[.]([0-9]+)"
        )
            set(NDK_MAJOR_VERSION ${CMAKE_MATCH_1})
        endif()
    endforeach()
else()
    message(FATAL_ERROR "Please set CMAKE_ANDROID_NDK to your NDK root!")
endif()

if(NDK_MAJOR_VERSION)
    message(STATUS "Building using NDK major version ${NDK_MAJOR_VERSION}")
else()
    message(
            FATAL_ERROR
            "Could not parse the major version from ${CMAKE_ANDROID_NDK}/source.properties"
    )
endif()

find_path(
        ANDROID_NATIVE_APP_GLUE android_native_app_glue.h
        PATHS "${ANDROID_NDK}/sources/android/native_app_glue"
)

add_library(
        android_native_app_glue OBJECT
        "${ANDROID_NATIVE_APP_GLUE}/android_native_app_glue.c"
)
target_include_directories(
        android_native_app_glue PUBLIC "${ANDROID_NATIVE_APP_GLUE}"
)
target_compile_options(
        android_native_app_glue PRIVATE -Wno-unused-parameter
)

find_library(ANDROID_LIBRARY NAMES android)
find_library(ANDROID_LOG_LIBRARY NAMES log)

if(ANDROID_PLATFORM_LEVEL LESS 24)
    message( FATAL_ERROR "Vulkan disabled due to incompatibility: need to target at least API 24" )
endif()

add_library( mynativeapp SHARED src/main.cpp src/gles.cpp src/log.cpp $<TARGET_OBJECTS:android_native_app_glue> )

target_link_libraries( mynativeapp PRIVATE ${ANDROID_LIBRARY} ${ANDROID_LOG_LIBRARY} EGL GLESv3 )

target_include_directories( mynativeapp PUBLIC "${ANDROID_NATIVE_APP_GLUE}")