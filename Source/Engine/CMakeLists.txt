qengine_add_target(QEngineUtilities
    ENABLE_MOC
    PUBLIC_DEPENDENCY   
        QEngineCore
    PRIVATE_DEPENDENCY
        $<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QEngineEditor>
    PUBLIC_DEFINITIONS
        "$<$<OR:$<CONFIG:DebugEditor>,$<CONFIG:ReleaseEditor>>:QENGINE_WITH_EDITOR>"
    SOURCE_GROUP "QEngineUtilities"
)

get_property(QENGINE_RUNTIME_PLUGIN_LIST GLOBAL PROPERTY QENGINE_RUNTIME_PLUGIN_LIST)
get_property(QENGINE_EDITOR_PLUGIN_LIST GLOBAL PROPERTY QENGINE_EDITOR_PLUGIN_LIST)

add_dependencies(QEngineUtilities 
    ${QENGINE_RUNTIME_PLUGIN_LIST} 
    ${QENGINE_EDITOR_PLUGIN_LIST} 
)