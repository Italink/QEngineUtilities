#include "QPropertyHandleImpl_Enum.h"
#include <QComboBox>
#include <QMetaEnum>
#include <QWidget>
#include <QBoxLayout>
#include "QPropertyHandle.h"


QPropertyHandleImpl_Enum::QPropertyHandleImpl_Enum(QPropertyHandle* inHandle)
	:IPropertyHandleImpl(inHandle) {
	const QMetaObject* metaObj = mHandle->getType().metaObject();
	if (metaObj){
		const QMetaEnum& metaEnum = metaObj->enumerator(metaObj->indexOfEnumerator(QString(mHandle->getType().name()).split("::").last().toLocal8Bit()));
		for (int i = 0; i < metaEnum.keyCount(); i++) {
			mNameToValueMap[metaEnum.key(i)] = metaEnum.value(i);
			mKeys << metaEnum.key(i);
		}
	}
}

QQuickItem* QPropertyHandleImpl_Enum::createValueEditor(QQuickItem* inParent)
{
	QQmlEngine* engine = qmlEngine(inParent);
	QQmlContext* context = qmlContext(inParent);
	QQmlComponent comp(engine);
	comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					TextComboBox{
						width: parent.width
					}
				)", QUrl());
	QVariantMap initialProperties;
	initialProperties["parent"] = QVariant::fromValue(inParent);
	auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
	qDebug() << comp.errorString();
	valueEditor->setParentItem(inParent);
	valueEditor->setProperty("value", mHandle->getVar());
	valueEditor->setProperty("model", mKeys);
	QObject::connect(valueEditor, SIGNAL(asValueChanged(QVariant)), mHandle, SLOT(setVar(QVariant)));
	QObject::connect(mHandle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
	return valueEditor;
}