#include "PropertyTypeCustomization_QStaticMesh.h"
#include "Utils/MathUtils.h"
#include "Asset/QStaticMesh.h"
#include <QStackedWidget>
#include "Widgets/QFilePathBox.h"
#include "DetailView/QDetailViewRow.h"

QSharedPointer<IStaticMeshCreator> PropertyTypeCustomization_QStaticMesh::mCreator;

void PropertyTypeCustomization_QStaticMesh::CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	CurrComboBox = new QComboBox();
	static QMap<QString, const QMetaObject*> CreatorMap = {
		{"None",nullptr},
		{"File",&QStaticMeshCreator_FromFile::staticMetaObject},
		{"Text",&QStaticMeshCreator_FromText::staticMetaObject},
		{"Cube",&QStaticMeshCreator_FromCube::staticMetaObject},
		{"Sphere",&QStaticMeshCreator_FromSphere::staticMetaObject},
		{"Grid",&QStaticMeshCreator_FromGrid::staticMetaObject},
	};

	CurrComboBox->addItems({"None","File","Text","Cube","Sphere","Grid"});

	if (mCreator) {
		for (auto keyValue : CreatorMap.asKeyValueRange()) {
			if (keyValue.second == mCreator->metaObject()) {
				CurrComboBox->setCurrentText(keyValue.first);
				break;
			}
		}
		QObject::connect(mCreator.get(), &IStaticMeshCreator::AsCreateMesh, [PropertyHandle](QSharedPointer<QStaticMesh> mesh) {
			PropertyHandle->SetValue(QVariant::fromValue<>(mesh), "Create Static Mesh");
		});
	}
	else {
		CurrComboBox->setCurrentText("None");
	}
	QObject::connect(CurrComboBox, &QComboBox::currentTextChanged, [PropertyHandle](const QString& text) {
		const QMetaObject* metaObj = CreatorMap.value(text);
		if (metaObj) {
			mCreator.reset((IStaticMeshCreator*)metaObj->newInstance());
			PropertyHandle->SetValue(QVariant::fromValue<>(mCreator->create()),"Create Static Mesh");
		}
		else {
			mCreator.reset();
		}
		Q_EMIT PropertyHandle->AsRequestRebuildRow();
	});

	Builder->AsNameValueWidget(PropertyHandle->GenerateNameWidget(), CurrComboBox);
}

void PropertyTypeCustomization_QStaticMesh::CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) {
	Builder->AddObject(mCreator.get());
	//QFilePathBox* FilePathBox = new QFilePathBox;
	//auto FilePathRow = Builder->AddRowByNameValueWidget("Path", FilePathBox)->Row();
	//FilePathBox->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Expanding);
	//FilePathRow->GetWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//QObject::connect(CurrComboBox, &QComboBox::currentTextChanged, [FilePathRow](const QString& mode) {
	//	FilePathRow->SetVisible(mode == "File");
	//});
}

