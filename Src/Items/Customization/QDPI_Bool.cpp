#include "QDPI_Bool.h"
#include "QCheckBox"

QWidget* QDPI_Bool::GenerateValueWidget() {
	QCheckBox* checkBox = new QCheckBox;
	GetHandler()->Bind(checkBox, &QCheckBox::stateChanged,
		[checkBox]() {
			return checkBox->checkState() == Qt::Checked;
		},
		[checkBox](QVariant var) {
			checkBox->setCheckState(var.toBool() ? Qt::Checked : Qt::Unchecked);
		}
		);
	return checkBox;
}