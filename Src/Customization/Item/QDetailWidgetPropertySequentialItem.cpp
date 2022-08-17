#include "QDetailWidgetPropertySequentialItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "Customization\QDetailWidgetManager.h"
#include "QPushButton"
#include "Widgets\Toolkits\QSvgButton.h"

QDetailWidgetPropertySequentialItem::QDetailWidgetPropertySequentialItem() {
}

bool QDetailWidgetPropertySequentialItem::FilterType(QMetaType inID) {
	if (QMetaType::canConvert(inID, QMetaType::fromType<QVariantList>())
		&& !QMetaType::canConvert(inID, QMetaType::fromType<QString>())
		) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertySequentialItem::SetHandler(QPropertyHandler* inHandler) {
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	mValueType = iterable.valueMetaType();
	connect(GetHandler(), &QPropertyHandler::AsValueChanged, this, &QDetailWidgetPropertySequentialItem::RecreateChildren);
}

void QDetailWidgetPropertySequentialItem::FindOrCreateChildItem(int index) {
	QPropertyHandler* handler = QPropertyHandler::FindOrCreate(
		GetParentObject(),
		mValueType,
		GetHandler()->GetSubPath(QString::number(index)),
		[this, index]() {
		QVariant varList = GetValue();
		QSequentialIterable iterable = varList.value<QSequentialIterable>();
		QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
		return iterable.at(index);
	},
		[this, index](QVariant var) {
		QVariant varList = GetValue();
		QSequentialIterable iterable = varList.value<QSequentialIterable>();
		const QMetaSequence metaSequence = iterable.metaContainer();
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		QtPrivate::QVariantTypeCoercer coercer;
		const void* dataPtr = coercer.coerce(var, var.metaType());
		metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
		SetValue(varList);
	});

	if (index < childCount()) {
		QDetailWidgetPropertyItem* item = (QDetailWidgetPropertyItem*)child(index);
		item->SetHandler(handler);
	}
	else {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(handler);
		if (item) {
			item->SetBuildContentAndChildrenCallback([item,this,index](QHBoxLayout* inLayout) {
				if (!GetHandler()->GetMetaData("FixedOrder").toBool()) {
					if (index != 0) {
						QSvgButton* moveUp = new QSvgButton(":/Resources/up.png");
						moveUp->setFixedSize(20, 20);
						connect(moveUp, &QSvgButton::clicked, this, [index, this]() {
							MoveItem(index, index - 1);
							});
						inLayout->addWidget(moveUp);
					}
					else {
						inLayout->addSpacing(22);
					}
					if (index != mCount - 1) {
						QSvgButton* moveDown = new QSvgButton(":/Resources/down.png");
						moveDown->setFixedSize(20, 20);
						connect(moveDown, &QSvgButton::clicked, this, [index, this]() {
							MoveItem(index, index + 1);
							});
						inLayout->addWidget(moveDown);
					}
					else {
						inLayout->addSpacing(22);
					}
				}
				if (!GetHandler()->GetMetaData("FixedSize").toBool()) {
					QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
					deleteButton->setFixedSize(20, 20);
					connect(deleteButton, &QSvgButton::clicked, this, [index, this]() {
						RemoveItem(index);
						});
					inLayout->addWidget(deleteButton);
				}
			});
			item->AttachTo(this);
			item->setExpanded(true);
		}
	}
}

void QDetailWidgetPropertySequentialItem::MoveItem(int inSrcIndex, int inDstIndex) {
	QVariant varList = GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant srcVar = iterable.at(inSrcIndex);
	QVariant dstVar = iterable.at(inDstIndex);
	metaSequence.setValueAtIndex(containterPtr, inDstIndex, coercer.coerce(srcVar, srcVar.metaType()));
	metaSequence.setValueAtIndex(containterPtr, inSrcIndex, coercer.coerce(dstVar, dstVar.metaType()));
	SetValue(varList, QString("%1 Move: %2->%3").arg(GetHandler()->GetPath()).arg(inSrcIndex).arg(inDstIndex));

	FroceRebuildChild(inSrcIndex);
	FroceRebuildChild(inDstIndex);
}

void QDetailWidgetPropertySequentialItem::RemoveItem(int inIndex) {
	QVariant varList = GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	for (int i = inIndex; i < iterable.size() - 1; i++) {
		QVariant nextVar = iterable.at(i + 1);
		metaSequence.setValueAtIndex(containterPtr, inIndex, coercer.coerce(nextVar, nextVar.metaType()));
	}
	metaSequence.removeValueAtEnd(containterPtr);
	SetValue(varList, QString("%1 Remove: %2").arg(GetHandler()->GetPath()).arg(inIndex));
	for (int i = inIndex; i < childCount(); i++) {
		FroceRebuildChild(i);
	}
}

void QDetailWidgetPropertySequentialItem::RecreateChildren() {
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	mCount = iterable.size();
	for (int i = 0; i < iterable.size(); i++) {
		FindOrCreateChildItem(i);
	}
	while (childCount() > iterable.size()) {
		delete takeChild(iterable.size());
	}
}

void QDetailWidgetPropertySequentialItem::CreateNewItem() {
	QVariant varList = GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant var = QPropertyHandler::CreateNewVariant(mValueType);
	const void* dataPtr = coercer.coerce(var, var.metaType());
	metaSequence.addValue(containterPtr, dataPtr);
	SetValue(varList, QString("%1 Append: %2").arg(GetHandler()->GetPath()).arg(metaSequence.size(containterPtr) - 1));
	setExpanded(true);
	child(childCount() - 1)->setExpanded(true);
}

void QDetailWidgetPropertySequentialItem::FroceRebuildChild(int inIndex)
{
	QDetailWidgetPropertyItem* item = (QDetailWidgetPropertyItem*)child(inIndex);
	item->BuildContentAndChildren();
}


QWidget* QDetailWidgetPropertySequentialItem::GenerateValueWidget() {
	if (GetHandler()->GetMetaData("FixedSize").toBool())
		return nullptr;
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertySequentialItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertySequentialItem::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	RecreateChildren();
}

