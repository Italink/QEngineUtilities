#include "QDPI_Color.h"
#include "Widgets\Color\QColorButton.hpp"

QWidget* QDPI_Color::GenerateValueWidget() {
	QColorButton* colorButton = new QColorButton(GetValue().value<QColor>());
	connect(colorButton, &QColorButton::AsColorChanged, this, &QDPI_Color::SetValue);
	return colorButton;
}

