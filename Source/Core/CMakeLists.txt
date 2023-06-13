qengine_add_target(QEngineCore
    ENABLE_MOC
    PUBLIC_DEPENDENCY   
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
    PUBLIC_INCLUDE
        ../../3rdParty/fftw3/api
    SOURCE_GROUP "QEngineUtilities"
)
