#include "QQuickDetailsView.h"
#include "private/qqmldata_p.h"
#include <QQmlEngine>
#include "private/qquicksplitview_p.h"
#include "qqmlcontext.h"
#include "QQuickDetailsViewPrivate.h"
#include "QQuickDetailsViewRow.h"
#include "QQmlHelper.h"

void QQuickDetailsViewPrivate::initItemCallback(int serializedModelIndex, QObject* object)
{
    QQuickTreeViewExPrivate::initItemCallback(serializedModelIndex, object);
	auto item = qobject_cast<QQuickItem*>(object);
	if (!item)
		return;
	const QModelIndex& index = m_treeModelToTableModel.mapToModel(serializedModelIndex);;
	IDetailsViewRow* node = static_cast<IDetailsViewRow*>(index.internalPointer());
	node->setupItem(item);
 //   Q_Q(QQuickDetailsView);
}


QQuickDetailsView::QQuickDetailsView(QQuickItem* parent /*= nullptr*/)
    : QQuickTreeViewEx(*(new QQuickDetailsViewPrivate()),parent)
{
    setModel(QVariant::fromValue(d_func()->mModel));
	setReuseItems(false);
	setEditTriggers(QQuickTableView::EditTrigger::DoubleTapped);
}

qreal QQuickDetailsView::getSpliterPencent() const
{
    return d_func()->mSpliterPencent;
}

void QQuickDetailsView::setSpliterPencent(qreal val)
{
    if(val != d_func()->mSpliterPencent){
		d_func()->mSpliterPencent = val;
		Q_EMIT asSpliterPencentChanged(val);
    }
}

void QQuickDetailsView::setObject(QObject* inObject)
{
	if (inObject != d_func()->mModel->getObject()) {
		d_func()->mModel->setObject(inObject);
		Q_EMIT asObjectChanged(inObject);
	}
}

QObject* QQuickDetailsView::getObject() const
{
	return d_func()->mModel->getObject();
}

void QQuickDetailsView::componentComplete()
{
    QQmlEngine* engine = qmlEngine(this);
	QQmlComponent* delegate = new QQmlComponent(engine, this);
	engine->rootContext()->setContextProperty("helper", QQmlHelper::Get());
	delegate->setData(R"(
        import QtQuick;
        import QtQuick.Controls;
        import QtQuick.Layouts;
		import Qt.DetailsView;
        Item {
			 id: detailsDelegate
			 readonly property real indent: 20
             readonly property real padding: 5
			 required property DetailsView detailsView
			 required property int row
             required property bool isTreeNode
             required property bool expanded
             required property int hasChildren
             required property int depth
             implicitWidth: detailsView.width
             implicitHeight: 30
		     TapHandler {
                 onTapped: detailsView.toggleExpanded(row)
             }
         })", QUrl("QQuickDetailsView.componentComplete"));;
	setDelegate(delegate);
    QQuickTreeViewEx::componentComplete();
}
