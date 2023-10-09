#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailView.h"
#include "DetailView/QDetailViewRow.h"


QDetailViewRow* QDetailLayoutBuilder::newChildRow() {
	return mDetailView->addTopLevelRow();
}

QDetailViewRow* QRowLayoutBuilder::newChildRow() {
	return mRow->addChildRow();
}
