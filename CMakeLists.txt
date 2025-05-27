cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(Controllable VERSION 1.0.0)


file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/hooks/*.cpp
    src/hooks/compat/*.cpp
)
file(GLOB_RECURSE WINSOURCES
    src/platform/windows/*.cpp
    src/platform/windows/hooks/*.cpp
)
file(GLOB_RECURSE MACSOURCES
    src/platform/macos/*.cpp
    src/platform/macos/hooks/*.cpp
)

if (WIN32)
    list(APPEND SOURCES ${WINSOURCES})
elseif (APPLE)
    list(APPEND SOURCES ${MACSOURCES})
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})


if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# gh:matcool/gd-render-texture fork
CPMAddPackage("gh:undefined06855/gd-render-texture#9f51bd1")
target_link_libraries(${PROJECT_NAME} gd-render-texture)

setup_geode_mod(${PROJECT_NAME})
