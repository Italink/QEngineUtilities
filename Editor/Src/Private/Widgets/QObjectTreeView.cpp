#include "Widgets/QObjectTreeView.h"
#include "QPainter"
#include "qcoreevent.h"
#include "Utils/QEngineUndoStack.h"
#include "QEngineEditorStyleManager.h"
#include "DetailView/QPropertyHandle.h"

QObjectTreeView::QObjectTreeView() {
	setHeaderHidden(true);
	this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	connect(this, &QTreeWidget::itemSelectionChanged, this, [this]() {
		auto items = selectedItems();
		if (items.size() == 1) {
			Q_EMIT AsObjecteSelected(mItemMap[items.first()]);
		}
	});
	QFont font = QEngineEditorStyleManager::Instance()->GetFont();
	font.setPointSize(14);
	setFont(font);
}

void QObjectTreeView::SetObjects(QObjectList InObjects) {
	mTopLevelObjects = InObjects;
	ForceRefresh();
}

void QObjectTreeView::SelectObjects(QObjectList InObjects) {
	this->blockSignals(true);
	this->clearSelection();
	for (auto object : InObjects) {
		QTreeWidgetItem* item = mItemMap.key(object,nullptr);
		if (item) {
			QTreeWidgetItem* parent = item->parent();
			while (parent && !parent->isExpanded()) {
				parent->setExpanded(true);
				parent = item->parent();
			}
			item->setSelected(true);
		}
	}
	this->blockSignals(false);
}

void QObjectTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const {
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
	painter->fillRect(options.rect.adjusted(level * indentation(), 0, 0, 0), QEngineEditorStyleManager::Instance()->GetCategoryColor());
	QPen pen(QEngineEditorStyleManager::Instance()->GetGridLineColor());
	pen.setWidth(1);
	painter->setPen(pen);
	if (hovered)
		painter->setBrush(QEngineEditorStyleManager::Instance()->GetHoveredColor());
	if (seleted) {
		painter->setBrush(QEngineEditorStyleManager::Instance()->GetSelectedColor());
	}
	painter->drawRect(options.rect);
	if (hasChildren) {
		QColor arrowColor = QEngineEditorStyleManager::Instance()->GetArrowColor();
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
	QColor mShadowColor = QEngineEditorStyleManager::Instance()->GetShadowColor();
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

void QObjectTreeView::AddItemInternal(QTreeWidgetItem* inParentItem, QObject* inParentInstance) {
	if (inParentInstance->metaObject()->inherits(QObject::metaObject())) {
		QObject* Object = inParentInstance;
		for (auto& child : Object->children()) {
			if (IsIgnoreObject(child)) {
				continue;
			}
			QTreeWidgetItem* item = CreateItemForInstance(child);
			mItemMap[item] = child;
			inParentItem->addChild(item);
			AddItemInternal(item, child);
		}
	}
}

QTreeWidgetItem* QObjectTreeView::CreateItemForInstance(QObject* InInstance) {
	QString name;
	name += QString::asprintf("[%s: %p] : ", InInstance->metaObject()->className(), InInstance);
	if (InInstance->metaObject()->inherits(QObject::metaObject())) {
		name += InInstance->objectName();
	}
	QTreeWidgetItem* item = new QTreeWidgetItem({ name });
	item->setSizeHint(0, QSize(30, 30));
	InInstance->removeEventFilter(this);
	InInstance->installEventFilter(this);
	return item;
}

void QObjectTreeView::ForceRefresh() {
	clear();
	mItemMap.clear();
	if (mTopLevelObjects.isEmpty())
		return;
	for (auto& instance : mTopLevelObjects) {
		if (!instance)
			continue;
		QTreeWidgetItem* topItem = CreateItemForInstance(instance);
		mItemMap[topItem] = instance;
		addTopLevelItem(topItem);
		AddItemInternal(topItem, instance);
	}
}

bool QObjectTreeView::eventFilter(QObject* object, QEvent* event) {
	if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved) {
		QChildEvent* childEvent = static_cast<QChildEvent*>(event);
		qDebug() << event->type() << childEvent->child() << childEvent->child()->metaObject() << childEvent->child()->metaObject()->className();
		if (!IsIgnoreObject(childEvent->child()))
			ForceRefresh();
	}
	return QTreeWidget::eventFilter(object, event);
}

bool QObjectTreeView::IsIgnoreObject(QObject* inObject) {
	return inObject == nullptr
		|| inObject->metaObject()->inherits(&QPropertyHandle::staticMetaObject)
		|| inObject->metaObject()->inherits(&QEngineUndoEntry::staticMetaObject)
		|| inObject->metaObject()->inherits(&QEngineUndoStack::staticMetaObject)
		;
}