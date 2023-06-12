file(GLOB_RECURSE PROJECT_SOURCE FILES *.h *.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

qt6_add_resources(QRC_FILES Resources.qrc)

add_library(QEngineEditor STATIC 
    ${PROJECT_SOURCE}
    ${QRC_FILES}
)

target_link_libraries(QEngineEditor PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::Multimedia
    QScintilla
)

target_include_directories(QEngineEditor PUBLIC Public) 
target_include_directories(QEngineEditor PRIVATE Private) 

set_target_properties(QEngineEditor PROPERTIES FOLDER "QEngineUtilities")