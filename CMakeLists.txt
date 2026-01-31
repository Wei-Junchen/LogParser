cmake_minimum_required(VERSION 3.16)
project(LogParser VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Windows MSVC UTF-8 源文件编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 查找Qt6，如果没有则尝试Qt5
find_package(Qt6 COMPONENTS Widgets Charts Qml QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets Charts Qml REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/csvparser.cpp
    src/chartwidget.cpp
    src/canvaspanel.cpp
    src/presetmanager.cpp
    src/scriptengine.cpp
    src/scripteditor.cpp
    src/appsettings.cpp
    src/seriesstyledialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/csvparser.h
    src/chartwidget.h
    src/canvaspanel.h
    src/presetmanager.h
    src/scriptengine.h
    src/scripteditor.h
    src/appsettings.h
    src/seriesstyledialog.h
)

# WIN32: 在Windows上隐藏控制台窗口，仅显示GUI
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Charts Qt6::Qml)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Charts Qt5::Qml)
endif()
