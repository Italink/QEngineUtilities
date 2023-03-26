#include "DetailView/QDetailViewManager.h"
#include "Customization/DetailCustomization_QMediaPlayer.h"
#include "Customization/DetailCustomization_QObject.h"
#include "Customization/DetailCustomization_QRhiGraphicsPipelineBuilder.h"
#include "Customization/DetailCustomization_QRhiUniformBlock.h"
#include "Customization/PropertyTypeCustomization_QMatrix4x4.h"
#include "Customization/PropertyTypeCustomization_TextureInfo.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
#include "Type/QColor4D.h"
#include "Widgets/Color/QColorButton.h"
#include "Widgets/QColor4DButton.hpp"
#include "Widgets/QFilePathBox.h"
#include "Widgets/QHoverTextEdit.h"
#include "Widgets/QImageBox.h"
#include "Widgets/QNumberBox.h"
#include "Widgets/QVectorBox.h"
#include <QCheckBox>
#include <QComboBox>
#include <QMetaMethod>
#include <QObject>
#include "Customization/PropertyTypeCustomization_QStaticMesh.h"
#include "Asset/QStaticMesh.h"
#include "Customization/DetailCustomization_QGlslSandboxRenderPass.h"

QDetailViewManager* QDetailViewManager::Instance()
{
	static QDetailViewManager ins;
	return &ins;
}

void QDetailViewManager::UnregisterCustomClassLayout(const QMetaObject* InMetaObject) {
	mCustomClassLayoutMap.remove(InMetaObject);
}

void QDetailViewManager::UnregisterCustomClassLayout(const QMetaType& InMetaType) {
	mCustomPropertyTypeLayoutMap.remove(InMetaType);
}

void QDetailViewManager::RegisterCustomPropertyValueWidgetCreator(const QMetaType& InMetaType, CustomPropertyValueWidgetCreator Creator) {
	mCustomPropertyValueWidgetMap.insert(InMetaType, Creator);
}

void QDetailViewManager::UnregisterCustomPropertyValueWidgeCreator(const QMetaType& InMetaType) {
	mCustomPropertyValueWidgetMap.remove(InMetaType);
}

QSharedPointer<IDetailCustomization> QDetailViewManager::GetCustomDetailLayout(const QMetaObject* InMetaObject)
{
	for(const auto& It: mCustomClassLayoutMap.asKeyValueRange()){
		if (It.first == InMetaObject){
			return It.second();
		}
	}
	for (const auto& It : mCustomClassLayoutMap.asKeyValueRange()) {
		if (InMetaObject->inherits(It.first)){
			return It.second();
		}
	}
	return nullptr;
}

QSharedPointer<IPropertyTypeCustomization> QDetailViewManager::GetCustomPropertyType(const QMetaType& InMetaType) {
	for (const auto& It : mCustomPropertyTypeLayoutMap.asKeyValueRange()) {
		if (It.first == InMetaType) {
			return It.second();
		}
	}
	for (const auto& It : mCustomPropertyTypeLayoutMap.asKeyValueRange()) {
		const QMetaObject* Child = InMetaType.metaObject();
		const QMetaObject* Parent = It.first.metaObject();
		if (Parent && Child && Child->inherits(Parent)) {
			return It.second();
		}
	}
	return nullptr;
}

QWidget* QDetailViewManager::GetCustomPropertyValueWidget(QPropertyHandle* InHandler) {
	if(mCustomPropertyValueWidgetMap.contains(InHandler->GetType())){
		return mCustomPropertyValueWidgetMap[InHandler->GetType()](InHandler);
	}
	return new QWidget;
}


