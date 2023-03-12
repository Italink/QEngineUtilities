cmake_minimum_required(VERSION 3.12)

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
    Qt::Multimedia
    Qt::MultimediaPrivate
    assimp::assimp
    ImGui
    ImGuizmo
)

set_target_properties(QEngineCore PROPERTIES FOLDER "QEngineUtilities")