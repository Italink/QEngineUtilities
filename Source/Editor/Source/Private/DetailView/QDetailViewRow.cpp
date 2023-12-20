#include "DetailView/QDetailViewRow.h"
#include "DetailView/QPropertyHandle.h"
#include "QEngineEditorStyleManager.h"
#include "QPainter"
#include "Widgets/QSvgIcon.h"
#include "DetailView/QDetailView.h"
#include "QSplitter"
#include "qevent.h"
#include "Widgets/QElideLabel.h"
#include "Widgets/QHoverWidget.h"

const int INDENT_WIDTH = 10;
const int INDENT_SHADOW_WIDTH = 6;
const int ICON_SIZE = 15;
const int NAME_WIDGET_MIN_WIDTH = 10;

class QDetailViewRowIndentWidget: public QWidget {
	Q_OBJECT
public:
	enum ExpandState {
		None,
		Expand,
		Unexpand,
	};
	void FixupWidth() {
		setFixedWidth(INDENT_WIDTH * mLevel + ICON_SIZE);
	}
	void RefreshState(bool HasChild, bool bExpanded) {
		ExpandState newState;
		if (!HasChild) {
			newState = QDetailViewRowIndentWidget::None;
		}
		else{
			newState = bExpanded ? QDetailViewRowIndentWidget::Expand : QDetailViewRowIndentWidget::Unexpand;
		}
		if (newState != mState) {
			mState = newState;
			update();
		}	
	}
Q_SIGNALS:
	void AsToggledExpand();
protected:
	void paintEvent(QPaintEvent* event) override {
		QPainter painter(this);
		QRect iconRect(0, (height() - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);
		iconRect.moveLeft(INDENT_WIDTH * mLevel);
		if (mState == Expand) {
			ExpandIcon.getIcon().paint(&painter, iconRect);
		}
		else if (mState == Unexpand) {
			UnexpandIcon.getIcon().paint(&painter, iconRect);
		}
		QColor shadowColor = QEngineEditorStyleManager::Instance()->getShadowColor();
		QRect shadowRect(0, 0, INDENT_WIDTH, rect().height());
		if(mLevel > 0){
			QLinearGradient shadowLinearGradient;
			shadowLinearGradient.setColorAt(0, QColor(shadowColor.red(), shadowColor.green(), shadowColor.blue(), 0));
			shadowLinearGradient.setColorAt(1, QColor(shadowColor.red(), shadowColor.green(), shadowColor.blue()));
			for (int i = 0; i < mLevel; i++) {
				shadowRect.moveRight((i + 1) * INDENT_WIDTH);
				shadowLinearGradient.setStart(shadowRect.topLeft());
				shadowLinearGradient.setFinalStop(shadowRect.topRight());
				painter.fillRect(shadowRect, shadowLinearGradient);
			}
		}
	}
	void mousePressEvent(QMouseEvent* event) override {
		Q_EMIT AsToggledExpand();
		event->accept();
	}

public:
	int mLevel = 0;
	ExpandState mState = None;
	QSvgIcon ExpandIcon = QSvgIcon(":/Resources/expand.png","DetailView");
	QSvgIcon UnexpandIcon = QSvgIcon(":/Resources/unexpand.png", "DetailView");
};

class QDetailViewRowWidget: public QHoverWidget {
	Q_OBJECT
	using QHoverWidget::QHoverWidget;
public:
	QDetailViewRowWidget(QDetailViewRow* InRow)
		: mRow(InRow)
		, mLayout(new QHBoxLayout(this))
		, mContentWidget(new QWidget)
		, mIndentWidget(new QDetailViewRowIndentWidget) {
		setAttribute(Qt::WA_TranslucentBackground);
		mContentWidget->setAttribute(Qt::WA_TranslucentBackground);
		mLayout->setContentsMargins(0, 0, 0, 0);
		mLayout->setSpacing(0);
		mLayout->addWidget(mIndentWidget);
		mLayout->addSpacing(5);
		mLayout->addWidget(mContentWidget);
		connect(mIndentWidget, &QDetailViewRowIndentWidget::AsToggledExpand, this, &QDetailViewRowWidget::AsToggledExpand);
	}
	void SetContentWidget(QWidget* inWidget) {
		if (inWidget == nullptr)
			inWidget = new QWidget;
		inWidget->setAttribute(Qt::WA_TranslucentBackground);
		mLayout->replaceWidget(mContentWidget, inWidget);
		if (mContentWidget)
			mContentWidget->deleteLater();
		mContentWidget = inWidget;
	}
Q_SIGNALS:
	void AsToggledExpand();
	void AsShowEvent();
protected:
	void mousePressEvent(QMouseEvent* event) override {
		if (event->button() == Qt::LeftButton) {
			mRow->mView->setCurrentRow(mRow);
		}
	}