QMetaData* QDetailViewManager::GetClassMetaData(const IDetailLayoutBuilder::ObjectContext& Context) {
	QSharedPointer<QMetaData> metaData = mClassMetaData.value(Context.MetaObject);
	if (metaData)
		return metaData.get();
	metaData = QSharedPointer<QMetaData>::create();
	for (int i = 0; i < Context.MetaObject->methodCount(); i++) {
		QMetaMethod method = Context.MetaObject->method(i);
		if (QString(method.name()).endsWith("_GetMetaData")) {
			QMetaData localMetaData;
			if(Context.ObjectPtr == Context.OwnerObject){
				method.invoke(Context.OwnerObject, Q_RETURN_ARG(QMetaData, localMetaData));
			}
			else{
				method.invokeOnGadget(Context.ObjectPtr, Q_RETURN_ARG(QMetaData, localMetaData));
			}
			for(auto meta:localMetaData.mClassMetaData.asKeyValueRange()){
				metaData->mClassMetaData[meta.first] = meta.second;
			}
			for (auto PropertyIter = localMetaData.mPropertiesMetaData.begin(); PropertyIter != localMetaData.mPropertiesMetaData.end(); ++PropertyIter) {
				metaData->mPropertiesMetaData[PropertyIter.key()] = PropertyIter.value();
			}
			for (auto category : localMetaData.mCategories.asKeyValueRange()) {
				metaData->mCategories[category.first] = category.second;
			}
		}
	}
	mClassMetaData.insert(Context.MetaObject, metaData);
	return metaData.get();
}

QDetailViewManager::QDetailViewManager()
{
	RegisterBuildIn();
}

#define  REGISTER_NUMERIC_TYPE(TypeName) \
		RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<TypeName>(), [](QPropertyHandle* InHandler) { \
			TypeName min = InHandler->GetMetaData("Min").toDouble();\
			TypeName max = InHandler->GetMetaData("Max").toDouble();\
			QNumberBox* numberBox = new QNumberBox((TypeName)0.0, min < max, min, max);\
			InHandler->Bind(numberBox, &QNumberBox::AsValueChanged,\
				[numberBox]() {\
				return numberBox->GetVar();\
			},\
			[numberBox](QVariant var) {\
				numberBox->SetVar(var);\
			});\
			return numberBox;\
		});

