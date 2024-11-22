
#include "QPropertyHandleImpl_Associative.h"
#include <QAssociativeIterable>
#include "QBoxLayout"
#include "QPropertyHandle.h"

QPropertyHandleImpl_Associative::QPropertyHandleImpl_Associative(QPropertyHandle* inHandle)
	:IPropertyHandleImpl(inHandle) {
	QVariant varMap = mHandle->getVar();
	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
	mMetaAssociation = iterable.metaContainer();
}

//void QPropertyHandleImpl_Associative::generateChildrenRow(QRowLayoutBuilder* Builder) {
//	QVariant varMap = mHandle->getVar();
//	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
//	for (auto iter = iterable.begin(); iter != iterable.end(); ++iter) {
//		QString path = mHandle->getSubPath(iter.key().toString());
//		QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
//		if (handle) {
//			Builder->addProperty(handle);
//		}
//	}
//}

//QWidget* QPropertyHandleImpl_Associative::generateValueWidget() {
//	if (mHandle->hasMetaData("FixedSize")) {
//		return IPropertyHandleImpl::generateValueWidget();
//	}
//	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
//	btAppend->setFixedSize(20, 20);
//	QObject::connect(btAppend, &QPushButton::clicked, [this](){
//		QVariant varList = mHandle->getVar();
//		QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
//		QString newKey = "Item0";
//		int index = 0;
//		while (iterable.containsKey(newKey)) {
//			newKey = "Item" + QString::number(++index);
//		}
//		QVariant newValue = QPropertyHandle::createNewVariant(mMetaAssociation.mappedMetaType());
//		appendItem(newKey, newValue);
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

QQuickItem* QPropertyHandleImpl_Associative::createValueEditor(QQuickItem* inParent)
{
	return nullptr;
}

QPropertyHandle* QPropertyHandleImpl_Associative::createChildHandle(const QString& inKey) {
	QPropertyHandle* handle = QPropertyHandle::Create(
		mHandle->parent(),
		mMetaAssociation.mappedMetaType(),
		mHandle->createSubPath(inKey),
		[this, inKey]() {
			QVariant varMap = mHandle->getVar();
			QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
			return iterable.value(inKey);
		},
		[this, inKey](QVariant var) {
			QVariant varMap = mHandle->getVar();
			QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
			QtPrivate::QVariantTypeCoercer keyCoercer;
			QtPrivate::QVariantTypeCoercer mappedCoercer;
			void* containterPtr = const_cast<void*>(iterable.constIterable());
			const void* dataPtr = mappedCoercer.coerce(var, var.metaType());
			mMetaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, mMetaAssociation.keyMetaType()), dataPtr);
			mHandle->setVar(varMap);
		}
	);
	//if (!mHandle->hasMetaData("FixedSize")) {
	//	handle->setAttachButtonWidgetCallback([inKey, this](QHBoxLayout* Layout) {
	//		QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
	//		deleteButton->setFixedSize(20, 20);
	//		Layout->addWidget(deleteButton);
	//		QObject::connect(deleteButton, &QSvgButton::clicked, [inKey, this]() {
	//			removeItem(inKey);
	//			});
	//		});
	//}
	return handle;
}

bool QPropertyHandleImpl_Associative::renameItem(QString inSrc, QString inDst) {
	bool canRename = false;
	QVariant varMap = mHandle->getVar();
	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
	if (iterable.containsKey(inSrc) && !iterable.containsKey(inDst)) {
		canRename = true;
		QVariant var = iterable.value(inSrc);
		QtPrivate::QVariantTypeCoercer keyCoercer;
		QtPrivate::QVariantTypeCoercer mappedCoercer;
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		QMetaAssociation metaAssociation = iterable.metaContainer();
		metaAssociation.removeKey(containterPtr, keyCoercer.coerce(inSrc, QMetaType::fromType<QString>()));
		metaAssociation.setMappedAtKey(
			containterPtr,
			keyCoercer.coerce(inDst, QMetaType::fromType<QString>()),
			mappedCoercer.coerce(var, var.metaType())
		);
		//mHandle->setVar(varMap, QString("Rename: %1 -> %2").arg(inSrc).arg(inDst));
		//Q_EMIT mHandle->asRequestRebuildRow();
	}
	return canRename;
}

void QPropertyHandleImpl_Associative::appendItem(QString inKey, QVariant inValue) {
	QVariant varList = mHandle->getVar();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant key(inKey);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	const void* valueDataPtr = coercer.coerce(inValue, inValue.metaType());
	//metaAssociation.insertKey(containterPtr, keyDataPtr);
	mMetaAssociation.setMappedAtKey(containterPtr, keyDataPtr, valueDataPtr);
	//mHandle->setVar(varList, QString("%1 Insert: %2").arg(mHandle->getPath()).arg(inKey));
	//Q_EMIT mHandle->asRequestRebuildRow();
}

void QPropertyHandleImpl_Associative::removeItem(QString inKey) {
	QVariant varList = mHandle->getVar();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	const QMetaAssociation metaAssociation = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant key(inKey);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	metaAssociation.removeKey(containterPtr, keyDataPtr);
	//mHandle->setVar(varList, QString("%1 Remove: %2").arg(mHandle->getPath()).arg(inKey));
	//Q_EMIT mHandle->asRequestRebuildRow();
}

