#include "QDPI_Bool.h"
#include "QCheckBox"

QWidget* QDPI_Bool::GenerateValueWidget() {
	QCheckBox* checkBox = new QCheckBox;
	checkBox->setCheckState(GetValue().toBool() ? Qt::Checked : Qt::Unchecked);
	QObject::connect(checkBox, &QCheckBox::stateChanged, this, [this](int var) {
		SetValue(var == Qt::Checked);
	});
	return checkBox;
}
