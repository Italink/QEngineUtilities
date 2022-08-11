#include "QDetailWidgetPrivate.h"

#include "QBoxLayout"
#include "QLabel"
#include "QLineEdit"
#include "QObjectDetailBuilder.h"
#include "QPainter"
#include "QPushButton"
#include "QApplication"

#include "Items\QDetailWidgetCategoryItem.h"
#include "Items\QDetailWidgetItem.h"
#include "Items\QDetailWidgetPropertyItem.h"
#include "Widgets\Toolkits\QHoverLineEdit.h"
#include "Widgets\Toolkits\QSvgIcon.h"

QDetailSearcher::QDetailSearcher()
	: mLbSearch(new QLabel)
	, mIconSearch(new QSvgIcon(":/Resources/search.png"))
	, mLeSearch(new QHoverLineEdit)
	, mPbSearch(new QPushButton) {
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mLbSearch);
	h->addWidget(mLeSearch);
	h->addWidget(mPbSearch);
	h->setContentsMargins(0,0,0,0);
	mPbSearch->setFixedWidth(30);
	mLeSearch->setMinimumHeight(20);
	mLeSearch->SetPlaceholdText("Search...");
	mLbSearch->setFixedSize(20, 20);
	mIconSearch->setUpdateCallBack([this]() {
		mLbSearch->setPixmap(mIconSearch->getIcon().pixmap(mLbSearch->size()));
	});

	connect(mLeSearch, &QHoverLineEdit::AsEditingFinished, this, [this]() {
		Q_EMIT AsRequestSearch(mLeSearch->GetText());
	});
}

QDetailTreeWidget::QDetailTreeWidget()
{
	setColumnCount(1);
	setIndentation(15);
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setColumnWidth(0, 120);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setFrameStyle(QFrame::NoFrame);
	connect(this, &QTreeWidget::itemPressed, this, [](QTreeWidgetItem* item, int) {
		if (qApp->mouseButtons() & Qt::RightButton) {
			QDetailWidgetItem* detailItem = (QDetailWidgetItem*)item;
			QMenu menu;
			detailItem->BuildMenu(menu);
			menu.exec(QCursor::pos());
		}
	});
}

void QDetailTreeWidget::SetObjects(const QList<QObject*>& inObjects)
{
	for (auto& Object : mObjects) {
		QDetailUndoEntry* UndoEntry = Object->findChild<QDetailUndoEntry*>(QString(), Qt::FindDirectChildrenOnly);
		mUndoStack.RemoveEntry(UndoEntry);
	}
	mObjects = inObjects;
	Recreate();
	for (auto& Object : mObjects) {
		QDetailUndoEntry* UndoEntry = Object->findChild<QDetailUndoEntry*>(QString(), Qt::FindDirectChildrenOnly);
		mUndoStack.AddEntry(UndoEntry);
	}
}

void QDetailTreeWidget::Recreate()
{
	clear();
	if (mObjects.isEmpty())
		return;
	for (QObject* object : mObjects) {
		if (object == nullptr)
			continue;
		QObjectDetailBuilder builder(object, this);
		//const QSharedPointer<IObjectDetailCustomization>& customizer = QDetailWidgetManager::instance()->getObjectDetailCustomizer(object->metaObject());
		//if (customizer) {
		//	customizer->Build(builder);
		//}
		//else {
			builder.BuildDefault();
		//}
	}
}

void SearchByKeywordsInternal(QString inKeywords, QTreeWidgetItem* inItem) {
	if (inItem == nullptr)
		return;
	inItem->setHidden(true);
	QString keywordsInRow;
	QDetailWidgetItem* item = dynamic_cast<QDetailWidgetItem*>(inItem);
	if (item) {
		keywordsInRow = item->GetKeywords();
	}
	else {
		keywordsInRow = inItem->text(0);
	}
	if (keywordsInRow.contains(inKeywords,Qt::CaseInsensitive)) {
		while (inItem&&inItem->isHidden()) {
			inItem->setHidden(false);
			inItem->setExpanded(true);
			inItem = inItem->parent();
		}
		return;
	}
	for (int i = 0; i < inItem->childCount(); i++) {
		SearchByKeywordsInternal(inKeywords, inItem->child(i));
	}
}

void QDetailTreeWidget::SearchByKeywords(QString inKeywords) {
	if (inKeywords.isEmpty()) {
		QTreeWidgetItemIterator Iterator(this);
		while (*Iterator) {
			(*Iterator)->setHidden(false);
			++Iterator;
		}
	}
	else {
		for (int i = 0; i < topLevelItemCount(); i++) {
			SearchByKeywordsInternal(inKeywords, topLevelItem(i));
		}
	}
}

void QDetailTreeWidget::Undo()
{
	mUndoStack.undo();
}

void QDetailTreeWidget::Redo()
{
	mUndoStack.redo();
}


QList<int> QDetailTreeWidget::GetSplitterSizes() const
{
	return mSplitterSizes;
}

