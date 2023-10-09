#ifndef QObjectTreeView_h__
#define QObjectTreeView_h__

#include "QTreeWidget"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QObjectTreeView : public QTreeWidget {
	Q_OBJECT
public:
	QObjectTreeView();
	void setObjects(QObjectList InObjects);
	void selectObjects(QObjectList InObjects);
protected:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
	void addItemInternal(QTreeWidgetItem* inParentItem, QObject* inParentInstance);
	QTreeWidgetItem* createItemForInstance(QObject* inInstance);
	void forceRefresh();
	bool eventFilter(QObject* object, QEvent* event) override;
	bool isIgnoreObject(QObject* inObject);
Q_SIGNALS:
	void asObjecteSelected(QObject*);
private:
	QVector<QObject*> mTopLevelObjects;
	QMap<QTreeWidgetItem*, QObject*> mItemMap;
};


#endif // QObjectTreeView_h__

