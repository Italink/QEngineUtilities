#include "DetailView/PropertyHandleImpl/QSequentialPropertyHandleImpl.h"
#include <qsequentialiterable.h>
#include "DetailView/QDetailLayoutBuilder.h"
#include "Widgets/QSvgButton.h"
#include "QBoxLayout"
#include "DetailView/QDetailViewManager.h"

QSequentialPropertyHandleImpl::QSequentialPropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	mMetaSequence = iterable.metaContainer();
}


void QSequentialPropertyHandleImpl::GenerateChildrenRow(QRowLayoutBuilder* Builder) {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	for(int index = 0 ; index < ItemCount() ; index++){
		QString path = mHandle->GetSubPath(QString::number(index));
		QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
		if(handle){
			Builder->AddProperty(handle);
		}
	}
}

QWidget* QSequentialPropertyHandleImpl::GenerateValueWidget() {
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	QObject::connect(btAppend, &QPushButton::clicked, [this](){
		QVariant var = QPropertyHandle::CreateNewVariant(mMetaSequence.valueMetaType());
		this->AppendItem(var);
	});
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(btAppend);
	mHandle->GenerateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

QPropertyHandle* QSequentialPropertyHandleImpl::CreateChildHandle(const QString& inSubName) {
	int index = inSubName.toInt();
	QPropertyHandle* handle = new QPropertyHandle(
		mHandle->parent(),
		mMetaSequence.valueMetaType(),
		mHandle->GetSubPath(inSubName),
		[this, index]() {
		QVariant varList = mHandle->GetValue();
		QSequentialIterable iterable = varList.value<QSequentialIterable>();
		return iterable.at(index);
		},
		[this, index](QVariant var) {
		QVariant varList = mHandle->GetValue();
		QSequentialIterable iterable = varList.value<QSequentialIterable>();
		const QMetaSequence metaSequence = iterable.metaContainer();
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		QtPrivate::QVariantTypeCoercer coercer;
		const void* dataPtr = coercer.coerce(var, var.metaType());
		metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
		mHandle->SetValue(varList);
	},
	mHandle->GetMetaData()
	);
	handle->SetAttachButtonWidgetCallback([this,index](QHBoxLayout* Layout) {
		int count = ItemCount();
		if (index != 0) {
			QSvgButton* moveUp = new QSvgButton(":/Resources/up.png");
			moveUp->setFixedSize(20, 20);
			QObject::connect(moveUp, &QSvgButton::clicked,  [index, this]() {
				MoveItem(index, index - 1);
			});
			Layout->addWidget(moveUp);
		}
		else {
			Layout->addSpacing(22);
		}
		if (index != count - 1) {
			QSvgButton* moveDown = new QSvgButton(":/Resources/down.png");
			moveDown->setFixedSize(20, 20);
			QObject::connect(moveDown, &QSvgButton::clicked, [index, this]() {
				MoveItem(index, index + 1);
			});
			Layout->addWidget(moveDown);
		}
		else {
			Layout->addSpacing(22);
		}
		QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
		deleteButton->setFixedSize(20, 20);
		QObject::connect(deleteButton, &QSvgButton::clicked, [index, this]() {
			RemoveItem(index);
		});
		Layout->addWidget(deleteButton);
	});
	return handle;
}

int QSequentialPropertyHandleImpl::ItemCount() {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	return iterable.size();
}

void QSequentialPropertyHandleImpl::AppendItem( QVariant InVar) {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	const void* dataPtr = coercer.coerce(InVar, InVar.metaType());
	metaSequence.addValue(containterPtr, dataPtr);
	mHandle->SetValue(varList, QString("%1 Append: %2").arg(mHandle->GetPath()).arg(metaSequence.size(containterPtr) - 1));
	Q_EMIT mHandle->AsRequestRebuildRow();
}

void QSequentialPropertyHandleImpl::MoveItem(int InSrcIndex, int InDstIndex) {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant srcVar = iterable.at(InSrcIndex);
	QVariant dstVar = iterable.at(InDstIndex);
	metaSequence.setValueAtIndex(containterPtr, InDstIndex, coercer.coerce(srcVar, srcVar.metaType()));
	metaSequence.setValueAtIndex(containterPtr, InSrcIndex, coercer.coerce(dstVar, dstVar.metaType()));
	mHandle->SetValue(varList, QString("%1 Move: %2->%3").arg(mHandle->GetPath()).arg(InSrcIndex).arg(InDstIndex));
	Q_EMIT mHandle->AsRequestRebuildRow();
}

void QSequentialPropertyHandleImpl::RemoveItem(int InIndex) {
	QVariant varList = mHandle->GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	for (int i = InIndex; i < iterable.size() - 1; i++) {
		QVariant nextVar = iterable.at(i + 1);
		metaSequence.setValueAtIndex(containterPtr, InIndex, coercer.coerce(nextVar, nextVar.metaType()));
	}
	metaSequence.removeValueAtEnd(containterPtr);
	mHandle->SetValue(varList, QString("%1 Remove: %2").arg(mHandle->GetPath()).arg(InIndex));
	Q_EMIT mHandle->AsRequestRebuildRow();
}
