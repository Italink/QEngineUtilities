#include "QEngineEditorSignals.h"

QEngineEditorSignals* QEngineEditorSignals::Instance() {
	static QEngineEditorSignals Ins;
	return &Ins;
}

