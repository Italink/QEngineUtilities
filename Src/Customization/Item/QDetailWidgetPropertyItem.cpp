#include "QDetailWidgetPropertyItem.h"

#include "QLabel"
#include "QPainter"
#include "QPushButton"

#include "Core\QDetailWidgetPrivate.h"
#include "Customization\QDetailWidgetManager.h"
#include "Widgets/Toolkits/QNameLabel.h"
#include "Widgets/Toolkits/QSvgIcon.h"


class QDetailWidgetPropertyResetButton :public QPushButton {
public:
	QDetailWidgetPropertyResetButton() {
		setFocusPolicy(Qt::NoFocus);
	}
protected:
	virtual void paintEvent(QPaintEvent*) {
		QPainter painter(this);
		if (isEnabled()) {
			mIcon.getIcon().paint(&painter, rect().adjusted(5, 5, -5, -5));
		}
	}
	QSvgIcon mIcon = QSvgIcon(":/Resources/reset.png");
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
	mValueContentLayout->setSpacing(2);
	mResetButton->setFixedSize(25,25);
	mResetButton->setEnabled(false);
	setMinimumHeight(25);
	addWidget(mNameContent);
	addWidget(mValueContent);
	addWidget(mResetButton);
	connect(mResetButton, &QPushButton::clicked, this, &QDetailWidgetPropertyItemWidget::AsRequsetReset);
}

void QDetailWidgetPropertyItemWidget::RefleshSplitterFactor() {
	QDetailTreeWidget* widget = static_cast<QDetailTreeWidget*>(mRow->treeWidget());
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
	if (mNameWidget == nullptr ) {
		if (inWidget)
			mNameContentLayout->addWidget(inWidget);
	}
	else {
		if (inWidget)
			mNameContentLayout->replaceWidget(mNameWidget, inWidget);
		mNameWidget->setParent(nullptr);
		mNameWidget->deleteLater();
	}
	mNameWidget = inWidget;
}

void QDetailWidgetPropertyItemWidget::SetNameWidgetByText(QString inName) {
	QNameLabel* nameEditor = new QNameLabel(inName);
	connect(nameEditor, &QNameLabel::AsNameChanged, this, [this,nameEditor]() {
		auto callback = mRow->GetRenameCallback();
		bool accepted = false;
		if (callback) {
			accepted = callback(nameEditor->GetText());
		}
		if (!accepted) {
			nameEditor->SetText(mRow->GetName());
		}
	});
	SetNameWidget(nameEditor);
}

void QDetailWidgetPropertyItemWidget::ClearValueAttachWidget() {
	while (auto item = mValueContentLayout->takeAt(1)) {
		if (item->widget()) {
			item->widget()->setParent(nullptr);
			item->widget()->deleteLater();
		}
	}
}

void QDetailWidgetPropertyItemWidget::AddValueWidget(QWidget* inWidget) {
	if(inWidget)
		mValueContentLayout->addWidget(inWidget);
}

QHBoxLayout* QDetailWidgetPropertyItemWidget::GetNameContentLayout() const {
	return mNameContentLayout;
}

QHBoxLayout* QDetailWidgetPropertyItemWidget::GetValueContentLayout() const {
	return mValueContentLayout;
}

void QDetailWidgetPropertyItemWidget::ShowRenameEditor()
{
	QNameLabel* nameEdit = qobject_cast<QNameLabel*>(mNameWidget);
	if (nameEdit) {
		nameEdit->ShowRenameEditor();
	}
}

void QDetailWidgetPropertyItemWidget::resizeEvent(QResizeEvent* event) {
	QSplitter::resizeEvent(event);
	RefleshSplitterFactor();
}

