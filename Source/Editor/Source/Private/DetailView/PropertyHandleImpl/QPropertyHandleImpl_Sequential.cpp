#include "QPropertyHandleImpl_Sequential.h"
#include <qsequentialiterable.h>
#include "QBoxLayout"
#include "QPropertyHandle.h"


QPropertyHandleImpl_Sequential::QPropertyHandleImpl_Sequential(QPropertyHandle* inHandle)
	:IPropertyHandleImpl(inHandle) {
	QVariant varList = mHandle->getVar();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	mMetaSequence = iterable.metaContainer();
}

//void QPropertyHandleImpl_Sequential::generateChildrenRow(QRowLayoutBuilder* Builder) {
//	QVariant varList = mHandle->getVar();
//	QSequentialIterable iterable = varList.value<QSequentialIterable>();
//	for(int index = 0 ; index < itemCount() ; index++){
//		QString path = mHandle->getSubPath(QString::number(index));
//		QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
//		if(handle){
//			Builder->addProperty(handle);
//		}
//	}
//}

//QWidget* QPropertyHandleImpl_Sequential::generateValueWidget() {
//	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
//	btAppend->setFixedSize(20, 20);
//	QObject::connect(btAppend, &QPushButton::clicked, [this](){
//		QVariant var = QPropertyHandle::createNewVariant(mMetaSequence.valueMetaType());
//		this->appendItem(var);
//	});
//	QWidget* valueContent = new QWidget;
//	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
//	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
//	valueContentLayout->setAlignment(Qt::AlignLeft);
//	valueContentLayout->setContentsMargins(10, 2, 10, 2);
//	valueContentLayout->setSpacing(2);
//	valueContentLayout->addWidget(btAppend);
//	mHandle->generateAttachButtonWidget(valueContentLayout);
//	return valueContent;
//}

QQuickItem* QPropertyHandleImpl_Sequential::createValueEditor(QQuickItem* inParent)
{
	return nullptr;
}

QPropertyHandle* QPropertyHandleImpl_Sequential::createChildHandle(const QString& inSubName) {
	int index = inSubName.toInt();
	QPropertyHandle* handle = QPropertyHandle::Create(
		mHandle->parent(),
		mMetaSequence.valueMetaType(),
		mHandle->createSubPath(inSubName),
		[this, index]() {
			QVariant varList = mHandle->getVar();
			QSequentialIterable iterable = varList.value<QSequentialIterable>();
			return iterable.at(index);
		},
		[this, index](QVariant var) {
			QVariant varList = mHandle->getVar();
			QSequentialIterable iterable = varList.value<QSequentialIterable>();
			const QMetaSequence metaSequence = iterable.metaContainer();
			void* containterPtr = const_cast<void*>(iterable.constIterable());
			QtPrivate::QVariantTypeCoercer coercer;
			const void* dataPtr = coercer.coerce(var, var.metaType());
			metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
			mHandle->setVar(varList);
		}
	);
	//handle->setAttachButtonWidgetCallback([this,index](QHBoxLayout* Layout) {
	//	int count = itemCount();
	//	if (index != 0) {
	//		QSvgButton* moveUp = new QSvgButton(":/Resources/up.png");
	//		moveUp->setFixedSize(20, 20);
	//		QObject::connect(moveUp, &QSvgButton::clicked,  [index, this]() {
	//			moveItem(index, index - 1);
	//		});
	//		Layout->addWidget(moveUp);
	//	}
	//	else {
	//		Layout->addSpacing(22);
	//	}
	//	if (index != count - 1) {
	//		QSvgButton* moveDown = new QSvgButton(":/Resources/down.png");
	//		moveDown->setFixedSize(20, 20);
	//		QObject::connect(moveDown, &QSvgButton::clicked, [index, this]() {
	//			moveItem(index, index + 1);
	//		});
	//		Layout->addWidget(moveDown);
	//	}
	//	else {
	//		Layout->addSpacing(22);
	//	}
	//	QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
	//	deleteButton->setFixedSize(20, 20);
	//	QObject::connect(deleteButton, &QSvgButton::clicked, [index, this]() {
	//		removeItem(index);
	//	});
	//	Layout->addWidget(deleteButton);
	//});
	return handle;
}

int QPropertyHandleImpl_Sequential::itemCount() {
	QVariant varList = mHandle->getVar();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	return iterable.size();
}

void QPropertyHandleImpl_Sequential::appendItem( QVariant InVar) {
	QVariant varList = mHandle->getVar();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	const void* dataPtr = coercer.coerce(InVar, InVar.metaType());
	metaSequence.addValue(containterPtr, dataPtr);
	//mHandle->setVar(varList, QString("%1 Append: %2").arg(mHandle->getPath()).arg(metaSequence.size(containterPtr) - 1));
	//Q_EMIT mHandle->asRequestRebuildRow();
}

void QPropertyHandleImpl_Sequential::moveItem(int InSrcIndex, int InDstIndex) {
	QVariant varList = mHandle->getVar();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant srcVar = iterable.at(InSrcIndex);
	QVariant dstVar = iterable.at(InDstIndex);
	metaSequence.setValueAtIndex(containterPtr, InDstIndex, coercer.coerce(srcVar, srcVar.metaType()));
	metaSequence.setValueAtIndex(containterPtr, InSrcIndex, coercer.coerce(dstVar, dstVar.metaType()));
	//mHandle->setVar(varList, QString("%1 Move: %2->%3").arg(mHandle->getPath()).arg(InSrcIndex).arg(InDstIndex));
	//Q_EMIT mHandle->asRequestRebuildRow();
}

void QPropertyHandleImpl_Sequential::removeItem(int InIndex) {
	QVariant varList = mHandle->getVar();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	for (int i = InIndex; i < iterable.size() - 1; i++) {
		QVariant nextVar = iterable.at(i + 1);
		metaSequence.setValueAtIndex(containterPtr, InIndex, coercer.coerce(nextVar, nextVar.metaType()));
	}
	metaSequence.removeValueAtEnd(containterPtr);
	//mHandle->setVar(varList, QString("%1 Remove: %2").arg(mHandle->getPath()).arg(InIndex));
	//Q_EMIT mHandle->asRequestRebuildRow();
}
