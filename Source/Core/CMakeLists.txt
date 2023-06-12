cmake_minimum_required(VERSION 3.12)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(QEngineCore STATIC ${PROJECT_SOURCE})

set_property(TARGET QEngineCore PROPERTY AUTOMOC ON)

target_include_directories(QEngineCore PUBLIC Public) 
target_include_directories(QEngineCore PRIVATE 
    Private 
    Public/Asset 
    Public/Render 
    Public/Render/Painter 
    Public/Render/Pass 
    Public/Render/RHI 
    Public/Type 
    Public/Utils
    ../../3rdParty/fftw3/api
) 

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

target_link_libraries(QEngineCore PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::ShaderToolsPrivate
    Qt::Multimedia
    Qt::MultimediaPrivate
    assimp::assimp
    fftw3
    ImGui
    ImGuizmo
)

set_target_properties(QEngineCore PROPERTIES FOLDER "QEngineUtilities")