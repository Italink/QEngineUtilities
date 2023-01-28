#ifndef QDETAILAYOUTBUILDER_H
#define QDETAILAYOUTBUILDER_H

#include "DetailView/IDetailLayoutBuilder.h"

class QDetailLayoutBuilder: public IDetailLayoutBuilder{
public:
	QDetailLayoutBuilder(QDetailView* InDetailView)
	: IDetailLayoutBuilder(InDetailView){}
protected:
	QDetailViewRow* NewChildRow() override;
};

class QRowLayoutBuilder: public IDetailLayoutBuilder {
public:
	QRowLayoutBuilder(QDetailView* InDetailView, QDetailViewRow* InRow)
	: IDetailLayoutBuilder(InDetailView)
	, mRow(InRow){}
protected:
	QDetailViewRow* NewChildRow() override;
private:
	QDetailViewRow* mRow = nullptr;
};


#endif // QDETAILAYOUTBUILDER_H
