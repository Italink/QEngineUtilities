#include "IPropertyHandleImpl.h"
#include "QPropertyHandle.h"

IPropertyHandleImpl::IPropertyHandleImpl(QPropertyHandle* inHandle):
	mHandle(inHandle)
{
}

QPropertyHandle* IPropertyHandleImpl::findChildHandle(const QString& inSubName)
{
	return QPropertyHandle::Find(mHandle->parent(), mHandle->createSubPath(inSubName));
}

QPropertyHandle* IPropertyHandleImpl::createChildHandle(const QString& inSubName)
{
	return nullptr;
}

QQuickItem* IPropertyHandleImpl::createNameEditor(QQuickItem* inParent)
{
	QQmlEngine* engine = qmlEngine(inParent);
	QQmlContext* context = qmlContext(inParent);
	QQmlComponent nameComp(engine);
	nameComp.setData(R"(
		import QtQuick;
		import QtQuick.Controls;
		Item{
			implicitHeight: 25
			width: parent.width
			Text {
				anchors.fill: parent
				verticalAlignment: Text.AlignVCenter
				clip: true
				elide: Text.ElideRight
				text: model.name
			Component.onCompleted: {
				console.log("----Component.onCompleted",model.name)
			}
			Component.onDestruction: {
				console.log("----Component.onDestruction",model.name)
			}
			}
		}
   )", QUrl());
	QVariantMap initialProperties;
	initialProperties["parent"] = QVariant::fromValue(inParent);
	auto nameEditor = qobject_cast<QQuickItem*>(nameComp.createWithInitialProperties(initialProperties, context));
	nameEditor->setParentItem(inParent);
	return nameEditor;
}
