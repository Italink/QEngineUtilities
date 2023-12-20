#include "DetailView/QDetailViewManager.h"
#include "Widgets/Color/QColorButton.h"
#include "Widgets/QFilePathBox.h"
#include "Widgets/QHoverTextEdit.h"
#include "Widgets/QImageBox.h"
#include "Widgets/QNumberBox.h"
#include "Widgets/QVectorBox.h"
#include <QCheckBox>
#include <QComboBox>
#include <QMetaMethod>
#include <QObject>
#include "Widgets/QFontBox.h"

QDetailViewManager* QDetailViewManager::Instance()
{
	static QDetailViewManager ins;
	return &ins;
}

void QDetailViewManager::unregisterCustomClassLayout(const QMetaObject* InMetaObject) {
	mCustomClassLayoutMap.remove(InMetaObject);
}

void QDetailViewManager::unregisterCustomClassLayout(const QMetaType& InMetaType) {
	mCustomPropertyTypeLayoutMap.remove(InMetaType);
}

void QDetailViewManager::registerCustomPropertyValueWidgetCreator(const QMetaType& InMetaType, CustomPropertyValueWidgetCreator Creator) {
	mCustomPropertyValueWidgetMap.insert(InMetaType, Creator);
}

void QDetailViewManager::unregisterCustomPropertyValueWidgeCreator(const QMetaType& InMetaType) {
	mCustomPropertyValueWidgetMap.remove(InMetaType);
}

QSharedPointer<IDetailCustomization> QDetailViewManager::getCustomDetailLayout(const QMetaObject* InMetaObject)
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

