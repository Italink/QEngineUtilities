#include "QEngineCoreSignals.h"

QEngineCoreSignals* QEngineCoreSignals::Instance() {
	static QEngineCoreSignals Ins;
	return &Ins;
}

