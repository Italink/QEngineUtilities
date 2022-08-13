#include "QDPI_String.h"
#include "QComboBox"
#include "QJsonArray"
#include "Widgets\QFilePathBox.h"
#include "Widgets\Toolkits\QHoverLineEdit.h"
#include "Widgets\Toolkits\QHoverTextEdit.h"

QWidget* QDPI_String::GenerateValueWidget() {
	QString type = GetMetaData("Type").toString();
	if (canConvert<QString>()) {
		if (type.isEmpty()|| type == "Line") {
			QHoverLineEdit* lineEdit = new QHoverLineEdit(GetValue().toString());
			lineEdit->SetPlaceholdText(GetMetaData("PlaceholderText").toString());
			GetHandler()->Bind(lineEdit, &QHoverLineEdit::AsTextChanged,
				[lineEdit]() {
					return  lineEdit->GetText();
				},
				[lineEdit](QVariant var) {
					lineEdit->SetText(var.toString());
				});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			QHoverTextEdit* textEdit = new QHoverTextEdit(GetValue().toString());
			GetHandler()->Bind(textEdit, &QHoverTextEdit::AsTextChanged,
				[textEdit]() {
					return  textEdit->GetText();
				},
				[textEdit](QVariant var) {
					textEdit->SetText(var.toString());
				});
			textEdit->SetPlaceholdText(GetMetaData("PlaceholderText").toString());
			textEdit->setFixedHeight(GetMetaData("Height").toDouble());
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox(GetValue().toString());
			GetHandler()->Bind(filePathBox, &QFilePathBox::AsPathChanged,
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
			for (auto item : GetMetaData("ComboList").toStringList()) {
				comboBox->addItem(item);
			}
			GetHandler()->Bind(comboBox, &QComboBox::currentTextChanged,
				[comboBox]() {
					return comboBox->currentText();
				},
				[comboBox](QVariant var) {
					comboBox->setCurrentText(var.toString());
				});

			comboBox->setMinimumWidth(90);
			return comboBox;
		}
	}
	else if(canConvert<std::string>()) {
		if (type == "Line") {
			QHoverLineEdit* lineEdit = new QHoverLineEdit(QString::fromStdString(GetValue().value<std::string>()));
			lineEdit->SetPlaceholdText(GetMetaData("PlaceholderText").toString());
			//QObject::connect(lineEdit, &QHoverLineEdit::AsTextChanged, this, [this](QString text) {
			//	SetValue(QVariant::fromValue(text.toStdString()));
			//});
			return lineEdit;
		}
		else if (type == "MultiLine") {
			QTextEdit* textEdit = new QTextEdit(QString::fromStdString(GetValue().value<std::string>()));
			//QObject::connect(textEdit, &QTextEdit::textChanged, this, [this, textEdit]() {
			//	SetValue(QVariant::fromValue(textEdit->toPlainText().toStdString()));
			//});
			textEdit->setPlaceholderText(GetMetaData("PlaceholderText").toString());
			textEdit->setFixedHeight(GetMetaData("Height").toDouble());
			return textEdit;
		}
		else if (type == "FilePath") {
			QFilePathBox* filePathBox = new QFilePathBox(GetValue().toString());
			//QObject::connect(filePathBox, &QFilePathBox::AsPathChanged, this, [this](QString path) {
			//	SetValue(QVariant::fromValue(path.toStdString()));
			//});
			return filePathBox;
		}
		else if (type == "Combo") {
			QComboBox* comboBox = new QComboBox();
			for (auto item : GetMetaData("ComboList").toStringList()) {
				comboBox->addItem(item);
			}
			//connect(comboBox, &QComboBox::currentTextChanged, this, [this](QString text) {
			//	SetValue(QVariant::fromValue(text.toStdString()));
			//});
			comboBox->setMinimumWidth(90);
			return comboBox;
		}
	}
	return nullptr;
}