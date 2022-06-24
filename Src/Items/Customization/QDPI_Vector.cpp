#include "QDPI_Vector.h"
#include "Widgets\QVectorBox.h"

QWidget* QDPI_Vector::GenerateValueWidget() {
	GetContent()->GetValueContentLayout()->setAlignment(Qt::AlignLeft);
	if (canConvert<QVector4D>()) {
		QVector4DBox* vec4Box = new QVector4DBox(GetValue().value<QVector4D>());
		GetHandler()->Bind(vec4Box, &QVector4DBox::AsValueChanged,
			[vec4Box]() {
				return vec4Box->GetValue();
			},
			[vec4Box](QVariant var) {
				vec4Box->SetValue(var.value<QVector4D>());
			});
		return vec4Box;
	}
	else if (canConvert<QVector3D>()) {
		QVector3DBox* vec3Box = new QVector3DBox(GetValue().value<QVector3D>());
		GetHandler()->Bind(vec3Box, &QVector3DBox::AsValueChanged,
			[vec3Box]() {
				return vec3Box->GetValue();
			},
			[vec3Box](QVariant var) {
				vec3Box->SetValue(var.value<QVector3D>());
			});
		return vec3Box;
	}
	else if (canConvert<QVector2D>()) {
		QVector2DBox* vec2Box = new QVector2DBox(GetValue().value<QVector2D>());
		GetHandler()->Bind(vec2Box, &QVector2DBox::AsValueChanged,
			[vec2Box]() {
				return vec2Box->GetValue();
			},
			[vec2Box](QVariant var) {
				vec2Box->SetValue(var.value<QVector2D>());
			});
		return vec2Box;
	}
	return nullptr;
}

