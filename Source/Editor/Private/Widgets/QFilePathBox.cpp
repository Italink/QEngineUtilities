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
	mLePath->SetText(inPath);
	mLePath->GetQLineEdit()-> setReadOnly(true);
	connect(mLePath, &QHoverLineEdit::AsTextChanged, this, &QFilePathBox::AsPathChanged);
	connect(mPbOpen, &QPushButton::clicked, this, [this]() {
		QString path = QFileDialog::getOpenFileName(nullptr);
		if (!path.isEmpty())
			mLePath->SetText(path);
	});
}

void QFilePathBox::SetFilePath(QString inPath)
{
	mLePath->SetText(inPath);
}

QString QFilePathBox::GetFilePath()
{
	return mLePath->GetText();
}
