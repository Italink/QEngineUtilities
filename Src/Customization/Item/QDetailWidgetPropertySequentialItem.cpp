#include "QDetailWidgetPropertySequentialItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "Customization\QDetailWidgetManager.h"
#include "QPushButton"
#include "Widgets\Toolkits\QSvgButton.h"

QDetailWidgetPropertySequentialItem::QDetailWidgetPropertySequentialItem() {
}

bool QDetailWidgetPropertySequentialItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantList>())
		&& !QMetaType::canConvert(metaType, QMetaType::fromType<QString>())
		) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertySequentialItem::SetHandler(QPropertyHandler* inHandler) {
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	mValueTypeId = iterable.valueMetaType().id();
	connect(GetHandler(), &QPropertyHandler::AsValueChanged, this, &QDetailWidgetPropertySequentialItem::RecreateChildren);
}

void QDetailWidgetPropertySequentialItem::ResetValue() {
	QDetailWidgetPropertyItem::ResetValue();
}

void QDetailWidgetPropertySequentialItem::FindOrCreateChildItem(int index) {
	QPropertyHandler* handler = QPropertyHandler::FindOrCreate(
		GetParentObject(),
		mValueTypeId,
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
			item->SetBuildContentAndChildrenCallback([item,this,index]() {
				if (index != 0) {
					QSvgButton* moveUp = new QSvgButton(":/Resources/up.png");
					moveUp->setFixedSize(20, 20);
					item->AddValueWidget(moveUp);
				}
				else {
					item->GetContent()->GetValueContentLayout()->addSpacing(20);
				}
				if (index != mCount - 1) {
					QSvgButton* moveDown = new QSvgButton(":/Resources/down.png");
					moveDown->setFixedSize(20, 20);
					item->AddValueWidget(moveDown);	
				}
				else {
					item->GetContent()->GetValueContentLayout()->addSpacing(20);
				}
				QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
				deleteButton->setFixedSize(20, 20);
				item->AddValueWidget(deleteButton);
			});
			item->AttachTo(this);
		}
	}
}

void QDetailWidgetPropertySequentialItem::RecreateChildren() {
	Clear();
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	mCount = iterable.size();
	for (int i = 0; i < iterable.size(); i++) {
		FindOrCreateChildItem(i);
	}
}

void QDetailWidgetPropertySequentialItem::CreateNewItem() {
	QVariant varList = GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant var = QPropertyHandler::CreateNewVariant(mValueTypeId);
	const void* dataPtr = coercer.coerce(var, var.metaType());
	metaSequence.addValue(containterPtr, dataPtr);

	SetValue(varList, QString("%1 Append %2").arg(GetHandler()->GetPath()).arg(metaSequence.size(containterPtr) - 1));
	setExpanded(true);
	child(childCount() - 1)->setExpanded(true);
}

QWidget* QDetailWidgetPropertySequentialItem::GenerateValueWidget() {
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertySequentialItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertySequentialItem::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	RecreateChildren();
}

