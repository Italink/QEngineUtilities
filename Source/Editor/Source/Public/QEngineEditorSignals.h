#ifndef QEngineEditorSignals_h__
#define QEngineEditorSignals_h__

#include <QObject>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QEngineEditorSignals: public QObject {
	Q_OBJECT
public:
	static QEngineEditorSignals* Instance();
};

#endif // QEngineEditorSignals_h__
