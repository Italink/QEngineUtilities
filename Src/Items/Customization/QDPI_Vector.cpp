#include "QDPI_Vector.h"
#include "Widgets\QVectorBox.h"

QWidget* QDPI_Vector::GenerateValueWidget() {
	GetContent()->GetValueContentLayout()->setAlignment(Qt::AlignLeft);
	if (canConvert<QVector4D>()) {
		QVector4DBox* vec4Box = new QVector4DBox(GetValue().value<QVector4D>());
		connect(vec4Box, &QVector4DBox::AsValueChanged, this, &QDPI_Vector::SetValue);
		return vec4Box;
	}
	else if (canConvert<QVector3D>()) {
		QVector3DBox* vec3Box = new QVector3DBox(GetValue().value<QVector3D>());
		connect(vec3Box, &QVector3DBox::AsValueChanged, this, &QDPI_Vector::SetValue);
		return vec3Box;
	}
	else if (canConvert<QVector2D>()) {
		QVector2DBox* vec2Box = new QVector2DBox(GetValue().value<QVector2D>());
		connect(vec2Box, &QVector2DBox::AsValueChanged, this, &QDPI_Vector::SetValue);
		return vec2Box;
	}
	
}

