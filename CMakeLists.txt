cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
if (IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(Controllable VERSION 1.0.0)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DREMOVE_GJITEMICON)
endif()

if (WIN32)
    file(GLOB_RECURSE WIN_SOURCES src/platform/windows/*.cpp)
    list(APPEND SOURCES ${WIN_SOURCES})
elseif (APPLE)
    file(GLOB_RECURSE MAC_SOURCES 
        src/platform/macos/*.cpp
        src/platform/macos/*.mm
    )
    list(APPEND SOURCES ${MAC_SOURCES})
elseif (ANDROID)
    file(GLOB_RECURSE ANDROID_SOURCES src/platform/android/*.cpp)
    list(APPEND SOURCES ${ANDROID_SOURCES})
endif()

file(GLOB COMMON_SOURCES src/*.cpp)
file(GLOB_RECURSE HOOK_SOURCES src/hooks/*.cpp)
list(APPEND SOURCES ${COMMON_SOURCES})
list(APPEND SOURCES ${HOOK_SOURCES})

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE lib/)

if (PROJECT_IS_TOP_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE CONTROLLABLE_EXPORTING)
endif()

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# gh:matcool/gd-render-texture fork
CPMAddPackage("gh:undefined06855/gd-render-texture#9f51bd1")
target_link_libraries(${PROJECT_NAME} gd-render-texture)

CPMAddPackage("gh:undefined06855/node-identification#30944b4")
target_link_libraries(${PROJECT_NAME} node-identification)

setup_geode_mod(${PROJECT_NAME})
