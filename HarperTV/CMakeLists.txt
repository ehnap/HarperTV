cmake_minimum_required(VERSION 3.16)
project(HarperTV LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/cmake)

find_package(SDL2 CONFIG REQUIRED)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit 平台
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/x64/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/x64/release)
else()
    # 32-bit 平台
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/x86/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/x86/release)
endif()

include_directories(${SDL2_INCLUDE_DIRS})

# 设置FFmpeg路径
set(FFMPEG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg/include)
set(FFMPEG_LIB_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg/lib)
set(FFMPEG_DLL_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg/bin)

# 包含FFmpeg头文件路径
include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(${FFMPEG_LIB_DIRS})

set(PROJECT_SOURCES
    src/main.cpp
    src/harpertv.h
    src/harpertv.cpp
    src/HarperTV.rc
    src/playwidget.h
    src/playwidget.cpp
    src/decodethread.h
    src/decodethread.cpp
)

qt_add_executable(HarperTV ${PROJECT_SOURCES})

set_target_properties(HarperTV
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(HarperTV
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
    PRIVATE
        SDL2::SDL2
        avcodec
        avformat 
        avutil 
        swscale
)

add_custom_command(TARGET HarperTV POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIBDIR}/SDL2.dll"
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
    COMMENT "Copying SDL2.dll to debug directory"
)

# 将FFmpeg的DLL文件复制到可执行文件目录
add_custom_command(TARGET HarperTV POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${FFMPEG_DLL_DIRS}/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

# 创建一个自定义目标，用于在构建时执行自定义命令
add_custom_target(CopySDL2Dll ALL DEPENDS HarperTV)
