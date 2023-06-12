file(GLOB_RECURSE PROJECT_SOURCE FILES *.h *.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

add_library(QEngineUtilities STATIC 
    ${PROJECT_SOURCE}
)

target_link_libraries(QEngineUtilities PUBLIC QEngineCore)

target_link_libraries(QEngineUtilities PRIVATE 
    $<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QEngineEditor>
)

target_compile_definitions(QEngineUtilities PUBLIC "$<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QENGINE_WITH_EDITOR>")

target_include_directories(QEngineUtilities PUBLIC Public) 
target_include_directories(QEngineUtilities PRIVATE Private) 

set_target_properties(QEngineUtilities PROPERTIES FOLDER "QEngineUtilities")