void QDetailViewManager::RegisterBuildIn() {
	RegisterCustomClassLayout<DetailCustomization_QObject>(&QObject::staticMetaObject);
	RegisterCustomClassLayout<DetailCustomization_QRhiUniformBlock>(&QRhiUniformBlock::staticMetaObject);
	RegisterCustomClassLayout<DetailCustomization_QRhiGraphicsPipelineBuilder>(&QRhiGraphicsPipelineBuilder::staticMetaObject);
	RegisterCustomClassLayout<DetailCustomization_QMediaPlayer>(&QMediaPlayer::staticMetaObject);
	RegisterCustomClassLayout<DetailCustomization_QGlslSandboxRenderPass>(&QGlslSandboxRenderPass::staticMetaObject);

	RegisterCustomPropertyTypeLayout<QRhiGraphicsPipelineBuilder::TextureInfo*, PropertyTypeCustomization_TextureInfo>();
	RegisterCustomPropertyTypeLayout<QSharedPointer<QStaticMesh>, PropertyTypeCustomization_QStaticMesh>();
	RegisterCustomPropertyTypeLayout<QMatrix4x4, PropertyTypeCustomization_QMatrix4x4>();

	qRegisterMetaType<QRhiGraphicsPipelineBuilder::TextureInfo>();
	qRegisterMetaType<QRhiGraphicsPipelineBuilder*>();

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<bool>(),[](QPropertyHandle* InHandler) {
		QCheckBox* checkBox = new QCheckBox;
		InHandler->Bind(checkBox, &QCheckBox::stateChanged,
			[checkBox]() {
			return checkBox->checkState() == Qt::Checked;
		},
			[checkBox](QVariant var) {
			checkBox->setCheckState(var.toBool() ? Qt::Checked : Qt::Unchecked);
		}
		);
		return checkBox;
	});

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QImage>(), [](QPropertyHandle* InHandler) {
		QImageBox* imageBox = new QImageBox;
		InHandler->Bind(imageBox, &QImageBox::AsImageChanged,
			[imageBox]() {
			return imageBox->GetImage();
		},
			[imageBox](QVariant var) {
			imageBox->SetImage(var.value<QImage>());
		}
		);
		return imageBox;
	});

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QColor>(), [](QPropertyHandle* InHandler) {
		QColorButton* colorButton = new QColorButton();
		InHandler->Bind(colorButton, &QColorButton::AsColorChanged,
			[colorButton]() {
			return colorButton->GetColor();
		},
			[colorButton](QVariant var) {
			colorButton->SetColor(var.value<QColor>());
		}
		);
		return colorButton;
	});

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QColor4D>(), [](QPropertyHandle* InHandler) {
		QColor4DButton* colorButton = new QColor4DButton();
		InHandler->Bind(colorButton, &QColor4DButton::AsColorChanged,
			[colorButton]() {
			return QVariant::fromValue<QColor4D>(colorButton->GetColor());
		},
			[colorButton](QVariant var) {
			colorButton->SetColor(var.value<QColor4D>());
		}
		);
		return colorButton;
	});

	REGISTER_NUMERIC_TYPE(int);
	REGISTER_NUMERIC_TYPE(float);
	REGISTER_NUMERIC_TYPE(double);

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QString>(), [](QPropertyHandle* InHandler)->QWidget* {
		const QString& type = InHandler->GetMetaData("Type").toString();
		const QString& placeholderText = InHandler->GetMetaData("PlaceholderText").toString();
		if (type.isEmpty() || type == "Line") {
			QHoverLineEdit* lineEdit = new QHoverLineEdit();
			lineEdit->SetPlaceholdText(placeholderText);
			InHandler->Bind(lineEdit, &QHoverLineEdit::AsTextChanged,
				[lineEdit]() {
				return  lineEdit->GetText();
			},
				[lineEdit](QVariant var) {
				lineEdit->SetText(var.toString());
			});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			const double height = InHandler->GetMetaData("Height").toDouble();
			QHoverTextEdit* textEdit = new QHoverTextEdit();
			InHandler->Bind(textEdit, &QHoverTextEdit::AsTextChanged,
				[textEdit]() {
				return  textEdit->GetText();
			},
				[textEdit](QVariant var) {
				textEdit->SetText(var.toString());
			});
			textEdit->SetPlaceholdText(placeholderText);
			textEdit->setFixedHeight(height);
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox("");
			InHandler->Bind(filePathBox, &QFilePathBox::AsPathChanged,
				[filePathBox]() {
				return  filePathBox->GetFilePath();
			},
				[filePathBox](QVariant var) {
				filePathBox->SetFilePath(var.toString());
			});
			return filePathBox;
		}
		else if (type == "Combo") {
			QComboBox* comboBox = new QComboBox();
			QStringList comboList = InHandler->GetMetaData("ComboList").toStringList();
;			for (auto item : comboList) {
				comboBox->addItem(item);
			}
			InHandler->Bind(comboBox, &QComboBox::currentTextChanged,
				[comboBox]() {
				return comboBox->currentText();
			},
				[comboBox](QVariant var) {
				comboBox->setCurrentText(var.toString());
			});
			comboBox->setMinimumWidth(90);
			return comboBox;
		}
		return nullptr;
	});

	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector2D>(), [](QPropertyHandle* InHandler) {
		QVector2DBox* vec2Box = new QVector2DBox();
		InHandler->Bind(vec2Box, &QVector2DBox::AsValueChanged,
			[vec2Box]() {
			return vec2Box->GetValue();
		},
			[vec2Box](QVariant var) {
			vec2Box->SetValue(var.value<QVector2D>());
		});
		return vec2Box;
	});
	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector3D>(), [](QPropertyHandle* InHandler) {
		QVector3DBox* vec3Box = new QVector3DBox();
		InHandler->Bind(vec3Box, &QVector3DBox::AsValueChanged,
			[vec3Box]() {
			return vec3Box->GetValue();
		},
			[vec3Box](QVariant var) {
			vec3Box->SetValue(var.value<QVector3D>());
		});
		return vec3Box;
	});
	RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector4D>(), [](QPropertyHandle* InHandler) {
		QVector4DBox* vec4Box = new QVector4DBox();
		InHandler->Bind(vec4Box, &QVector4DBox::AsValueChanged,
			[vec4Box]() {
			return vec4Box->GetValue();
		},
			[vec4Box](QVariant var) {
			vec4Box->SetValue(var.value<QVector4D>());
		});
		return vec4Box;
	});
}

