#ifndef QObjectTreeView_h__
#define QObjectTreeView_h__

#include "QTreeWidget"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QObjectTreeView : public QTreeWidget {
	Q_OBJECT
public:
	QObjectTreeView();
	void SetObjects(QObjectList InObjects);
	void SelectObjects(QObjectList InObjects);
protected:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
	void AddItemInternal(QTreeWidgetItem* inParentItem, QObject* inParentInstance);
	QTreeWidgetItem* CreateItemForInstance(QObject* inInstance);
	void ForceRefresh();
	bool eventFilter(QObject* object, QEvent* event) override;
	bool IsIgnoreObject(QObject* inObject);
Q_SIGNALS:
	void AsObjecteSelected(QObject*);
private:
	QVector<QObject*> mTopLevelObjects;
	QMap<QTreeWidgetItem*, QObject*> mItemMap;
};


#endif // QObjectTreeView_h__

