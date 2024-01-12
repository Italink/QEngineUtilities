#include "IPropertyHandleImpl.h"
#include "QPropertyHandleImpl_RawType.h"
#include "../QQuickDetailsViewMananger.h"

QPropertyHandleImpl_RawType::QPropertyHandleImpl_RawType(QPropertyHandle* inHandle)
	: IPropertyHandleImpl(inHandle)
{

}

QQuickItem* QPropertyHandleImpl_RawType::createValueEditor(QQuickItem* inParent)
{
	return QQuickDetailsViewManager::Get()->createValueEditor(mHandle, inParent);
	//QQmlEngine* engine = qmlEngine(inParent);
	//QQmlContext* context = qmlContext(inParent);
	//QQmlComponent nameComp(engine);
	//nameComp.setData(R"(
	//	import QtQuick;
	//	import QtQuick.Controls;
	//	import "qrc:/Qml/"
	//	NumberBox{
	//	}
 //  )", QUrl());
	//QVariantMap initialProperties;
	//initialProperties["parent"] = QVariant::fromValue(inParent);
	//auto valueEditor = qobject_cast<QQuickItem*>(nameComp.createWithInitialProperties(init-ialProperties, context));
	//qDebug() << nameComp.errorString();
	//valueEditor->setParentItem(inParent);
}
