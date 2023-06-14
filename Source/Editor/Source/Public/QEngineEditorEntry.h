#ifndef QEngineEditorEntry_h__
#define QEngineEditorEntry_h__

#include <QObject>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QEngineEditorEntry: public QObject {
	Q_OBJECT
public:
	static QEngineEditorEntry* Instance();
};

#endif // QEngineEditorEntry_h__
