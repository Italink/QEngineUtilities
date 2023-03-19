file(GLOB_RECURSE PROJECT_SOURCE FILES Src/*.h Src/*.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Src FILES ${PROJECT_SOURCE})

qt6_add_resources(QRC_FILES Resources.qrc)

add_library(QEngineEditor STATIC 
    ${PROJECT_SOURCE}
    ${QRC_FILES}
)

target_link_libraries(QEngineEditor PUBLIC 
    QEngineCore
)

target_include_directories(QEngineEditor PUBLIC
    Src/Public
) 
target_include_directories(QEngineEditor PRIVATE
    Src/Private
) 
set_target_properties(QEngineEditor PROPERTIES FOLDER "QEngineUtilities")