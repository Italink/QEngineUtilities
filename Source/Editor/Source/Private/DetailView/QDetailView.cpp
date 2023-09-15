#include "DetailView/QDetailView.h"
#include "DetailView/QDetailViewRow.h"
#include "QLabel"
#include "QPushButton"
#include "Widgets/QElideLabel.h"
#include "DetailView/QDetailViewManager.h"
#include "qevent.h"
#include "DetailView/QDetailLayoutBuilder.h"
#include "QEngineEditorStyleManager.h"
#include "QApplication"
#include "QEngineUndoStack.h"

QDetailView::QDetailView()
	: mView(new QWidget)
	, mLayout(new QVBoxLayout())
	, mLayoutBuilder(new QDetailLayoutBuilder(this))
{
	Q_INIT_RESOURCE(Resources);
	this->setWidget(mView);
	this->setWidgetResizable(true);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	mView->setLayout(mLayout);
	mView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	mLayout->setSpacing(0);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	qApp->setStyleSheet(QEngineEditorStyleManager::Instance()->GetStylesheet());

	mIgnoreMetaObjects = {
		&QPropertyHandle::staticMetaObject,
		&QEngineUndoEntry::staticMetaObject,
		&QEngineUndoStack::staticMetaObject,
	};
}

void QDetailView::SetObject(QObject* inObject) {
	SetObjects({ inObject });
}

void QDetailView::SetObjects(const QObjectList& inObjects) {
	if (mObjects != inObjects) {
		mObjects = inObjects;
		ForceRebuild();
	}
}

void QDetailView::SetFlags(Flags inFlag)
{
	mFlags = inFlag;
}

QDetailView::Flags QDetailView::GetFlags() const
{
	return mFlags;
}

void QDetailView::SearchByKeywords(QString inKeywords) {

}

void QDetailView::Undo() {

}

void QDetailView::Redo() {

}

void QDetailView::ForceRebuild() {
	Reset();
	mObjects.removeAll(nullptr);
	setWidget(mView);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	for(auto& object:mObjects){
		mLayoutBuilder->AddObject(object);
	}
	RefreshRowsState();
}

void QDetailView::SetPage(QWidget* inPage) {
	takeWidget();
	setWidget(inPage);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void QDetailView::resizeEvent(QResizeEvent* event) {
	QScrollArea::resizeEvent(event);
	if (mValueWidgetWidth == 0) {
		mValueWidgetWidth = event->size().width() / 2;
		RefreshRowsSplitter();
		return;
	}
	float factor = event->size().width() / (float)event->oldSize().width();
	if (factor > 0) {
		mValueWidgetWidth *= factor;
		RefreshRowsSplitter();
	}
}

void QDetailView::Reset() {
	for (auto row : mTopLevelRows) {
		row->setParent(nullptr);
		row->deleteLater();
	}
	mTopLevelRows.clear();
	while (mLayout && mLayout->count()) {
		QLayoutItem* child = mLayout->takeAt(0);
		if (child && child->widget()) {
			QWidget* widget = child->widget();
			widget->setParent(nullptr);
			delete widget;
		}
		delete child;
	}
}

void QDetailView::RefreshRowsState() {
	RefreshRowsSplitter();
;	for (auto Row : mTopLevelRows) {
		Row->SetExpanded(true, true);
		Row->Refresh();
	}
}

void QDetailView::RefreshRowsSplitter() {
	for (auto Row : mTopLevelRows) {
		Row->FixupSplitter();
	}
	for (auto Row : mTopLevelRows) {
		Row->RequestRefreshSplitter();
	}
	update();
}

QDetailViewRow* QDetailView::AddTopLevelRow() {
	QDetailViewRow* row = new QDetailViewRow(this);
	mTopLevelRows << row;
	mLayout->addWidget((QWidget*)row->mWidget, 0, Qt::AlignTop);
	return row;
}

QDetailViewRow* QDetailView::GetCurrentRow() const{
	return mCurrentRow;
}

void QDetailView::SetCurrentRow(QDetailViewRow* val){
	if (mCurrentRow != val) {
		auto LastRow = mCurrentRow;
		mCurrentRow = val;
		if (mCurrentRow) 
			mCurrentRow->UpdateWidget();	
		if (LastRow) 
			LastRow->UpdateWidget();
		Q_EMIT AsCurrentRowChanged(mCurrentRow);
	}
}

const QSet<QMetaType>& QDetailView::GetIgnoreMetaTypes() const{
	return mIgnoreMetaTypes;
}

void QDetailView::SetIgnoreMetaTypes(QSet<QMetaType> val){
	mIgnoreMetaTypes = val;
}

const QSet<const QMetaObject*>& QDetailView::GetIgnoreMetaObjects() const{
	return mIgnoreMetaObjects;
}

void QDetailView::SetIgnoreMetaObjects(QSet<const QMetaObject*> val){
	mIgnoreMetaObjects = val;
	mIgnoreMetaObjects 
		<< &QPropertyHandle::staticMetaObject
		<< &QEngineUndoEntry::staticMetaObject
		<< &QEngineUndoStack::staticMetaObject;
}

