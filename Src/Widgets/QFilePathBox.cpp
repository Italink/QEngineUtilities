#include "QFilePathBox.h"
#include "QLineEdit"
#include "QPushButton"
#include "QBoxLayout"
#include "QFileDialog"
QFilePathBox::QFilePathBox(QString inPath) 
	: mLePath(new QLineEdit)
	, mPbOpen(new QPushButton("Open"))
{
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mLePath);
	h->addWidget(mPbOpen);
	h->setContentsMargins(0, 0, 0, 0);
	mLePath->setText(inPath);
	mLePath->setReadOnly(true);
	connect(mLePath, &QLineEdit::textChanged, this, &QFilePathBox::AsPathChanged);
	connect(mPbOpen, &QPushButton::clicked, this, [this]() {
		QString path = QFileDialog::getOpenFileName(nullptr);
		if (!path.isEmpty())
			mLePath->setText(path);
	});
}
