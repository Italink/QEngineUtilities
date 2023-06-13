#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailView.h"
#include "DetailView/QDetailViewRow.h"


QDetailViewRow* QDetailLayoutBuilder::NewChildRow() {
	return mDetailView->AddTopLevelRow();
}

QDetailViewRow* QRowLayoutBuilder::NewChildRow() {
	return mRow->AddChildRow();
}
