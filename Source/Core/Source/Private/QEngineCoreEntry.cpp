#include "QEngineCoreEntry.h"

QEngineCoreEntry* QEngineCoreEntry::Instance() {
	static QEngineCoreEntry Ins;
	return &Ins;
}

