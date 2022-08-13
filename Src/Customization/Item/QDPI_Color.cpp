#include "QDPI_Color.h"
#include "Widgets\Color\QColorButton.hpp"

QWidget* QDPI_Color::GenerateValueWidget() {
	QColorButton* colorButton = new QColorButton(GetValue().value<QColor>());
	GetHandler()->Bind(colorButton, &QColorButton::AsColorChanged,
		[colorButton]() {
			return colorButton->GetColor();
		},
		[colorButton](QVariant var) {
			colorButton->SetColor(var.value<QColor>());
		}
		);
	return colorButton;
}

