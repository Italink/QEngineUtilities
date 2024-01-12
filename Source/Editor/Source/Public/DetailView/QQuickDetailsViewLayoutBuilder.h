#ifndef QQuickDetailsViewLayoutBuilder_h__
#define QQuickDetailsViewLayoutBuilder_h__

#include "QQuickDetailsViewRow.h"

class QENGINEEDITOR_API QQuickDetailsViewHeaderRowBuilder {
public:
	QQuickDetailsViewHeaderRowBuilder(QQuickItem* inRootItem);
	QPair<QQuickItem*, QQuickItem*> makeNameValueSlot();
	void makePropertyHeader(QPropertyHandle* inHandle);
private:
	QQuickItem* mRootItem = nullptr;
};

class  QENGINEEDITOR_API QQuickDetailsViewLayoutBuilder {
public:
	QQuickDetailsViewLayoutBuilder(IDetailsViewRow* inRow);
	QQuickDetailsViewLayoutBuilder* addCustomRow(QQuickItem* item);
	void addProperty(QPropertyHandle* inPropertyHandle);
	void addObject(QObject* inObject);
private:
	IDetailsViewRow* mRow = nullptr;
};

#endif // QQuickDetailsViewLayoutBuilder_h__
