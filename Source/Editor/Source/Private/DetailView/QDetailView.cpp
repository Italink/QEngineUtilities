#include "DetailView/QDetailView.h"
#include "DetailView/QDetailViewRow.h"
#include "Widgets/QElideLabel.h"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/QDetailLayoutBuilder.h"
#include "QEngineEditorStyleManager.h"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QQueue>
#include "QEngineUndoStack.h"
#include "QAbstractAnimation"


QDetailView::QDetailView()
	: mView(new QWidget)
	, mLayout(new QVBoxLayout())
	, mLayoutBuilder(new QDetailLayoutBuilder(this))
{
	Q_INIT_RESOURCE(Resources);
	this->setWidget(mView);
	this->setWidgetResizable(true);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	this->setStyleSheet(QEngineEditorStyleManager::Instance()->getStylesheet());
	connect(QEngineEditorStyleManager::Instance(), &QEngineEditorStyleManager::asPaletteChanged, this, [this]() {
		bNeedUpdateStyle = true;
	});
	this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	mView->setLayout(mLayout);
	mView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	mLayout->setSpacing(0);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

	mIgnoreMetaObjects = {
		&QAbstractAnimation::staticMetaObject,
		&QPropertyHandle::staticMetaObject,
		&QEngineUndoEntry::staticMetaObject,
		&QEngineUndoStack::staticMetaObject,
	};
}

QDetailView::~QDetailView()
{
}

void QDetailView::setObject(QObject* inObject) {
	setObjects({ inObject });
}

void QDetailView::setObjects(const QObjectList& inObjects) {
	if (mObjects != inObjects) {
		mObjects = inObjects;
		forceRebuild();
	}
}

void QDetailView::selectSubObject(QObject* inObject)
{
	if (!inObject) {
		setCurrentRow(nullptr);
	}
	else {
		QDetailViewRow* objectRow = nullptr;
		foreachRows([&objectRow, inObject](QDetailViewRow* row) {
			if (QPropertyHandle* handle = row->getPropertyHandle()) {
				if (handle->parent() == inObject) {
					objectRow = row;
					return false;
				}
			}
			return true;
		});
		if (objectRow) {
			setCurrentRow(objectRow);
		}
	}
}

void QDetailView::setFlags(Flags inFlag)
{
	mFlags = inFlag;
}

QDetailView::Flags QDetailView::getFlags() const
{
	return mFlags;
}

void QDetailView::searchByKeywords(QString inKeywords) {

}

void QDetailView::undo() {

}

void QDetailView::redo() {

}

void QDetailView::forceRebuild() {
	reset();
	mObjects.removeAll(nullptr);
	setWidget(mView);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	for(auto& object:mObjects){
		mLayoutBuilder->addObject(object);
	}
	refreshRowsState();
}

void QDetailView::setPage(QWidget* inPage) {
	takeWidget();
	setWidget(inPage);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void QDetailView::paintEvent(QPaintEvent* event)
{
	if (bNeedUpdateStyle) {
		setStyleSheet(QEngineEditorStyleManager::Instance()->getStylesheet());
		bNeedUpdateStyle = false;
	}
}

void QDetailView::resizeEvent(QResizeEvent* event) {
	QScrollArea::resizeEvent(event);
	if (mValueWidgetWidth == 0) {
		mValueWidgetWidth = event->size().width() / 2;
		refreshRowsSplitter();
		return;
	}
	float factor = event->size().width() / (float)event->oldSize().width();
	if (factor > 0) {
		mValueWidgetWidth *= factor;
		refreshRowsSplitter();
	}
}

void QDetailView::reset() {
	mCurrentRow = nullptr;
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

void QDetailView::refreshRowsState() {
	refreshRowsSplitter();
;	for (auto row : mTopLevelRows) {
		row->setExpanded(true, true);
		row->refresh();
	}
}

void QDetailView::refreshRowsSplitter() {
	for (auto row : mTopLevelRows) {
		row->fixupSplitter();
	}
	for (auto row : mTopLevelRows) {
		row->requestRefreshSplitter();
	}
	update();
}

void QDetailView::foreachRows(std::function<bool(QDetailViewRow*)> inProcessor)
{
	QQueue<QDetailViewRow*> queue;
	queue << mTopLevelRows;
	while (!queue.isEmpty()){
		QDetailViewRow* top = queue.dequeue();
		if (!inProcessor(top)) {
			break;
		}
		queue << top->mChildren;
	}
}

QDetailViewRow* QDetailView::addTopLevelRow() {
	QDetailViewRow* row = new QDetailViewRow(this);
	row->setParent(this);
	mTopLevelRows << row;
	mLayout->addWidget((QWidget*)row->mWidget, 0, Qt::AlignTop);
	return row;
}

QDetailViewRow* QDetailView::getCurrentRow() const{
	return mCurrentRow;
}

void QDetailView::setCurrentRow(QDetailViewRow* val){
	if (mCurrentRow != val) {
		auto LastRow = mCurrentRow;
		mCurrentRow = val;
		if (mCurrentRow) 
			mCurrentRow->updateWidget();	
		if (LastRow) 
			LastRow->updateWidget();
		Q_EMIT asCurrentRowChanged(mCurrentRow);
	}
}

const QSet<QMetaType>& QDetailView::getIgnoredTypes() const{
	return mIgnoreMetaTypes;
}

void QDetailView::setIgnoredTypes(QSet<QMetaType> val){
	mIgnoreMetaTypes = val;
}

const QSet<const QMetaObject*>& QDetailView::getIgnoredObjects() const{
	return mIgnoreMetaObjects;
}

void QDetailView::setIgnoreObjects(QSet<const QMetaObject*> val){
	mIgnoreMetaObjects = val;
	mIgnoreMetaObjects 
		<< &QAbstractAnimation::staticMetaObject
		<< &QPropertyHandle::staticMetaObject
		<< &QEngineUndoEntry::staticMetaObject
		<< &QEngineUndoStack::staticMetaObject;
}

