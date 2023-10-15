qengine_add_target(QEngineEditor
    ENABLE_MOC
    PUBLIC_DEPENDENCY   
        Qt::Gui
        Qt::GuiPrivate
        Qt::Widgets
        Qt::WidgetsPrivate
        Qt::Multimedia
        QScintilla
    QRC_FILE Resources.qrc
)
