#include "QDetailWidgetPropertyItem.h"
#include "QPushButton"
#include "QDetailWidget.h"
#include "QLabel"
#include "QPainter"


class QDetailWidgetPropertyResetButton :public QPushButton {
protected:
	virtual void paintEvent(QPaintEvent*) {
		QPainter painter(this);
		if (isEnabled()) {
			painter.fillRect(rect(), Qt::red);
		}
	}
};


QDetailWidgetPropertyItemWidget::QDetailWidgetPropertyItemWidget(QDetailWidgetPropertyItem* inRow) 
	: mRow(inRow)
	, mNameContent(new QWidget)
	, mNameContentLayout(new QHBoxLayout)
	, mValueContent(new QWidget)
	, mValueContentLayout(new QHBoxLayout)
	, mResetButton(new QDetailWidgetPropertyResetButton) {
	setHandleWidth(1);
	mNameContent->setLayout(mNameContentLayout);
	mNameContent->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);
	mNameContentLayout->setAlignment(Qt::AlignLeft);
	mNameContentLayout->setContentsMargins(2, 1, 2, 1);
	mValueContent->setLayout(mValueContentLayout);
	mValueContent->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);
	mValueContentLayout->setAlignment(Qt::AlignCenter);
	mValueContentLayout->setContentsMargins(10, 2, 10, 2);
	mResetButton->setMinimumWidth(30);
	mResetButton->setEnabled(false);
	addWidget(mNameContent);
	addWidget(mValueContent);
	addWidget(mResetButton);
	connect(mResetButton, &QPushButton::clicked, this, &QDetailWidgetPropertyItemWidget::AsRequsetReset);
}

void QDetailWidgetPropertyItemWidget::RefleshSplitterFactor() {
	QDetailWidget* widget = dynamic_cast<QDetailWidget*>(mRow->treeWidget());
	if (widget) {
		blockSignals(true);
		QList<int> sizes = widget->GetSplitterSizes();
		sizes[1] = (widget->width() - sizes[2]) * (sizes[1] / float(sizes[0] + sizes[1]));
		sizes[0] = width() - sizes[1] - sizes[2];
		setSizes(sizes);
		blockSignals(false);
	}
}

void QDetailWidgetPropertyItemWidget::SetNameWidget(QWidget* inWidget) {
	if (mNameWidget == nullptr) {
		mNameContentLayout->addWidget(inWidget);
	}
	else {
		mNameContentLayout->replaceWidget(mNameWidget, inWidget);
	}
	mNameWidget = inWidget;
}

void QDetailWidgetPropertyItemWidget::SetNameWidgetByText(QString inName) {
	SetNameWidget(new QLabel(inName));
}

void QDetailWidgetPropertyItemWidget::SetValueWidget(QWidget* inWidget) {
	if (mValueWidget == nullptr) {
		mValueContentLayout->addWidget(inWidget);
	}
	else {
		mValueContentLayout->replaceWidget(mValueWidget, inWidget);
	}
	mValueWidget = inWidget;
}

QHBoxLayout* QDetailWidgetPropertyItemWidget::GetNameContentLayout() const {
	return mNameContentLayout;
}

QHBoxLayout* QDetailWidgetPropertyItemWidget::GetValueContentLayout() const {
	return mValueContentLayout;
}

void QDetailWidgetPropertyItemWidget::resizeEvent(QResizeEvent* event) {
	QSplitter::resizeEvent(event);
	RefleshSplitterFactor();
}

QDetailWidgetPropertyItem::QDetailWidgetPropertyItem()
	: mContent(new QDetailWidgetPropertyItemWidget(this))
{
	QObject::connect(mContent, &QSplitter::splitterMoved, [this](int pos, int index) {
		QDetailWidget* widget = dynamic_cast<QDetailWidget*>(treeWidget());
		if (widget) {
			QList<int> sizes = widget->GetSplitterSizes();
			if (index == 2) {
				sizes[2] = mContent->width() - pos;
				sizes[1] = mContent->width() - sizes[0] - sizes[2];
			}
			if (index == 1) {
				sizes[1] = mContent->width() - pos - sizes[2];
				sizes[0] = mContent->width() - sizes[1] - sizes[2];
			}
			if (sizes[0] > 0 && sizes[1] > 0 && sizes[2] > 0) {
				widget->SetSplitterSizes(sizes[0], sizes[1], sizes[2]);
			}
			else {
				RefleshSplitterFactor();
			}
		}
	});
	connect(mContent, &QDetailWidgetPropertyItemWidget::AsRequsetReset, this, &QDetailWidgetPropertyItem::ResetValue);
}

void QDetailWidgetPropertyItem::Initialize(TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter, QJsonObject inMetaData) {
	mTypeID = inTypeID;
	mName = inName;
	mGetter = inGetter;
	mSetter = inSetter;
	mInitialValue = inGetter();
	mMetaData = inMetaData;
}

void QDetailWidgetPropertyItem::SetValue(QVariant value)
{
	mIsChanged = (value != mInitialValue);
	RefleshResetButtonStatus();
	mSetter(value);
}

QVariant QDetailWidgetPropertyItem::GetValue()
{
	return mGetter();
}

void QDetailWidgetPropertyItem::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue);
	}
}

QDetailWidgetPropertyItem::TypeId QDetailWidgetPropertyItem::GetTypeID() {
	return mTypeID;
}

QString QDetailWidgetPropertyItem::GetName()
{
	return mName;
}

QString QDetailWidgetPropertyItem::GetKeywords() {
	return mName + QDebug::toString(GetValue());
}

void QDetailWidgetPropertyItem::BuildContentAndChildren() {
	mContent->SetNameWidgetByText(mName);
	mContent->SetValueWidget(GenerateValueWidget());
	treeWidget()->setItemWidget(this, 0, mContent);
}

QDetailWidgetPropertyItemWidget* QDetailWidgetPropertyItem::GetContent() const {
	return mContent;
}

void QDetailWidgetPropertyItem::RefleshSplitterFactor() {
	mContent->RefleshSplitterFactor();
}

const QJsonObject& QDetailWidgetPropertyItem::GetMetaData() const {
	return mMetaData;
}

void QDetailWidgetPropertyItem::RefleshResetButtonStatus()
{
	mContent->GetResetButton()->setEnabled(mIsChanged);
}
