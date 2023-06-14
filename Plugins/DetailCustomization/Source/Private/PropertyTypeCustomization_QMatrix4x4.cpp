#include "PropertyTypeCustomization_QMatrix4x4.h"
#include "QMatrix4x4"
#include "Utils/MathUtils.h"

void PropertyTypeCustomization_QMatrix4x4::CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	Builder->AsNameValueWidget(PropertyHandle->GenerateNameWidget(), PropertyHandle->GenerateValueWidget());
}

void PropertyTypeCustomization_QMatrix4x4::CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) {
	QPropertyHandle* position = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->GetSubPath("Position"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatTranslate(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			MathUtils::setMatTranslate(Mat, var.value<QVector3D>());
			PropertyHandle->SetValue(Mat);
		}
	);
	Builder->AddProperty(position);

	QPropertyHandle* rotation = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->GetSubPath("Rotation"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatRotation(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			MathUtils::setMatRotation(Mat, var.value<QVector3D>());
			PropertyHandle->SetValue(Mat);
		}
		);
	Builder->AddProperty(rotation);

	QPropertyHandle* scale = QPropertyHandle::FindOrCreate(
		PropertyHandle->parent(),
		QMetaType::fromType<QVector3D>(),
		PropertyHandle->GetSubPath("Scale"),
		[PropertyHandle]() {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatScale3D(Mat);
		},
		[PropertyHandle](QVariant var) {
			QMatrix4x4 Mat = PropertyHandle->GetValue().value<QMatrix4x4>();
			MathUtils::setMatScale3D(Mat, var.value<QVector3D>());
			PropertyHandle->SetValue(Mat);
		}
		);
	Builder->AddProperty(scale);
}
