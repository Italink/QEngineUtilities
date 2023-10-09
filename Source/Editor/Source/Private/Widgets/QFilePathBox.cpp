#include "Widgets/QFilePathBox.h"
#include "QPushButton"
#include "QBoxLayout"
#include "QFileDialog"
#include "Widgets/QHoverLineEdit.h"

QFilePathBox::QFilePathBox(QString inPath) 
	: mLePath(new QHoverLineEdit)
	, mPbOpen(new QPushButton("Open"))
{
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mLePath);
	h->addWidget(mPbOpen);
	h->setContentsMargins(0, 0, 0, 0);
	mLePath->setDisplayText(inPath);
	mLePath->GetQLineEdit()-> setReadOnly(true);
	connect(mLePath, &QHoverLineEdit::asTextChanged, this, &QFilePathBox::asPathChanged);
	connect(mPbOpen, &QPushButton::clicked, this, [this]() {
		QString path = QFileDialog::getOpenFileName(nullptr);
		if (!path.isEmpty())
			mLePath->setDisplayText(path);
	});
}

void QFilePathBox::setFilePath(QString inPath)
{
	mLePath->setDisplayText(inPath);
}

QString QFilePathBox::getFilePath()
{
	return mLePath->getDisplayText();
}