void QDetailTreeWidget::SetSplitterSizes(int item0, int item1, int item2)
{
	mSplitterSizes = { item0 ,item1,item2 };
	UpdateSplitterFactor();
}

QColor QDetailTreeWidget::GetGridLineColor() const
{
	return mGridLineColor;
}

void QDetailTreeWidget::SetGridLineColor(QColor val)
{
	mGridLineColor = val;
}

QColor QDetailTreeWidget::GetShadowColor() const
{
	return mShadowColor;
}

void QDetailTreeWidget::SetShadowColor(QColor val)
{
	mShadowColor = val;
}

QColor QDetailTreeWidget::GetCategoryColor() const {
	return mCategoryColor;
}

void QDetailTreeWidget::SetCategoryColor(QColor val) {
	mCategoryColor = val;
}

QColor QDetailTreeWidget::GetHoveredColor() const {
	return mHoveredColor;
}

void QDetailTreeWidget::SetHoveredColor(QColor val) {
	mHoveredColor = val;
}

QColor QDetailTreeWidget::GetIconColor() const
{
	return QSvgIcon::GetIconColor();
}

void QDetailTreeWidget::SetIconColor(QColor val)
{
	QSvgIcon::setIconColor(val);
}

QDetailWidgetCategoryItem* QDetailTreeWidget::FindOrAddCategory(QString inName)
{
	for (int i = 0; i < topLevelItemCount(); i++) {
		QTreeWidgetItem* item = topLevelItem(i);
		if (item->text(0) == inName)
			return dynamic_cast<QDetailWidgetCategoryItem*>(item);
	}
	QDetailWidgetCategoryItem* newItem = new QDetailWidgetCategoryItem( inName );
	newItem->setSizeHint(0, { 25,25 });
	newItem->AttachTo(this);
	return newItem;
}

void QDetailTreeWidget::UpdateSplitterFactor()
{
	QTreeWidgetItemIterator Iterator(this);
	while (*Iterator) {
		QDetailWidgetPropertyItem* item = dynamic_cast<QDetailWidgetPropertyItem*>(*Iterator);
		if (item) {
			item->RefleshSplitterFactor();
		}
		++Iterator;
	}
}

void QDetailTreeWidget::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const
{
	bool hovered = false;
	bool seleted = false;
	bool hasChildren = false;
	bool isExpanded = false;
	int level = 0;

	QTreeWidgetItem* item = itemFromIndex(index);
	QModelIndex parent = index.parent();
	while (parent.isValid()) {
		level++;
		parent = parent.parent();
	}
	QRect branchRect(options.rect.x() + level * indentation(), options.rect.y(), indentation(), options.rect.height());
	QPoint mousePos = mapFromGlobal(QCursor::pos());
	seleted = item->isSelected();
	hovered = options.rect.contains(mousePos);
	hasChildren = item->childCount() > 0;
	isExpanded = item->isExpanded();

	painter->save();

	if (level == 0) {
		painter->fillRect(options.rect, mCategoryColor);
	}

	QPen pen(mGridLineColor);
	pen.setWidth(1);
	painter->setPen(pen);
	if (hovered) {
		painter->setBrush(mHoveredColor);
	}
	else
		painter->setBrush(Qt::NoBrush);
	painter->drawRect(options.rect);

	if (hasChildren) {
		QColor arrowColor = mArrowColor;
		if (hovered)
			arrowColor = arrowColor.lighter();
		QPolygonF arrow;
		QPointF center = branchRect.center();
		painter->setBrush(arrowColor);
		painter->setPen(Qt::NoPen);
		painter->setRenderHint(QPainter::Antialiasing);
		if (item->isExpanded()) {
			arrow << center + QPointF(5, -3) << center + QPointF(0, 5) << center + QPointF(-5, -3);
		}
		else {
			arrow << center + QPointF(-3, -5) << center + QPointF(5, 0) << center + QPointF(-3, 5);
		}
		painter->drawPolygon(arrow);
	}
	QRect shadowRect(options.rect.x(), options.rect.y(), 6, options.rect.height());
	QLinearGradient shadowColor;
	
	shadowColor.setColorAt(0, QColor(mShadowColor.red(), mShadowColor.green(), mShadowColor.blue(), 0));
	shadowColor.setColorAt(1, QColor(mShadowColor.red(), mShadowColor.green(), mShadowColor.blue()));

	for (int i = 0; i < level; i++) {
		shadowRect.moveRight((i + 1) * indentation());
		shadowColor.setStart(shadowRect.topLeft());
		shadowColor.setFinalStop(shadowRect.topRight());
		painter->fillRect(shadowRect, shadowColor);
	}

	QStyleOptionViewItem opt = options;
	if (level == 0) {
		opt.rect.moveLeft(indentation());
	}
	painter->restore();
	itemDelegateForIndex(index)->paint(painter, opt, index);
}

void QDetailTreeWidget::showEvent(QShowEvent* event)
{
	QTreeWidget::showEvent(event);
	UpdateSplitterFactor();
}

