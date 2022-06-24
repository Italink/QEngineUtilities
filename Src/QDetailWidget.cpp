#include "QDetailWidget.h"
#include "QPainter"
#include "QLineEdit"
#include "QPushButton"
#include "QBoxLayout"
#include "QObjectDetailBuilder.h"
#include "Items\QDetailWidgetItem.h"
#include "Items\QDetailWidgetPropertyItem.h"
#include "Items\QDetailWidgetCategoryItem.h"
#include "Widgets\Toolkits\QSvgIcon.h"

QDetailSearcher::QDetailSearcher()
	: mLeSearch(new QLineEdit)
	, mPbSearch(new QPushButton) {
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mLeSearch);
	h->addWidget(mPbSearch);
	h->setContentsMargins(0, 0, 0, 0);
	mPbSearch->setFixedWidth(30);
	mLeSearch->setPlaceholderText("Search...");
	connect(mLeSearch, &QLineEdit::editingFinished, this, [this]() {
		Q_EMIT AsRequestSearch(mLeSearch->text());
	});
}

QDetailWidget::QDetailWidget()
	: mSearcher(new QDetailSearcher)
{
	setColumnCount(1);
	setIndentation(15);
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setColumnWidth(0, 120);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setFrameStyle(QFrame::NoFrame);
	setStyleSheet(R"(
QWidget{
	color:rgb(220,220,220);
}
QNumberBox{
	background-color:rgb(10,10,10);
}
QDetailWidget{
	background-color:rgb(36,36,36);
	qproperty-ShadowColor:rgb(5,5,5);
	qproperty-GridLineColor:rgb(5,5,5);
	qproperty-CategoryColor:rgb(61,61,61);  
	qproperty-HoveredColor:rgb(51,51,51); 
	qproperty-IconColor:rgb(251,251,251); 
}
QLineEdit,QTextEdit{
	background-color:rgb(5,5,5);
	border-radius: 3px;
	color: rgb(220,220,220);
	border: 1px solid transparent;
}

QLineEdit:hover,QTextEdit:hover,QPushButton:hover,QComboBox:hover{
	border: 1px solid rgb(79, 110, 242);
}

QPushButton,QComboBox{
	background-color:rgb(5,5,5);
	color: rgb(220,220,220);
	border: 1px solid transparent;
}

QComboBox QAbstractItemView {
	padding: 0px 0px 4px 0px;
    border: 0px solid transparent;
	border-radius: 0px;
	color: rgb(200,200,200);
    selection-color: rgb(255,255,255);
	background-color: rgb(26,26,26);
    selection-background-color: rgb(49,49,49); 
}
)"
	);
	connect(this, &QTreeWidget::itemPressed, this, [](QTreeWidgetItem* item, int) {
	});

	connect(mSearcher, &QDetailSearcher::AsRequestSearch, this, &QDetailWidget::SearchByKeywords);
}

void QDetailWidget::SetObjects(const QList<QObject*>& inObjects)
{
	mObjects = inObjects;
	Recreate();
}

void QDetailWidget::Recreate()
{
	clear();
	QTreeWidgetItem* searchItem = new QTreeWidgetItem;
	addTopLevelItem(searchItem);
	setItemWidget(searchItem, 0, mSearcher);
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

void QDetailWidget::SearchByKeywords(QString inKeywords) {
	if (inKeywords.isEmpty()) {
		QTreeWidgetItemIterator Iterator(this);
		while (*Iterator) {
			(*Iterator)->setHidden(false);
			++Iterator;
		}
	}
	else {
		for (int i = 1; i < topLevelItemCount(); i++) {
			SearchByKeywordsInternal(inKeywords, topLevelItem(i));
		}
	}
}

QList<int> QDetailWidget::GetSplitterSizes() const
{
	return mSplitterSizes;
}

void QDetailWidget::SetSplitterSizes(int item0, int item1, int item2)
{
	mSplitterSizes = { item0 ,item1,item2 };
	UpdateSplitterFactor();
}

QColor QDetailWidget::GetGridLineColor() const
{
	return mGridLineColor;
}

void QDetailWidget::SetGridLineColor(QColor val)
{
	mGridLineColor = val;

	QTreeWidgetItemIterator Iterator(this);
	while (*Iterator) {
		QDetailWidgetPropertyItem* item = dynamic_cast<QDetailWidgetPropertyItem*>(*Iterator);
		if (item) {
			item->GetContent()->setStyleSheet(QString("QSplitter::handle {background-color: %1;}").arg(mGridLineColor.name(QColor::HexArgb)));
			item->RefleshSplitterFactor();
		}
		++Iterator;
	}
}

QColor QDetailWidget::GetShadowColor() const
{
	return mShadowColor;
}

void QDetailWidget::SetShadowColor(QColor val)
{
	mShadowColor = val;
}

QColor QDetailWidget::GetIconColor() const
{
	return QSvgIcon::GetIconColor();
}

void QDetailWidget::SetIconColor(QColor val)
{
	QSvgIcon::setIconColor(val);
}

QDetailWidgetCategoryItem* QDetailWidget::FindOrAddCategory(QString inName)
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

void QDetailWidget::UpdateSplitterFactor()
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

void QDetailWidget::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const
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
		QBrush brush = options.palette.brush(QPalette::WindowText);
		if (hovered)
			brush.setColor(QColor(42, 140, 254));
		QPolygonF arrow;
		QPointF center = branchRect.center();
		painter->setBrush(brush);
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

void QDetailWidget::showEvent(QShowEvent* event)
{
	QTreeWidget::showEvent(event);
	int resetItemWidth = 30;
	int width0 = (width() - resetItemWidth) / 2;
	SetSplitterSizes(width0, width0, resetItemWidth);
}

