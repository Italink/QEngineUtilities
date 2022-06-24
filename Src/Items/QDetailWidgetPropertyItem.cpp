#include "QDetailWidgetPropertyItem.h"
#include "QPushButton"
#include "QDetailWidget.h"
#include "QLabel"
#include "QPainter"
#include "QDetailWidgetManager.h"


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
}

QDetailWidgetPropertyItem* QDetailWidgetPropertyItem::Create(QPropertyHandler* inHandler, QJsonObject inMetaData /*= QJsonObject()*/)
{
	for (auto& filter : QDetailWidgetManager::instance()->GetPropertyItemFilterList()) {
		if (filter.first(inHandler->GetTypeID())) {
			QDetailWidgetPropertyItem* item = filter.second();
			item->SetHandler(inHandler);
			item->mMetaData = inMetaData;
			return item;
		}
	}

	auto iter = QDetailWidgetManager::instance()->GetPropertyItemCreatorMap().find(inHandler->GetTypeID());
	if (iter != QDetailWidgetManager::instance()->GetPropertyItemCreatorMap().end()) {
		QDetailWidgetPropertyItem* item = (*iter)();
		item->SetHandler(inHandler);
		item->mMetaData = inMetaData;
		return item;
	}
	qWarning() << QString("Name: %1, TypeID: %2 : is not registered !").arg(inHandler->GetName()).arg(inHandler->GetTypeID());
	return nullptr;
}

void QDetailWidgetPropertyItem::SetValue(QVariant inValue)
{
	mHandler->SetValue(inValue);
}

QVariant QDetailWidgetPropertyItem::GetValue()
{
	return mHandler->GetValue();
}

void QDetailWidgetPropertyItem::ResetValue()
{
	mHandler->ResetValue();
}

QString QDetailWidgetPropertyItem::GetName()
{
	return GetHandler()->GetName();
}

QString QDetailWidgetPropertyItem::GetKeywords() {
	return GetHandler()->GetName() + QDebug::toString(mHandler->GetValue());
}

void QDetailWidgetPropertyItem::BuildContentAndChildren() {
	mContent->SetNameWidgetByText(GetHandler()->GetName());
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

void QDetailWidgetPropertyItem::SetHandler(QPropertyHandler* inHandler)
{
	mHandler = inHandler;
	QObject::connect(mContent, &QDetailWidgetPropertyItemWidget::AsRequsetReset, inHandler, &QPropertyHandler::ResetValue);
	QObject::connect(inHandler, &QPropertyHandler::AsValueChanged, [this]() {
		RefleshResetButtonStatus();
	});
}

void QDetailWidgetPropertyItem::RefleshResetButtonStatus()
{
	mContent->GetResetButton()->setEnabled(GetHandler()->IsChanged());
}
