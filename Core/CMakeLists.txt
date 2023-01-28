cmake_minimum_required(VERSION 3.12)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets ShaderTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets ShaderTools REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
add_definitions(-DVK_NO_PROTOTYPES)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp)

add_library(QEngineCore STATIC ${PROJECT_SOURCE})

set_property(TARGET QEngineCore PROPERTY AUTOMOC ON)

target_include_directories(QEngineCore PUBLIC ./Src/)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

add_subdirectory(3rdParty)

target_link_libraries(QEngineCore PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::ShaderToolsPrivate
    assimp::assimp
    ImGui
    ImGuizmo
)

set_target_properties(QEngineCore PROPERTIES FOLDER "QEngineUtilties")