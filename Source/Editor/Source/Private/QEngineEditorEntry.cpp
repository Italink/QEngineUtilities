#include "QEngineEditorEntry.h"

QEngineEditorEntry* QEngineEditorEntry::Instance() {
	static QEngineEditorEntry Ins;
	return &Ins;
}