QSharedPointer<IPropertyTypeCustomization> QDetailViewManager::getCustomPropertyType(const QMetaType& InMetaType) {
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

QWidget* QDetailViewManager::getCustomPropertyValueWidget(QPropertyHandle* InHandler) {
	if(mCustomPropertyValueWidgetMap.contains(InHandler->getType())){
		return mCustomPropertyValueWidgetMap[InHandler->getType()](InHandler);
	}
	return new QWidget;
}

QDetailViewManager::QDetailViewManager()
{
	registerBuiltIn();
}

#define  REGISTER_NUMERIC_TYPE(TypeName) \
		registerCustomPropertyValueWidgetCreator(QMetaType::fromType<TypeName>(), [](QPropertyHandle* InHandler) { \
			TypeName min = InHandler->getMetaData("Min").toDouble();\
			TypeName max = InHandler->getMetaData("Max").toDouble();\
			QNumberBox* numberBox = new QNumberBox((TypeName)0.0, min < max, min, max);\
			InHandler->bind(numberBox, &QNumberBox::asValueChanged,\
				[numberBox]() {\
				return numberBox->getVar();\
			},\
			[numberBox](QVariant var) {\
				numberBox->setVar(var);\
			});\
			return numberBox;\
		});

void QDetailViewManager::registerBuiltIn() {
	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<bool>(),[](QPropertyHandle* InHandler) {
		QCheckBox* checkBox = new QCheckBox;
		InHandler->bind(checkBox, &QCheckBox::stateChanged,
			[checkBox]() {
			return checkBox->checkState() == Qt::Checked;
		},
			[checkBox](QVariant var) {
			checkBox->setCheckState(var.toBool() ? Qt::Checked : Qt::Unchecked);
		}
		);
		return checkBox;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QImage>(), [](QPropertyHandle* InHandler) {
		QImageBox* imageBox = new QImageBox;
		InHandler->bind(imageBox, &QImageBox::asImageChanged,
			[imageBox]() {
			return imageBox->getImage();
		},
			[imageBox](QVariant var) {
			imageBox->setImage(var.value<QImage>());
		}
		);
		return imageBox;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QFont>(), [](QPropertyHandle* InHandler) {
		QFontBox* fontBox = new QFontBox;
		InHandler->bind(fontBox, &QFontBox::asFontChanged,
			[fontBox]() {
				return fontBox->getFont();
			},
			[fontBox](QVariant var) {
				fontBox->setFont(var.value<QFont>());
			}
		);
		return fontBox;
	});


	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QColor>(), [](QPropertyHandle* InHandler) {
		QColorButton* colorButton = new QColorButton();
		InHandler->bind(colorButton, &QColorButton::asColorChanged,
			[colorButton]() {
				return colorButton->GetColor();
			},
			[colorButton](QVariant var) {
				colorButton->setColor(var.value<QColor>());
			}
		);
		return colorButton;
	});

	REGISTER_NUMERIC_TYPE(int);
	REGISTER_NUMERIC_TYPE(float);
	REGISTER_NUMERIC_TYPE(double);

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QString>(), [](QPropertyHandle* InHandler)->QWidget* {
		const QString& type = InHandler->getMetaData("Type").toString();
		const QString& placeholderText = InHandler->getMetaData("PlaceholderText").toString();
		if (type.isEmpty() || type == "Line") {
			QHoverLineEdit* lineEdit = new QHoverLineEdit();
			lineEdit->setPlaceholdText(placeholderText);
			InHandler->bind(lineEdit, &QHoverLineEdit::asTextChanged,
				[lineEdit]() {
				return  lineEdit->getDisplayText();
			},
				[lineEdit](QVariant var) {
				lineEdit->setDisplayText(var.toString());
			});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			const double height = InHandler->getMetaData("Height").toDouble();
			QHoverTextEdit* textEdit = new QHoverTextEdit();
			InHandler->bind(textEdit, &QHoverTextEdit::asTextChanged,
				[textEdit]() {
				return  textEdit->getDisplayText();
			},
				[textEdit](QVariant var) {
				textEdit->setDisplayText(var.toString());
			});
			textEdit->setPlaceholdText(placeholderText);
			textEdit->setFixedHeight(height);
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox("");
			InHandler->bind(filePathBox, &QFilePathBox::asPathChanged,
				[filePathBox]() {
				return  filePathBox->getFilePath();
			},
				[filePathBox](QVariant var) {
				filePathBox->setFilePath(var.toString());
			});
			return filePathBox;
		}
		else if (type == "Combo") {
			QComboBox* comboBox = new QComboBox();
			QStringList comboList = InHandler->getMetaData("ComboList").toStringList();
;			for (auto item : comboList) {
				comboBox->addItem(item);
			}
			InHandler->bind(comboBox, &QComboBox::currentTextChanged,
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

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector2D>(), [](QPropertyHandle* InHandler) {
		QVector2DBox* vec2Box = new QVector2DBox();
		InHandler->bind(vec2Box, &QVector2DBox::asValueChanged,
			[vec2Box]() {
			return vec2Box->getValue();
		},
			[vec2Box](QVariant var) {
			vec2Box->setValue(var.value<QVector2D>());
		});
		return vec2Box;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector3D>(), [](QPropertyHandle* InHandler) {
		QVector3DBox* vec3Box = new QVector3DBox();
		InHandler->bind(vec3Box, &QVector3DBox::asValueChanged,
			[vec3Box]() {
			return vec3Box->getValue();
		},
			[vec3Box](QVariant var) {
			vec3Box->setValue(var.value<QVector3D>());
		});
		return vec3Box;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QVector4D>(), [](QPropertyHandle* InHandler) {
		QVector4DBox* vec4Box = new QVector4DBox();
		InHandler->bind(vec4Box, &QVector4DBox::asValueChanged,
			[vec4Box]() {
			return vec4Box->getValue();
		},
			[vec4Box](QVariant var) {
			vec4Box->setValue(var.value<QVector4D>());
		});
		return vec4Box;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QPoint>(), [](QPropertyHandle* InHandler) {
		QPointBox* vec2Box = new QPointBox();
		InHandler->bind(vec2Box, &QPointBox::asValueChanged,
			[vec2Box]() {
				return vec2Box->getValue();
			},
			[vec2Box](QVariant var) {
				vec2Box->setValue(var.value<QPoint>());
			});
		return vec2Box;
	});

	registerCustomPropertyValueWidgetCreator(QMetaType::fromType<QPointF>(), [](QPropertyHandle* InHandler) {
		QPointFBox* vec2Box = new QPointFBox();
		InHandler->bind(vec2Box, &QPointFBox::asValueChanged,
			[vec2Box]() {
				return vec2Box->getValue();
			},
			[vec2Box](QVariant var) {
				vec2Box->setValue(var.value<QPointF>());
			});
		return vec2Box;
		});
}

