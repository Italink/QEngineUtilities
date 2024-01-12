#include "QQuickDetailsViewMananger.h"
#include "QPropertyHandle.h"
#include "QQmlHelper.h"
#include "Color/QtColorDialog.h"
#include <QRegularExpression>

QQuickDetailsViewManager* QQuickDetailsViewManager::Get()
{
	static QQuickDetailsViewManager ins;
	return &ins;
}

void QQuickDetailsViewManager::unregisterCustomClassLayout(const QMetaObject* InMetaObject)
{
	mCustomClassLayoutMap.remove(InMetaObject);
}

void QQuickDetailsViewManager::unregisterCustomPropertyTypeLayout(const QMetaType& InMetaType)
{
	mCustomPropertyTypeLayoutMap.remove(InMetaType);
}

void QQuickDetailsViewManager::registerCustomPropertyValueEditorCreator(const QMetaType& inMetaType, CustomPropertyValueWidgetCreator Creator)
{
	mPropertyValueEditorCreatorMap.insert(inMetaType, Creator);
}

void QQuickDetailsViewManager::unregisterCustomPropertyValueEditorCreator(const QMetaType& inMetaType)
{
	mPropertyValueEditorCreatorMap.remove(inMetaType);
}

QQuickItem* QQuickDetailsViewManager::createValueEditor(QPropertyHandle* inHandle, QQuickItem* parent)
{
	if (mPropertyValueEditorCreatorMap.contains(inHandle->getType())) {
		return mPropertyValueEditorCreatorMap[inHandle->getType()](inHandle, parent);
	}
	return nullptr;
}

QSharedPointer<IDetailCustomization> QQuickDetailsViewManager::getCustomDetailLayout(const QMetaObject* InMetaObject)
{
	for (const auto& It : mCustomClassLayoutMap.asKeyValueRange()) {
		if (It.first == InMetaObject) {
			return It.second();
		}
	}
	for (const auto& It : mCustomClassLayoutMap.asKeyValueRange()) {
		if (InMetaObject->inherits(It.first)) {
			return It.second();
		}
	}
	return nullptr;
}

QSharedPointer<IPropertyTypeCustomization> QQuickDetailsViewManager::getCustomPropertyType(const QMetaType& InMetaType)
{
	for (const auto& It : mCustomPropertyTypeLayoutMap.asKeyValueRange()) {
		if (It.first == InMetaType) {
			return It.second();
		}
	}
	const QMetaObject* Child = nullptr;
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(InMetaType.name());
	QStringList matchTexts = match.capturedTexts();
	QMetaType innerMetaType;
	if (!matchTexts.isEmpty()) {
		innerMetaType = QMetaType::fromName((matchTexts.back()).toLocal8Bit());
		Child = innerMetaType.metaObject();
	}
	else {
		Child = InMetaType.metaObject();
	}

	for (const auto& It : mCustomPropertyTypeLayoutMap.asKeyValueRange()) {

		const QMetaObject* Parent = It.first.metaObject();
		if (Parent && Child && Child->inherits(Parent)) {
			return It.second();
		}
	}
	return nullptr;
}

