#ifndef QInstanceTreeWidget_h__
#define QInstanceTreeWidget_h__

#include "QTreeWidget"
#include "QInstance.h"

class QInstanceTreeWidget: public QTreeWidget
{
	Q_OBJECT
public:
	QInstanceTreeWidget();
	void SetInstances(const QList<QSharedPointer<QInstance>>& inInstances);
protected:
	void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
	void AddItemInternal(QTreeWidgetItem* inParentItem, QInstance* inParentInstance);
	QTreeWidgetItem* CreateItemForInstance(QInstance* inInstance);
	void ForceRefresh();
	bool eventFilter(QObject* object, QEvent* event) override;
	bool IsIgnoreObject(QObject* inObject);
Q_SIGNALS:
	void AsInstanceSelected(QSharedPointer<QInstance>);
private:
	QVector<QSharedPointer<QInstance>> mTopLevelInstances;
	QMap<QTreeWidgetItem*, QSharedPointer<QInstance>> mItemMap;
};

#endif // QInstanceTreeWidget_h__