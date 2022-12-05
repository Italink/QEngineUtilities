#include "QInstanceTreeWidget.h"
#include "QPainter"
#include "QDetailWidgetStyleManager.h"

QInstanceTreeWidget::QInstanceTreeWidget() {
	setHeaderHidden(true);
	this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	connect(this, &QTreeWidget::itemSelectionChanged, this, [this]() {
		auto items = selectedItems();
		if (items.size() == 1) {
			Q_EMIT AsInstanceSelected(mItemMap[items.first()]);
		}
	});
}

void QInstanceTreeWidget::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const {
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
	painter->fillRect(options.rect.adjusted(level * indentation(), 0, 0, 0), QDetailWidgetStyleManager::Instance()->GetCategoryColor());
	QPen pen(QDetailWidgetStyleManager::Instance()->GetGridLineColor());
	pen.setWidth(1);
	painter->setPen(pen);
	if (hovered)
		painter->setBrush(QDetailWidgetStyleManager::Instance()->GetHoveredColor());
	if(seleted){
		painter->setBrush(QDetailWidgetStyleManager::Instance()->GetSelectedColor());
	}
	painter->drawRect(options.rect);
	if (hasChildren) {
		QColor arrowColor = QDetailWidgetStyleManager::Instance()->GetArrowColor();
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
	QColor mShadowColor = QDetailWidgetStyleManager::Instance()->GetShadowColor();
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

void QInstanceTreeWidget::AddItemInternal(QTreeWidgetItem* inParentItem, QInstance* inParentInstance) {
	if (inParentInstance->GetMetaObject()->inherits(QObject::metaObject())) {
		QObject* Object = inParentInstance->GetOuterObject();
		for (auto& child : Object->children()) {
			auto tempInstance = QInstance::CreateObjcet(child);
			QTreeWidgetItem* item = CreateItemForInstance(tempInstance.get());
			mItemMap[item] = tempInstance;
			inParentItem->addChild(item);
			AddItemInternal(item, tempInstance.get());
		}
	}
}

void QInstanceTreeWidget::SetInstances(const QList<QSharedPointer<QInstance>>& inInstances) {
	clear();
	mTopLevelInstances = inInstances;
	if (mTopLevelInstances.isEmpty())
		return;
	for (auto& instance : mTopLevelInstances) {
		if (instance.isNull())
			continue;
		QTreeWidgetItem* topItem = CreateItemForInstance(instance.get());
		mItemMap[topItem] = instance;
		addTopLevelItem(topItem);
		AddItemInternal(topItem, instance.get());
	}
}

QTreeWidgetItem* QInstanceTreeWidget::CreateItemForInstance(QInstance* InInstance) {
	QString name;
	name += QString::asprintf("[%s: %p] : ", InInstance->GetMetaObject()->className(),InInstance->GetPtr());
	if (InInstance->GetMetaObject()->inherits(QObject::metaObject())) {
		name += InInstance->GetOuterObject()->objectName();
	}
	QTreeWidgetItem* item = new QTreeWidgetItem({ name });
	item->setSizeHint(0, QSize(25,25));
	return item;
}