QDetailWidgetPropertyItem::QDetailWidgetPropertyItem()
	: mContent(new QDetailWidgetPropertyItemWidget(this))
{
	QObject::connect(mContent, &QSplitter::splitterMoved, [this](int pos, int index) {
		QDetailTreeWidget* widget = static_cast<QDetailTreeWidget*>(treeWidget());
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

QDetailWidgetPropertyItem* QDetailWidgetPropertyItem::Create(QPropertyHandler* inHandler)
{
	for (auto& filter : QDetailWidgetManager::instance()->GetPropertyItemFilterList()) {
		if (filter.first(inHandler->GetTypeID())) {
			QDetailWidgetPropertyItem* item = filter.second();
			item->SetHandler(inHandler);
			return item;
		}
	}

	auto iter = QDetailWidgetManager::instance()->GetPropertyItemCreatorMap().find(inHandler->GetTypeID());
	if (iter != QDetailWidgetManager::instance()->GetPropertyItemCreatorMap().end()) {
		QDetailWidgetPropertyItem* item = (*iter)();
		item->SetHandler(inHandler);
		return item;
	}
	qWarning() << QString("Name: %1, TypeID: %2 : is not registered !").arg(inHandler->GetName()).arg(inHandler->GetTypeID());
	return nullptr;
}

void QDetailWidgetPropertyItem::SetValue(QVariant inValue, QString isPushUndoStackWithDesc /*= QString()*/) {
	mHandler->SetValue(inValue, isPushUndoStackWithDesc);
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

void QDetailWidgetPropertyItem::RequestRename()
{
	if (CanRename())
		mContent->ShowRenameEditor();
}

void QDetailWidgetPropertyItem::SetBuildContentAndChildrenCallback(std::function<void()> val) {
	mBuildContentAndChildrenCallback = val;
}

QString QDetailWidgetPropertyItem::GetKeywords() {
	return GetHandler()->GetName() + QDebug::toString(mHandler->GetValue());
}

void QDetailWidgetPropertyItem::BuildContentAndChildren() {
	mContent->SetNameWidgetByText(GetHandler()->GetName());
	if (mValueWidget==nullptr) {
		mValueWidget = GenerateValueWidget();
		AddValueWidget(mValueWidget);
	}
	RebuildAttachWidget();
	treeWidget()->setItemWidget(this, 0, mContent);
}

void QDetailWidgetPropertyItem::RebuildAttachWidget() {
	mContent->ClearValueAttachWidget();
	if (mBuildContentAndChildrenCallback)
		mBuildContentAndChildrenCallback();
}

void QDetailWidgetPropertyItem::AddValueWidget(QWidget* inWigdet) {
	mContent->AddValueWidget(inWigdet);
}

void QDetailWidgetPropertyItem::AddValueLayout(QLayout* inLayout) {
	mContent->GetValueContentLayout()->addLayout(inLayout);
}

void QDetailWidgetPropertyItem::BuildMenu(QMenu& inMenu)
{
	inMenu.addAction("Copy");
	inMenu.addAction("Paste");
	if(CanRename())
		inMenu.addAction("Rename", [this]() {  RequestRename(); });
}

QDetailWidgetPropertyItemWidget* QDetailWidgetPropertyItem::GetContent() const {
	return mContent;
}

void QDetailWidgetPropertyItem::RefleshSplitterFactor() {
	mContent->RefleshSplitterFactor();
}

const QVariantHash& QDetailWidgetPropertyItem::GetMetaData() const {
	return mHandler->GetMetaData();
}

QVariant QDetailWidgetPropertyItem::GetMetaData(const QString& Key)
{
	return mHandler->GetMetaData(Key);
}

void QDetailWidgetPropertyItem::SetHandler(QPropertyHandler* inHandler)
{
	if (mHandler != inHandler) {
		mHandler = inHandler;
		QObject::connect(mContent, &QDetailWidgetPropertyItemWidget::AsRequsetReset, this, &QDetailWidgetPropertyItem::ResetValue);
		QObject::connect(inHandler, &QPropertyHandler::AsValueChanged, this, &QDetailWidgetPropertyItem::RefleshResetButtonStatus);
		if (QTreeWidgetItem::parent() != nullptr) {
			BuildContentAndChildren();
		}
	}
	else {
		RebuildAttachWidget();
	}
}


void QDetailWidgetPropertyItem::RefleshResetButtonStatus()
{
	mContent->GetResetButton()->setEnabled(GetHandler()->IsChanged());
}