	void mouseDoubleClickEvent(QMouseEvent* event) override{
		Q_EMIT AsToggledExpand();
		event->accept();
	}
	void paintEvent(QPaintEvent *event){
		QPainter painter(this);
		if(mRow->isCategory()){
			painter.fillRect(rect(), QEngineEditorStyleManager::Instance()->getCategoryColor());
		}
		else if (mRow->isCurrent()) {
			painter.fillRect(rect(), QEngineEditorStyleManager::Instance()->getSelectedColor());
		}
		QPen pen(QEngineEditorStyleManager::Instance()->getGridLineColor());
		pen.setWidth(1);
		painter.setPen(pen);
		if (mHoverd)
			painter.setBrush(QEngineEditorStyleManager::Instance()->getHoveredColor());
		else
			painter.setBrush(Qt::NoBrush);
		painter.drawRect(rect());

		if (mRow->bNeedRefreshSplitter) {
			mRow->refreshSplitter();
			mRow->bNeedRefreshSplitter = false;
			update();
		}
	}
	void showEvent(QShowEvent *event) override{
		Q_EMIT AsShowEvent();
		event->accept();
	}
public:
	QDetailViewRow* mRow = nullptr;
	QHBoxLayout* mLayout = nullptr;
	QDetailViewRowIndentWidget* mIndentWidget = nullptr;
	QWidget* mContentWidget = nullptr;
	bool bExpanded = false;
};

QDetailViewRow::QDetailViewRow(QDetailView* inView)
	: mView(inView)
	, mWidget(new QDetailViewRowWidget(this))
{
	connect(mWidget, &QDetailViewRowWidget::AsToggledExpand, this, [this]() {
		setExpanded(!bExpanded);
	});
	connect(mWidget, &QDetailViewRowWidget::AsShowEvent, this, [this]() {
		requestRefreshSplitter();
	});
}

QDetailViewRow::~QDetailViewRow()
{
}

void QDetailViewRow::setupContentWidget(QWidget* inContent) {
	mWidget->SetContentWidget(inContent);
	if (mView->isVisible()) {
		inContent->show();
	}
}

void QDetailViewRow::setupNameValueWidget(QWidget* inNameWidget, QWidget* inValueWidget) {
	QSplitter* content = new QSplitter;
	content->setHandleWidth(2);
	content->setChildrenCollapsible(false);
	QObject::connect(content, &QSplitter::splitterMoved,this, [this, content](int pos, int index) {
		mView->mValueWidgetWidth = qMax(1, content->width() - content->handleWidth() - pos);
		mView->refreshRowsSplitter();
	});
	if (inNameWidget) {
		inNameWidget->setAttribute(Qt::WA_TranslucentBackground);
		content->addWidget(inNameWidget);
	}
	if (inValueWidget) {
		QSizePolicy policy = inValueWidget->sizePolicy();
		policy.setHorizontalPolicy(QSizePolicy::Ignored);
		inValueWidget->setSizePolicy(policy);
		inValueWidget->setAttribute(Qt::WA_TranslucentBackground);
		content->addWidget(inValueWidget);
	}
	setupContentWidget(content);
}

void QDetailViewRow::setupPropertyHandle(QPropertyHandle* val)
{
	mHandle = val;
	if (mHandle) {
		//connect(mHandle, &QPropertyHandle::asChildEvent, this, [this](QChildEvent* event) {
		//	if(event->child()->metaObject())
		//	if (!mView->getIgnoredObjects().contains(event->child()->metaObject()))
		//		mHandle->asRequestRebuildRow();
		//});
	}
}

int QDetailViewRow::childrenCount() const {
	return mChildren.count();
}

QDetailViewRow* QDetailViewRow::childAt(int inIndex) {
	return mChildren.value(inIndex);
}

int CountChildrenRow(QDetailViewRow* Root) {
	if (Root->childrenCount() == 0)
		return 0;
	int counter = Root->childrenCount();
	for (int i = 0; i < Root->childrenCount(); i++) {
		counter += CountChildrenRow(Root->childAt(i));
	}
	return counter;
}

QDetailViewRow* QDetailViewRow::addChildRow() {
	QDetailViewRow* row = new QDetailViewRow(mView);
	row->setParent(this);
	int parentIndex = mView->mLayout->indexOf(mWidget);
	int childCount = CountChildrenRow(this);
	mChildren << row;
	mView->mLayout->insertWidget(parentIndex + childCount + 1, row->mWidget, 0, Qt::AlignTop);
	return row;
}

void QDetailViewRow::removeChild(QDetailViewRow* inChild) {
	mChildren.removeOne(inChild);
	inChild->clear();
	mView->mLayout->removeWidget(inChild->mWidget);
	inChild->mWidget->deleteLater();
}

void QDetailViewRow::clear() {
	for (auto child : mChildren) {
		child->clear();
		mView->mLayout->removeWidget(child->mWidget);
		child->mWidget->deleteLater();
		child->deleteLater();
	}
	mChildren.clear();
}

void QDetailViewRow::setVisible(bool inVisiable) {
	mWidget->setVisible(inVisiable);
	requestRefreshSplitter();
	if (isExpanded()) {
		for (auto Child : mChildren) {
			Child->setVisible(inVisiable);
		}	
	}
}

bool QDetailViewRow::isVisible() const {
	return mWidget->isVisible();
}

bool QDetailViewRow::isCurrent() const
{
	return mView->getCurrentRow() == this;
}

void QDetailViewRow::setExpanded(bool inExpanded, bool bRecursive) {
	bExpanded = inExpanded;
	mWidget->mIndentWidget->RefreshState(mChildren.count(), bExpanded);
	for (auto Child : mChildren) {
		Child->setVisible(inExpanded);
		if (bRecursive) {
			Child->setExpanded(inExpanded, bRecursive);
		}
	}
}

bool QDetailViewRow::isExpanded() const {
	return bExpanded;
}

void QDetailViewRow::updateWidget()
{
	mWidget->update();
}

void QDetailViewRow::refresh() {
	mWidget->mIndentWidget->FixupWidth();
	setExpanded(bExpanded);
	for (auto Child : mChildren) {
		Child->mWidget->mIndentWidget->mLevel = mWidget->mIndentWidget->mLevel + 1;
		Child->setVisible(bExpanded);
		Child->refresh();
	}
}

void QDetailViewRow::markIsCategory() {
	bIsCategory = true;
}

bool QDetailViewRow::isCategory() {
	return bIsCategory;
}

QWidget* QDetailViewRow::getWidget() {
	return mWidget;
}

void QDetailViewRow::requestRefreshSplitter() {
	bNeedRefreshSplitter = true;
}

void QDetailViewRow::refreshSplitter() {
	if (QSplitter* splitter = qobject_cast<QSplitter*>(mWidget->mContentWidget)) {
		int nameWidgetWidth = splitter->width() - mView->mValueWidgetWidth - splitter->handleWidth();
		splitter->setSizes({ nameWidgetWidth ,mView->mValueWidgetWidth });
	}
	for (auto Child : mChildren) {
		Child->requestRefreshSplitter();
	}
}

void QDetailViewRow::fixupSplitter() {
	if (!isVisible())
		return;
	if (QSplitter* splitter = qobject_cast<QSplitter*>(mWidget->mContentWidget)) {
		int nameWidgetWidth = splitter->width() - mView->mValueWidgetWidth - splitter->handleWidth();
		if (nameWidgetWidth <= NAME_WIDGET_MIN_WIDTH) {
			mView->mValueWidgetWidth = splitter->width() - NAME_WIDGET_MIN_WIDTH - splitter->handleWidth();
		}
	}
	for (auto Child : mChildren) {
		Child->fixupSplitter();
	}
}


#include "QDetailViewRow.moc"