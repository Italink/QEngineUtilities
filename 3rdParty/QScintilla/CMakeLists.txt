cmake_minimum_required(VERSION 3.5)

project(QScintilla VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES FILES *.h *.cpp *.hpp)

add_library(QScintilla STATIC ${PROJECT_SOURCES})

target_compile_definitions(QScintilla PUBLIC SCINTILLA_QT SCI_LEXER INCLUDE_DEPRECATED_FEATURES)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES})

target_include_directories(QScintilla PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/lexlib ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include/Qsci)

target_include_directories(QScintilla PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(QScintilla PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
