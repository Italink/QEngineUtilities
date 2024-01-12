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

	//	return;
	//QQmlEngine* engine = qmlEngine(item);
	//QQmlContext* context = qmlContext(item);
	//QQmlComponent* component = new QQmlComponent(engine);
	//component->setData(R"(
 //       import QtQuick;
 //       import QtQuick.Window;
 //       import QtQuick.Controls;
 //       Item {
 //           id: pt
 //           height: 100
 //           anchors.left: parent.left
 //           anchors.right: parent.right
 //           Button{  
 //               text : index
 //           }
 //           Button{ 
 //               text : index + 1 
 //           }
 //       }
 //       )", QUrl());
	//QQmlContext* newContext = new QQmlContext(context, item);
	//newContext->setParent(item);
	//newContext->setContextProperty("index", serializedModelIndex);
	//newContext->setContextProperty("parent", item);
	//auto newItem = qobject_cast<QQuickItem*>(component->create(newContext));
	//qDebug() << component->errorString();
	////auto splitterView = qobject_cast<QQuickSplitView*>(newItem);
	////qDebug() << splitterView->property("height");
	//newContext->setContextObject(newItem);
	//context->parentContext()->setContextProperty("pt", newItem);
	////context->setContextProperty("pt", splitterView);
	//newItem->setParentItem(item);

	//QQmlComponent* splitter = new QQmlComponent(engine);
	//splitter->setData(R"(
 //       import QtQuick;
 //       import QtQuick.Window;
 //       import QtQuick.Controls;
 //       Rectangle{//spliter
 //           property real spliterPos : treeView.SpliterPencent * parent.width
 //           id:spliterRec
 //           x:spliterPos
 //           height: parent.height
 //           width: 2
 //           color: "red"
 //           MouseArea {
 //               id: dragArea
 //               hoverEnabled:true

 //               cursorShape:containsMouse?Qt.SplitHCursor:Qt.ArrowCursor

 //               anchors.fill: parent

 //               drag.target: parent
 //               drag.axis:Drag.XAxis
 //               drag.minimumX: 100
 //               drag.maximumX: root.width - 150

 //               onPositionChanged: {
 //                   console.log("---",spliterRec.x,treeView.width)
 //                   treeView.SpliterPencent = spliterRec.x/treeView.width
 //               }
 //           }
 //       }
 //       )", QUrl());
	//newContext->setContextProperty("parent", item);
	//newContext->setContextProperty("SpliterPencent", q->getSpliterPencent());
	//auto splitterItem = qobject_cast<QQuickItem*>(splitter->create(newContext));
	//qDebug() << splitter->errorString();
	//splitterItem->setParentItem(item);
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
             implicitHeight: heightProxy.implicitHeight
		     TapHandler {
                 onTapped: detailsView.toggleExpanded(row)
             }
         })", QUrl());
	delegate->create();
	qDebug() << delegate->errorString();
	setDelegate(delegate);
    QQuickTreeViewEx::componentComplete();
}
