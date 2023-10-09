#ifndef QDETAILAYOUTBUILDER_H
#define QDETAILAYOUTBUILDER_H

#include "DetailView/IDetailLayoutBuilder.h"

class QENGINEEDITOR_API QDetailLayoutBuilder: public IDetailLayoutBuilder{
public:
	QDetailLayoutBuilder(QDetailView* InDetailView)
	: IDetailLayoutBuilder(InDetailView){}
protected:
	QDetailViewRow* newChildRow() override;
};

class QENGINEEDITOR_API QRowLayoutBuilder: public IDetailLayoutBuilder {
public:
	QRowLayoutBuilder(QDetailView* InDetailView, QDetailViewRow* InRow)
	: IDetailLayoutBuilder(InDetailView)
	, mRow(InRow){}
protected:
	QDetailViewRow* newChildRow() override;
	QDetailViewRow* row() override { return mRow; }
private:
	QDetailViewRow* mRow = nullptr;
};

#endif // QDETAILAYOUTBUILDER_H
