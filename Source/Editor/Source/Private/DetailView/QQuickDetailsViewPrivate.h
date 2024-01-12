#ifndef QQuickDetailsViewPrivate_h__
#define QQuickDetailsViewPrivate_h__

#include "private/qquicktreeview_p_p.h"
#include "QQuickTreeViewExPrivate.h"
#include "QQuickDetailsView.h"
#include "QQuickDetailsViewModel.h"

class QQuickDetailsViewPrivate : public QQuickTreeViewExPrivate
{
	Q_DECLARE_PUBLIC(QQuickDetailsView)
public:
	QQuickDetailsViewPrivate();
	void initItemCallback(int serializedModelIndex, QObject* object) override;
	void updateRequiredProperties(int serializedModelIndex, QObject* object, bool init) override;
private:
	qreal mSpliterPencent = 0.3;
	QList<QObject*> mObjects;
	QQuickDetailsViewModel* mModel;
};

QQuickDetailsViewPrivate::QQuickDetailsViewPrivate()
	:mModel(new QQuickDetailsViewModel)
{

}

void QQuickDetailsViewPrivate::updateRequiredProperties(int serializedModelIndex, QObject* object, bool init)
{
	Q_Q(QQuickDetailsView);
	const QPoint cell = cellAtModelIndex(serializedModelIndex);
	const int row = cell.y();
	const int column = cell.x();
	setRequiredProperty("row", QVariant::fromValue(serializedModelIndex), serializedModelIndex, object, init);
	setRequiredProperty("detailsView", QVariant::fromValue(q), serializedModelIndex, object, init);
	setRequiredProperty("isTreeNode", column == 0, serializedModelIndex, object, init);
	setRequiredProperty("hasChildren", m_treeModelToTableModel.hasChildren(row), serializedModelIndex, object, init);
	setRequiredProperty("expanded", q->isExpanded(row), serializedModelIndex, object, init);
	setRequiredProperty("depth", m_treeModelToTableModel.depthAtRow(row), serializedModelIndex, object, init);
}

#endif // QQuickDetailsViewPrivate_h__