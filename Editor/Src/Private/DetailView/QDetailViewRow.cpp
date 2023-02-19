#include "QDetailViewRow.h"

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
		QColor shadowColor = QEngineEditorStyleManager::Instance()->GetShadowColor();
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
	QSvgIcon ExpandIcon = QSvgIcon(":/Resources/expand.png");
	QSvgIcon UnexpandIcon = QSvgIcon(":/Resources/unexpand.png");
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
		setMinimumHeight(25);
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
		mContentWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	}
Q_SIGNALS:
	void AsToggledExpand();
	void AsShowEvent();
protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override{
		Q_EMIT AsToggledExpand();
		event->accept();
	}
	void paintEvent(QPaintEvent *event){
		QPainter painter(this);
		if(mRow->IsCategory()){
			painter.fillRect(rect(), QEngineEditorStyleManager::Instance()->GetCategoryColor());
		}
		QPen pen(QEngineEditorStyleManager::Instance()->GetGridLineColor());
		pen.setWidth(1);
		painter.setPen(pen);
		if (mHoverd)
			painter.setBrush(QEngineEditorStyleManager::Instance()->GetHoveredColor());
		else
			painter.setBrush(Qt::NoBrush);
		painter.drawRect(rect());
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
		SetExpanded(!bExpanded);
	});
	connect(mWidget, &QDetailViewRowWidget::AsShowEvent, this, [this]() {
		RefreshSplitter();
	});
}

void QDetailViewRow::SetupContentWidget(QWidget* inContent) {
	mWidget->SetContentWidget(inContent);
	if (mView->isVisible()) {
		inContent->show();
		RefreshSplitter();
	}
}

void QDetailViewRow::SetupNameValueWidget(QWidget* inNameWidget, QWidget* inValueWidget) {
	QSplitter* content = new QSplitter;
	content->setHandleWidth(2);
	content->setChildrenCollapsible(false);
	QObject::connect(content, &QSplitter::splitterMoved,this, [this, content](int pos, int index) {
		mView->mValueWidgetWidth = qMax(1, content->width() - content->handleWidth() - pos);
		mView->RefreshRowsSplitter();
	});
	if (inNameWidget) {
		inNameWidget->setAttribute(Qt::WA_TranslucentBackground);
		content->addWidget(inNameWidget);
	}
	if (inValueWidget) {
		inValueWidget->setAttribute(Qt::WA_TranslucentBackground);
		content->addWidget(inValueWidget);
	}
	SetupContentWidget(content);
}

int QDetailViewRow::ChildCount() const {
	return mChildren.count();
}

QDetailViewRow* QDetailViewRow::ChildAt(int inIndex) {
	return mChildren.value(inIndex);
}

int CountChildrenRow(QDetailViewRow* Root) {
	if (Root->ChildCount() == 0)
		return 0;
	int counter = Root->ChildCount();
	for (int i = 0; i < Root->ChildCount(); i++) {
		counter += CountChildrenRow(Root->ChildAt(i));
	}
	return counter;
}

QDetailViewRow* QDetailViewRow::AddChildRow() {
	QDetailViewRow* row = new QDetailViewRow(mView);
	int parentIndex = mView->mLayout->indexOf(mWidget);
	int childCount = CountChildrenRow(this);
	mChildren << row;
	mView->mLayout->insertWidget(parentIndex + childCount + 1, row->mWidget, 0, Qt::AlignTop);
	return row;
}

void QDetailViewRow::DeleteChild(QDetailViewRow* inChild) {
	mChildren.removeOne(inChild);
	inChild->DeleteChildren();
	mView->mLayout->removeWidget(inChild->mWidget);
	inChild->mWidget->deleteLater();
}

void QDetailViewRow::DeleteChildren() {
	for (auto child : mChildren) {
		child->DeleteChildren();
		mView->mLayout->removeWidget(child->mWidget);
		child->mWidget->deleteLater();
	}
	mChildren.clear();
}

void QDetailViewRow::SetVisible(bool inVisiable) {
	mWidget->setVisible(inVisiable);
	RefreshSplitter();
	if (IsExpanded()) {
		for (auto Child : mChildren) {
			Child->SetVisible(inVisiable);
		}	
	}
}

bool QDetailViewRow::IsVisible() const {
	return mWidget->isVisible();
}

void QDetailViewRow::SetExpanded(bool inExpanded) {
	bExpanded = inExpanded;
	mWidget->mIndentWidget->RefreshState(mChildren.count(), bExpanded);
	for (auto Child : mChildren) {
		Child->SetVisible(inExpanded);
	}
}

bool QDetailViewRow::IsExpanded() const {
	return bExpanded;
}


void QDetailViewRow::Refresh() {
	mWidget->mIndentWidget->FixupWidth();
	SetExpanded(bExpanded);
	for (auto Child : mChildren) {
		Child->mWidget->mIndentWidget->mLevel = mWidget->mIndentWidget->mLevel + 1;
		Child->SetVisible(bExpanded);
		Child->Refresh();
	}
}

void QDetailViewRow::MarkIsCategory() {
	bIsCategory = true;
}

bool QDetailViewRow::IsCategory() {
	return bIsCategory;
}

void QDetailViewRow::RefreshSplitter() {
	if (QSplitter* splitter = qobject_cast<QSplitter*>(mWidget->mContentWidget)) {
		int nameWidgetWidth = splitter->width() - mView->mValueWidgetWidth - splitter->handleWidth();
		splitter->setSizes({ nameWidgetWidth ,mView->mValueWidgetWidth });
	}
	for (auto Child : mChildren) {
		Child->RefreshSplitter();
	}
}


void QDetailViewRow::FixupSplitter() {
	if (!IsVisible())
		return;
	if (QSplitter* splitter = qobject_cast<QSplitter*>(mWidget->mContentWidget)) {
		int nameWidgetWidth = splitter->width() - mView->mValueWidgetWidth - splitter->handleWidth();
		if (nameWidgetWidth <= NAME_WIDGET_MIN_WIDTH) {
			mView->mValueWidgetWidth = splitter->width() - NAME_WIDGET_MIN_WIDTH - splitter->handleWidth();
		}
	}
	for (auto Child : mChildren) {
		Child->FixupSplitter();
	}
}

#include "QDetailViewRow.moc"