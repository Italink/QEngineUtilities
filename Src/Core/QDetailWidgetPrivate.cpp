#include "QDetailWidgetPrivate.h"

#include "QBoxLayout"
#include "QLabel"
#include "QLineEdit"
#include "Customization\Instance\QInstanceDetail.h"
#include "QPainter"
#include "QPushButton"
#include "QApplication"
#include "QKeyEvent"

#include "Customization\Item\QDetailWidgetCategoryItem.h"
#include "Customization\Item\QDetailWidgetItem.h"
#include "Customization\Item\QDetailWidgetPropertyItem.h"
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

void QDetailTreeWidget::SetInstances(const QList<QSharedPointer<QInstance>>& inInstance)
{
	for (auto object : inInstance) 
		mInstanceList << object;
	Recreate();
}

void QDetailTreeWidget::Recreate()
{
	clear();
	if (mInstanceList.isEmpty())
		return;
	for (auto& instance : mInstanceList) {
		if (instance.isNull())
			continue;
		QInstanceDetail* builder = QInstanceDetail::FindOrCreate(instance, this);
		builder->Build();
	}
}

void SearchByKeywordsInternal(QString inKeywords, QTreeWidgetItem* inItem) {
	if (inItem == nullptr)
		return;
	inItem->setHidden(true);
	QString keywordsInRow;
	QDetailWidgetItem* item = static_cast<QDetailWidgetItem*>(inItem);
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
	QDetailUndoStack::Instance()->Undo();
}

void QDetailTreeWidget::Redo()
{
	QDetailUndoStack::Instance()->Redo();
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

void QDetailTreeWidget::UpdateSplitterFactor()
{
	QTreeWidgetItemIterator Iterator(this);
	while (*Iterator) {
		QDetailWidgetItem* item = static_cast<QDetailWidgetItem*>(*Iterator);
		if (item->Type() == QDetailWidgetItem::Property) {
			QDetailWidgetPropertyItem* propItem = static_cast<QDetailWidgetPropertyItem*>(*Iterator);
			propItem->RefleshSplitterFactor();
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
	QDetailWidgetItem* item = static_cast<QDetailWidgetItem*>(itemFromIndex(index));
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

	if (item->Type() == QDetailWidgetItem::Category) 
		painter->fillRect(options.rect.adjusted(level * indentation(),0,0,0), mCategoryColor);
	QPen pen(mGridLineColor);
	pen.setWidth(1);
	painter->setPen(pen);
	if (hovered) 
		painter->setBrush(mHoveredColor);
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
	opt.rect.moveLeft((level + 1) * indentation());
	painter->restore();
	itemDelegateForIndex(index)->paint(painter, opt, index);
}

void QDetailTreeWidget::showEvent(QShowEvent* event)
{
	QTreeWidget::showEvent(event);
	UpdateSplitterFactor();
}

void QDetailTreeWidget::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
		switch (event->key()) {
		case Qt::Key_Z: {
			Undo();
			break;
		}
		case Qt::Key_Y: {
			Redo();
			break;
		}
		}
	}
}