#define REGINTER_NUMBER_EDITOR_CREATOR(TypeName,DefaultPrecision) \
	registerCustomPropertyValueEditorCreator(QMetaType::fromType<TypeName>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* { \
		QQmlEngine* engine = qmlEngine(parent);\
		QQmlContext* context = qmlContext(parent);\
		QQmlComponent nameComp(engine);\
		nameComp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					NumberBox{
						width: parent.width
					}
				)", QUrl());\
		QVariantMap initialProperties; \
		initialProperties["parent"] = QVariant::fromValue(parent);\
		auto valueEditor = qobject_cast<QQuickItem*>(nameComp.createWithInitialProperties(initialProperties, context));\
		valueEditor->setParentItem(parent);\
		TypeName min = handle->getMetaData("Min").toDouble();\
		TypeName max = handle->getMetaData("Max").toDouble();\
		double step = handle->getMetaData("Step").toDouble();\
		int precision = handle->getMetaData("Precision").toInt(); \
		valueEditor->setProperty("step", step<=0.00001 ? 0.1:step);\
		valueEditor->setProperty("number", handle->getVar());\
		valueEditor->setProperty("precision", precision == 0 ? DefaultPrecision:precision); \
		if (min < max) {\
			valueEditor->setProperty("min", min);\
			valueEditor->setProperty("max", max);\
			valueEditor->setProperty("isLimited", true);\
		}\
		connect(valueEditor, SIGNAL(valueChanged(QVariant)), handle, SLOT(setVar(QVariant))); \
		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setNumber(QVariant))); \
		return valueEditor;\
	});

QQuickDetailsViewManager::QQuickDetailsViewManager()
{
	REGINTER_NUMBER_EDITOR_CREATOR(int,0);
	REGINTER_NUMBER_EDITOR_CREATOR(unsigned int, 0);
	REGINTER_NUMBER_EDITOR_CREATOR(float, 2);
	REGINTER_NUMBER_EDITOR_CREATOR(double, 3);

	registerCustomPropertyValueEditorCreator(QMetaType::fromType<QString>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* { 
		QQmlEngine* engine = qmlEngine(parent); 
		QQmlContext* context = qmlContext(parent); 
		QQmlComponent comp(engine); 
		comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					TextBox{
						 width: parent.width
					}
				)", QUrl()); 
		QVariantMap initialProperties; 
		initialProperties["parent"] = QVariant::fromValue(parent); 
		auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
		qDebug() << comp.errorString();
		valueEditor->setParentItem(parent); 
		valueEditor->setProperty("value", handle->getVar());
		connect(valueEditor, SIGNAL(asValueChanged(QVariant)), handle, SLOT(setVar(QVariant)));

		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
		return valueEditor;
	});

	registerCustomPropertyValueEditorCreator(QMetaType::fromType<QVector4D>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* {
		QQmlEngine* engine = qmlEngine(parent);
		QQmlContext* context = qmlContext(parent);
		QQmlComponent comp(engine);
		comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					Vec4Box{
						width: parent.width
					}
				)", QUrl());
		QVariantMap initialProperties;
		initialProperties["parent"] = QVariant::fromValue(parent);
		auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
		qDebug() << comp.errorString();
		valueEditor->setParentItem(parent);
		valueEditor->setProperty("value", handle->getVar());
		connect(valueEditor, SIGNAL(asValueChanged(QVariant)), handle, SLOT(setVar(QVariant)));
		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
		return valueEditor;
	});

	registerCustomPropertyValueEditorCreator(QMetaType::fromType<QVector3D>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* {
		QQmlEngine* engine = qmlEngine(parent);
		QQmlContext* context = qmlContext(parent);
		QQmlComponent comp(engine);
		comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					Vec3Box{
						 width: parent.width
					}
				)", QUrl());
		QVariantMap initialProperties;
		initialProperties["parent"] = QVariant::fromValue(parent);
		auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
		qDebug() << comp.errorString();
		valueEditor->setParentItem(parent);
		valueEditor->setProperty("value", handle->getVar());
		connect(valueEditor, SIGNAL(asValueChanged(QVariant)), handle, SLOT(setVar(QVariant)));
		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
		return valueEditor;
	});

	registerCustomPropertyValueEditorCreator(QMetaType::fromType<QVector2D>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* {
		QQmlEngine* engine = qmlEngine(parent);
		QQmlContext* context = qmlContext(parent);
		QQmlComponent comp(engine);
		comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					Vec2Box{
						 width: parent.width
					}
				)", QUrl());
		QVariantMap initialProperties;
		initialProperties["parent"] = QVariant::fromValue(parent);
		auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
		qDebug() << comp.errorString();
		valueEditor->setParentItem(parent);
		valueEditor->setProperty("value", handle->getVar());
		connect(valueEditor, SIGNAL(asValueChanged(QVariant)), handle, SLOT(setVar(QVariant)));
		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
		return valueEditor;
	});

	registerCustomPropertyValueEditorCreator(QMetaType::fromType<QColor>(), [](QPropertyHandle* handle, QQuickItem* parent)->QQuickItem* {
		QQmlEngine* engine = qmlEngine(parent);
		QQmlContext* context = qmlContext(parent);
		QQmlComponent comp(engine);
		comp.setData(R"(
					import QtQuick;
					import QtQuick.Controls;
					import "qrc:/Resources/Qml"
					ColorBox{
						 width: parent.width
					}
				)", QUrl());
		QVariantMap initialProperties;
		initialProperties["parent"] = QVariant::fromValue(parent);
		auto valueEditor = qobject_cast<QQuickItem*>(comp.createWithInitialProperties(initialProperties, context));
		qDebug() << comp.errorString();
		valueEditor->setParentItem(parent);
		valueEditor->setProperty("value", handle->getVar());
		connect(valueEditor, SIGNAL(asValueChanged(QVariant)), handle, SLOT(setVar(QVariant)));
		connect(handle, SIGNAL(asRequestRollback(QVariant)), valueEditor, SLOT(setValue(QVariant)));
		QQmlLambdaHelper::connect(valueEditor, SIGNAL(asClicked()), [valueEditor]() {
			QColor color = valueEditor->property("value").value<QColor>();
			QRect rect = valueEditor->boundingRect().toRect();
			auto g = valueEditor->mapToGlobal(0, 0);
			rect.moveTopLeft(valueEditor->mapToGlobal(0, 0).toPoint());
			QtColorDialog::CreateAndShow(color, rect);
			QObject::connect(QtColorDialog::Current, &QtColorDialog::asColorChanged, valueEditor, [&](const QColor& color) {
				valueEditor->metaObject()->invokeMethod(valueEditor, "setValue", Q_ARG(QVariant, color));
			});
		});
		return valueEditor;
	});
}

