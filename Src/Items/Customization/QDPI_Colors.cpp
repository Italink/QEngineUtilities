#include "QDPI_Colors.h"
#include "Widgets\Color\QColorsButton.hpp"

QWidget* QDPI_Colors::GenerateValueWidget() {
	QColorsButton* colorsButton = new QColorsButton(GetValue().value<QColors>());
	//connect(colorsButton, &QColorsButton::AsColorsChanged, this, [this](QColors colors) {
	//	SetValue(QVariant::fromValue(colors));
	//});
	return colorsButton;
}

