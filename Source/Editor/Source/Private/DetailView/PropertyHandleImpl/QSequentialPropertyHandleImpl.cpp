#include "DetailView/PropertyHandleImpl/QSequentialPropertyHandleImpl.h"
#include <qsequentialiterable.h>
#include "DetailView/QDetailLayoutBuilder.h"
#include "Widgets/QSvgButton.h"
#include "QBoxLayout"
#include "DetailView/QDetailViewManager.h"

QSequentialPropertyHandleImpl::QSequentialPropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	mMetaSequence = iterable.metaContainer();
}


void QSequentialPropertyHandleImpl::generateChildrenRow(QRowLayoutBuilder* Builder) {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	for(int index = 0 ; index < itemCount() ; index++){
		QString path = mHandle->getSubPath(QString::number(index));
		QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
		if(handle){
			Builder->addProperty(handle);
		}
	}
}

QWidget* QSequentialPropertyHandleImpl::generateValueWidget() {
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	QObject::connect(btAppend, &QPushButton::clicked, [this](){
		QVariant var = QPropertyHandle::createNewVariant(mMetaSequence.valueMetaType());
		this->appendItem(var);
	});
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(btAppend);
	mHandle->generateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

QPropertyHandle* QSequentialPropertyHandleImpl::createChildHandle(const QString& inSubName) {
	int index = inSubName.toInt();
	QPropertyHandle* handle = new QPropertyHandle(
		mHandle->parent(),
		mMetaSequence.valueMetaType(),
		mHandle->getSubPath(inSubName),
		[this, index]() {
			QVariant varList = mHandle->getValue();
			QSequentialIterable iterable = varList.value<QSequentialIterable>();
			return iterable.at(index);
		},
		[this, index](QVariant var) {
			QVariant varList = mHandle->getValue();
			QSequentialIterable iterable = varList.value<QSequentialIterable>();
			const QMetaSequence metaSequence = iterable.metaContainer();
			void* containterPtr = const_cast<void*>(iterable.constIterable());
			QtPrivate::QVariantTypeCoercer coercer;
			const void* dataPtr = coercer.coerce(var, var.metaType());
			metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
			mHandle->setValue(varList);
		}
	);
	handle->setAttachButtonWidgetCallback([this,index](QHBoxLayout* Layout) {
		int count = itemCount();
		if (index != 0) {
			QSvgButton* moveUp = new QSvgButton(":/Resources/up.png");
			moveUp->setFixedSize(20, 20);
			QObject::connect(moveUp, &QSvgButton::clicked,  [index, this]() {
				moveItem(index, index - 1);
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
				moveItem(index, index + 1);
			});
			Layout->addWidget(moveDown);
		}
		else {
			Layout->addSpacing(22);
		}
		QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
		deleteButton->setFixedSize(20, 20);
		QObject::connect(deleteButton, &QSvgButton::clicked, [index, this]() {
			removeItem(index);
		});
		Layout->addWidget(deleteButton);
	});
	return handle;
}

int QSequentialPropertyHandleImpl::itemCount() {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	return iterable.size();
}

void QSequentialPropertyHandleImpl::appendItem( QVariant InVar) {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	const void* dataPtr = coercer.coerce(InVar, InVar.metaType());
	metaSequence.addValue(containterPtr, dataPtr);
	mHandle->setValue(varList, QString("%1 Append: %2").arg(mHandle->getPath()).arg(metaSequence.size(containterPtr) - 1));
	Q_EMIT mHandle->asRequestRebuildRow();
}

void QSequentialPropertyHandleImpl::moveItem(int InSrcIndex, int InDstIndex) {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant srcVar = iterable.at(InSrcIndex);
	QVariant dstVar = iterable.at(InDstIndex);
	metaSequence.setValueAtIndex(containterPtr, InDstIndex, coercer.coerce(srcVar, srcVar.metaType()));
	metaSequence.setValueAtIndex(containterPtr, InSrcIndex, coercer.coerce(dstVar, dstVar.metaType()));
	mHandle->setValue(varList, QString("%1 Move: %2->%3").arg(mHandle->getPath()).arg(InSrcIndex).arg(InDstIndex));
	Q_EMIT mHandle->asRequestRebuildRow();
}

void QSequentialPropertyHandleImpl::removeItem(int InIndex) {
	QVariant varList = mHandle->getValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	for (int i = InIndex; i < iterable.size() - 1; i++) {
		QVariant nextVar = iterable.at(i + 1);
		metaSequence.setValueAtIndex(containterPtr, InIndex, coercer.coerce(nextVar, nextVar.metaType()));
	}
	metaSequence.removeValueAtEnd(containterPtr);
	mHandle->setValue(varList, QString("%1 Remove: %2").arg(mHandle->getPath()).arg(InIndex));
	Q_EMIT mHandle->asRequestRebuildRow();
}
