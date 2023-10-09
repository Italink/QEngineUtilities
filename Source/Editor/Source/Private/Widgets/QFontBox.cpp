#include "Widgets/QFontBox.h"

#include <QFontComboBox>
#include <QHBoxLayout>
#include "Widgets/QElideLabel.h"

QFontBox::QFontBox(QFont value /*= QFont()*/) 
	: mComboBox(new QFontComboBox)
	, mPointSize(new QNumberBox(value.pointSize(), true, 0, 500))
	, mWeight(new QNumberBox(value.pointSize(), true, 0, 500))
	, mItalic(new QCheckBox())
{
	mItalic->setCheckState(value.italic() ? Qt::Checked : Qt::Unchecked);
	mComboBox->setEditable(false);

	QVBoxLayout* layout = new QVBoxLayout(this);

	QHBoxLayout* row = new QHBoxLayout();
	row->addWidget(new QElideLabel("Family"));
	row->addWidget(mComboBox);
	layout->addLayout(row);

	row = new QHBoxLayout();
	row->addWidget(new QElideLabel("PointSize"));
	row->addWidget(mPointSize);
	layout->addLayout(row);

	row = new QHBoxLayout();
	row->addWidget(new QElideLabel("Weight"));
	row->addWidget(mWeight);
	layout->addLayout(row);

	row = new QHBoxLayout();
	row->addWidget(new QElideLabel("Italic"));
	row->addWidget(mItalic);
	layout->addLayout(row);

	mComboBox->setCurrentFont(value);

	connect(mComboBox, &QFontComboBox::currentFontChanged, this, [this](const QFont& font) {
		emit asFontChanged(getFont());
	});
	connect(mPointSize, &QNumberBox::asValueChanged, this, [this](QVariant) {
		emit asFontChanged(getFont());
	});
	connect(mWeight, &QNumberBox::asValueChanged, this, [this](QVariant) {
		emit asFontChanged(getFont());
	});
	connect(mItalic, &QCheckBox::stateChanged, this, [this](int) {
		emit asFontChanged(getFont());
	});
}

QFont QFontBox::getFont() {
	QFont font = mComboBox->currentFont();
	font.setPointSize(mPointSize->getVar().toInt());
	font.setLegacyWeight(mWeight->getVar().toInt());
	font.setItalic(mItalic->isChecked());
	return font;
}

void QFontBox::setFont(const QFont& inFont) {
	if (mComboBox->currentFont() != inFont) {
		mComboBox->setCurrentFont(inFont);
		Q_EMIT asFontChanged(inFont);
	}
}
