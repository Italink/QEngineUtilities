#include "QDPI_String.h"
#include "QLineEdit"
#include "QTextEdit"
#include "QComboBox"
#include "QJsonArray"
#include "Widgets\QFilePathBox.h"

QWidget* QDPI_String::GenerateValueWidget() {
	const QJsonObject& metaData = GetMetaData();
	QString type = metaData.value("Type").toString();

	if (canConvert<QString>()) {
		if (type == "Line") {
			QLineEdit* lineEdit = new QLineEdit(GetValue().toString());
			lineEdit->setPlaceholderText(metaData.value("PlaceholderText").toString());
			QObject::connect(lineEdit, &QLineEdit::textChanged, this, [this](QString text) {
				SetValue(QVariant::fromValue(text));
			});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			QTextEdit* textEdit = new QTextEdit(GetValue().toString());
			QObject::connect(textEdit, &QTextEdit::textChanged, this, [this, textEdit]() {
				SetValue(textEdit->toPlainText());
			});
			textEdit->setPlaceholderText(metaData.value("PlaceholderText").toString());
			textEdit->setFixedHeight(metaData.value("Height").toDouble());
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox(GetValue().toString());
			QObject::connect(filePathBox, &QFilePathBox::AsPathChanged, this, &QDPI_String::SetValue);
			return filePathBox;
		}
		else if (type == "Combo") {
			QComboBox* comboBox = new QComboBox();
			for (auto item : metaData.value("ComboList").toArray()) {
				comboBox->addItem(item.toString());
			}
			connect(comboBox, &QComboBox::currentTextChanged, this, [this](QString text) {
				SetValue(text);
			});
			comboBox->setMinimumWidth(90);
			return comboBox;
		}
	}
	else if(canConvert<std::string>()) {
		if (type == "Line") {
			QLineEdit* lineEdit = new QLineEdit(QString::fromStdString(GetValue().value<std::string>()));
			lineEdit->setPlaceholderText(metaData.value("PlaceholderText").toString());
			QObject::connect(lineEdit, &QLineEdit::textChanged, this, [this](QString text) {
				SetValue(QVariant::fromValue(text.toStdString()));
			});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			QTextEdit* textEdit = new QTextEdit(QString::fromStdString(GetValue().value<std::string>()));
			QObject::connect(textEdit, &QTextEdit::textChanged, this, [this, textEdit]() {
				SetValue(QVariant::fromValue(textEdit->toPlainText().toStdString()));
			});
			textEdit->setPlaceholderText(metaData.value("PlaceholderText").toString());
			textEdit->setFixedHeight(metaData.value("Height").toDouble());
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox(GetValue().toString());
			QObject::connect(filePathBox, &QFilePathBox::AsPathChanged, this, [this](QString path) {
				SetValue(QVariant::fromValue(path.toStdString()));
			});
			return filePathBox;
		}
		else if (type == "Combo") {
			QComboBox* comboBox = new QComboBox();
			for (auto item : metaData.value("ComboList").toArray()) {
				comboBox->addItem(item.toString());
			}
			connect(comboBox, &QComboBox::currentTextChanged, this, [this](QString text) {
				SetValue(QVariant::fromValue(text.toStdString()));
			});
			comboBox->setMinimumWidth(90);
			return comboBox;
		}
	}
}

