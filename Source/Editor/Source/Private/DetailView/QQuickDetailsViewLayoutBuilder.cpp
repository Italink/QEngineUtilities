#include "QQuickDetailsViewLayoutBuilder.h"
#include "QQuickDetailsViewMananger.h"

QQuickDetailsViewHeaderRowBuilder::QQuickDetailsViewHeaderRowBuilder(QQuickItem* inRootItem)
	: mRootItem(inRootItem)
{
}

QPair<QQuickItem*, QQuickItem*> QQuickDetailsViewHeaderRowBuilder::makeNameValueSlot()
{
	QQmlEngine* engine = qmlEngine(mRootItem);
	QQmlContext* context = qmlContext(mRootItem);
	QQmlContext* newContext = new QQmlContext(context, mRootItem);
	QQmlComponent rootComp(newContext->engine());
	rootComp.setData(R"(
        import QtQuick;
        import QtQuick.Controls;
        Rectangle{
            anchors.fill: parent
            Text {
                id: indicator
                visible: detailsDelegate.isTreeNode && detailsDelegate.hasChildren
                x: padding + (detailsDelegate.depth * detailsDelegate.indent)
                anchors.verticalCenter: parent.verticalCenter
                text: "▸"
                rotation: detailsDelegate.expanded ? 90 : 0
            }
            Item{
                id: nameEditorContent
                anchors.left: parent.left 
                anchors.leftMargin: padding + (detailsDelegate.isTreeNode ? (detailsDelegate.depth + 1) * detailsDelegate.indent : 0)
                anchors.right: splitter.left
            }
            Item{
                id: valueEditorContent
                anchors.left: splitter.left 
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.right: parent.right
            }
            Rectangle{
                id: splitter
                x: detailsView.SpliterPencent * detailsView.width
                height: parent.height
                width: 2
                color: "red"
                MouseArea {
                    id: dragArea
                    hoverEnabled:true
                    cursorShape:containsMouse?Qt.SplitHCursor:Qt.ArrowCursor
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis:Drag.XAxis
                    drag.minimumX: 10
                    drag.maximumX: detailsView.width - 10
                    onPositionChanged: {
                        detailsView.SpliterPencent = splitter.x/detailsView.width
                    }
                }
            }
        }
    )", QUrl());
    QQuickItem* slotItem = qobject_cast<QQuickItem*>(rootComp.create(newContext));
    slotItem->setParentItem(mRootItem);
    return { slotItem->childItems()[1] ,slotItem->childItems()[2] };
}

void QQuickDetailsViewHeaderRowBuilder::makePropertyHeader(QPropertyHandle* inHandle)
{
	QQmlEngine* engine = qmlEngine(mRootItem);
	QQmlContext* context = qmlContext(mRootItem);
    QPair<QQuickItem*, QQuickItem*> slotItem = makeNameValueSlot();
	QQuickItem* nameEditor = inHandle->createNameEditor(slotItem.first);
	QQuickItem* valueEditor = inHandle->createValueEditor(slotItem.second);
	context->parentContext()->setContextProperty("heightProxy", valueEditor ? valueEditor : nameEditor);
}

QQuickDetailsViewLayoutBuilder::QQuickDetailsViewLayoutBuilder(IDetailsViewRow* inRow)
	: mRow(inRow)
{
}

QQuickDetailsViewLayoutBuilder* QQuickDetailsViewLayoutBuilder::addCustomRow(QQuickItem* item)
{
    return nullptr;
}

void QQuickDetailsViewLayoutBuilder::addProperty(QPropertyHandle* inPropertyHandle)
{
	QSharedPointer<IDetailsViewRow> child(new QDetailsViewRow_Property(inPropertyHandle));
	mRow->addChild(child);
	child->attachChildren();
}

void QQuickDetailsViewLayoutBuilder::addObject(QObject* inObject)
{
	for (int i = 1; i < inObject->metaObject()->propertyCount(); i++) {
		QMetaProperty prop = inObject->metaObject()->property(i);
		QString propertyPath = prop.name();
		QPropertyHandle* handler = QPropertyHandle::FindOrCreate(inObject, propertyPath);
		if (handler)
			addProperty(handler);
		else
			qWarning() << "property handle is null";
	}
}
