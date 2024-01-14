qengine_add_target(QEngineEditor
    ENABLE_MOC
    PUBLIC_DEPENDENCY   
        Qt::Gui
        Qt::GuiPrivate
        Qt::Widgets
        Qt::WidgetsPrivate
        Qt::Multimedia
        Qt::QuickWidgets 
        QScintilla
        Qt::QuickPrivate
        Qt::QuickTemplates2
        Qt::QuickTemplates2Private
    QRC_FILE Resources.qrc
)
