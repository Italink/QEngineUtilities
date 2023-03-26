#include "PropertyTypeCustomization_QStaticMesh.h"
#include "Utils/MathUtils.h"
#include "Asset/QStaticMesh.h"
#include <QStackedWidget>
#include "Widgets/QFilePathBox.h"
#include "DetailView/QDetailViewRow.h"

void PropertyTypeCustomization_QStaticMesh::CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	CurrComboBox = new QComboBox();
	CurrComboBox->addItem("File");
	CurrComboBox->addItem("Text");
	CurrComboBox->addItem("Cube");
	Builder->AsNameValueWidget(PropertyHandle->GenerateNameWidget(), CurrComboBox);
}

void PropertyTypeCustomization_QStaticMesh::CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) {
	QFilePathBox* FilePathBox = new QFilePathBox;
	auto FilePathRow = Builder->AddRowByNameValueWidget("Path", FilePathBox)->Row();
	FilePathBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	FilePathRow->GetWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QObject::connect(CurrComboBox, &QComboBox::currentTextChanged, [FilePathRow](const QString& mode) {
		FilePathRow->SetVisible(mode == "File");
	});
}
