cmake_minimum_required(VERSION 3.20)

set(COMPANY "Au-Zone Technologies")
set(EMAIL "support@au-zone.com")
set(COPYRIGHT "Copyright 2023 Au-Zone Technologies.  All Rights Reserved.")

project(vpkui C ASM CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Version)
set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
add_definitions(-DVERSION=\"${PROJECT_VERSION}${VERSION_EXTRA}\")

option(ENABLE_VIVANTE "Enables Vivante OpenGL Extensions" ON)

if(ENABLE_VIVANTE)
    add_definitions(-DENABLE_VIVANTE)
endif()

include(CTest)
include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakeToolsHelpers OPTIONAL)

find_package(Threads)
find_package(VideoStream REQUIRED)
find_package(VAAL REQUIRED)
find_package(DeepViewRT REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLES REQUIRED IMPORTED_TARGET glesv2)
pkg_check_modules(Wayland REQUIRED IMPORTED_TARGET wayland-egl xkbcommon)
# Required for headless
pkg_check_modules(GBM REQUIRED IMPORTED_TARGET gbm)
pkg_check_modules(DRM REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(Soup REQUIRED IMPORTED_TARGET libsoup-2.4)
pkg_check_modules(GLibc REQUIRED IMPORTED_TARGET glib-2.0)

include_directories(include)
include_directories(ext/include)
include_directories(/src/libraries/assets/include)
include_directories(/src/libraries/models/include)
include_directories(/src/libraries/lib/include)
include_directories(/src/libraries/headless/include)

ADD_LIBRARY(assets STATIC IMPORTED)
SET_TARGET_PROPERTIES(assets PROPERTIES
    IMPORTED_LOCATON "/src/libraries/assets/libassets.a"
    IMPORTED_LOCATION_RELEASE "/src/libraries/assets/libassets.a"
    IMPORTED_LOCATION_DEBUG "/src/libraries/assets/libassets.a"
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
# SET_PROPERTY(TARGET assets PROPERTY
#              INTERFACE_LINK_LIBRARIES "/src/libraries/assets/calibrib.S.o" 
#                                       "/src/libraries/assets/overlay.S.o")
TARGET_INCLUDE_DIRECTORIES(assets INTERFACE "/src/libraries/assets/include")

ADD_LIBRARY(models STATIC IMPORTED)
SET_TARGET_PROPERTIES(models PROPERTIES
    IMPORTED_LOCATON "/src/libraries/models/libmodels.a"
    IMPORTED_LOCATION_RELEASE "/src/libraries/models/libmodels.a"
    IMPORTED_LOCATION_DEBUG "/src/libraries/models/libmodels.a"
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
# SET_PROPERTY(TARGET models PROPERTY
#              INTERFACE_LINK_LIBRARIES "/src/libraries/models/bodypose.S.o"
#                                       "/src/libraries/models/facedetect.S.o"
#                                       "/src/libraries/models/headpose.S.o"
#                                       "/src/libraries/models/peopledetect.S.o"
#                                       "/src/libraries/models/playingcards.S.o"
#                                       "/src/libraries/models/safetygear.S.o"
#                                       "/src/libraries/models/segmentation.S.o")
    TARGET_INCLUDE_DIRECTORIES(models INTERFACE "/src/libraries/models/include")

ADD_LIBRARY(vpkui STATIC IMPORTED)
SET_TARGET_PROPERTIES(vpkui PROPERTIES
    IMPORTED_LOCATON "/src/libraries/lib/libvpkui.a"
    IMPORTED_LOCATION_RELEASE "/src/libraries/lib/libvpkui.a"
    IMPORTED_LOCATION_DEBUG "/src/libraries/lib/libvpkui.a"
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
# SET_PROPERTY(TARGET vpkui PROPERTY
#              INTERFACE_LINK_LIBRARIES "/src/libraries/lib/opengl.c.o"
#                                       "/src/libraries/lib/opengl_font.c.o"
#                                       "/src/libraries/lib/shared.c.o"
#                                       "/src/libraries/lib/vpkui.c.o")
TARGET_INCLUDE_DIRECTORIES(vpkui INTERFACE "/src/libraries/lib/include" "/src/libraries/headless/include")
TARGET_LINK_LIBRARIES(vpkui INTERFACE assets)

ADD_LIBRARY(vpkui-headless STATIC IMPORTED)
SET_TARGET_PROPERTIES(vpkui-headless PROPERTIES
    IMPORTED_LOCATON "/src/libraries/headless/libvpkui-headless.a"
    IMPORTED_LOCATION_RELEASE "/src/libraries/headless/libvpkui-headless.a"
    IMPORTED_LOCATION_DEBUG "/src/libraries/headless/libvpkui-headless.a"
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
# SET_PROPERTY(TARGET vpkui-headless PROPERTY
#              INTERFACE_LINK_LIBRARIES "/src/libraries/headless/init_window_gbm.c.o"
#                                       "/src/libraries/headless/streaming.cpp.o"
#                                       "/src/libraries/headless/streaming_wrapper.cpp.o"
#                                       "/src/libraries/headless/streamingserver.cpp.o")
target_link_libraries(vpkui-headless INTERFACE videostream PkgConfig::DRM PkgConfig::GBM PkgConfig::Soup dl Threads::Threads)
TARGET_INCLUDE_DIRECTORIES(vpkui-headless INTERFACE "/src/libraries/headless/include")

ADD_LIBRARY(vpkui-wayland STATIC IMPORTED)
SET_TARGET_PROPERTIES(vpkui-wayland PROPERTIES
    IMPORTED_LOCATON "/src/libraries/wayland/libvpkui-wayland.a"
    IMPORTED_LOCATION_RELEASE "/src/libraries/wayland/libvpkui-wayland.a"
    IMPORTED_LOCATION_DEBUG "/src/libraries/wayland/libvpkui-wayland.a"
    IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    # INTERFACE_LINK_LIBRARIES "/src/libraries/wayland/init_window_wl.c.o")

add_definitions(-DEMBED)

project(vpkui)

find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    OUTPUT_VARIABLE GIT_LATEST_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Define a CMake target to set the version as a preprocessor macro
add_compile_definitions(GIT_LATEST_TAG="${GIT_LATEST_TAG}")

add_executable(facedetectgl
    main.c)
target_link_libraries(facedetectgl
    Threads::Threads
    PkgConfig::GLES
    PkgConfig::Wayland
    vpkui
    vpkui-wayland
    models
    DeepView::RT
    vaal
    videostream)

add_executable(facedetectgl_headless
    main.c)
target_link_libraries(facedetectgl_headless
    Threads::Threads
    PkgConfig::GLES
    vpkui
    vpkui-headless
    models
    DeepView::RT
    vaal
    videostream)
target_compile_definitions(facedetectgl_headless PRIVATE HEADLESS)