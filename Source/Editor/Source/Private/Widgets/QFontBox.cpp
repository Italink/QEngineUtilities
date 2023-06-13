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
		emit AsFontChanged(GetFont());
	});
	connect(mPointSize, &QNumberBox::AsValueChanged, this, [this](QVariant) {
		emit AsFontChanged(GetFont());
	});
	connect(mWeight, &QNumberBox::AsValueChanged, this, [this](QVariant) {
		emit AsFontChanged(GetFont());
	});
	connect(mItalic, &QCheckBox::stateChanged, this, [this](int) {
		emit AsFontChanged(GetFont());
	});
}

QFont QFontBox::GetFont() {
	QFont font = mComboBox->currentFont();
	font.setPointSize(mPointSize->GetVar().toInt());
	font.setLegacyWeight(mWeight->GetVar().toInt());
	font.setItalic(mItalic->isChecked());
	return font;
}

void QFontBox::SetFont(const QFont& inFont) {
	if (mComboBox->currentFont() != inFont) {
		mComboBox->setCurrentFont(inFont);
		Q_EMIT AsFontChanged(inFont);
	}
}
