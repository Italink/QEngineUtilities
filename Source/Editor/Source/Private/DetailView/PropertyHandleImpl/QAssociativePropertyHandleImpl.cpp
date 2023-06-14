#include "DetailView/PropertyHandleImpl/QAssociativePropertyHandleImpl.h"
#include <QAssociativeIterable>
#include "DetailView/QDetailLayoutBuilder.h"
#include "Widgets/QSvgButton.h"
#include "QBoxLayout"
#include "DetailView/QPropertyHandle.h"

QAssociativePropertyHandleImpl::QAssociativePropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	QVariant varMap = mHandle->GetValue();
	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
	mMetaAssociation = iterable.metaContainer();
}

void QAssociativePropertyHandleImpl::GenerateChildrenRow(QRowLayoutBuilder* Builder) {
	QVariant varMap = mHandle->GetValue();
	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
	for (auto iter = iterable.begin(); iter != iterable.end(); ++iter) {
		QString path = mHandle->GetSubPath(iter.key().toString());
		QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
		if (handle) {
			Builder->AddProperty(handle);
		}
	}
}

QWidget* QAssociativePropertyHandleImpl::GenerateValueWidget() {
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	QObject::connect(btAppend, &QPushButton::clicked, [this](){
		QVariant varList = mHandle->GetValue();
		QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
		QString newKey = "Item0";
		int index = 0;
		while (iterable.containsKey(newKey)) {
			newKey = "Item" + QString::number(++index);
		}
		QVariant newValue = QPropertyHandle::CreateNewVariant(mMetaAssociation.mappedMetaType());
		AppendItem(newKey, newValue);
	});
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(btAppend);
	mHandle->GenerateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

QPropertyHandle* QAssociativePropertyHandleImpl::CreateChildHandle(const QString& inKey) {
	QPropertyHandle* handle = new QPropertyHandle(
		mHandle->parent(),
		mMetaAssociation.mappedMetaType(),
		mHandle->GetSubPath(inKey),
		[this, inKey]() {
		QVariant varMap = mHandle->GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		return iterable.value(inKey);
	},
		[this, inKey](QVariant var) {
		QVariant varMap = mHandle->GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		QtPrivate::QVariantTypeCoercer keyCoercer;
		QtPrivate::QVariantTypeCoercer mappedCoercer;
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		const void* dataPtr = mappedCoercer.coerce(var, var.metaType());
		mMetaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, mMetaAssociation.keyMetaType()), dataPtr);
		mHandle->SetValue(varMap);
		}
		);
	handle->SetAttachButtonWidgetCallback([inKey, this](QHBoxLayout* Layout) {
		QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
		deleteButton->setFixedSize(20, 20);
		Layout->addWidget(deleteButton);
		QObject::connect(deleteButton, &QSvgButton::clicked, [inKey, this]() {
			RemoveItem(inKey);
		});
	});
	return handle;
}

bool QAssociativePropertyHandleImpl::RenameItem(QString inSrc, QString inDst) {
	bool canRename = false;
	QVariant varMap = mHandle->GetValue();
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
		mHandle->SetValue(varMap, QString("Rename: %1 -> %2").arg(inSrc).arg(inDst));
		Q_EMIT mHandle->AsRequestRebuildRow();
	}
	return canRename;
}

void QAssociativePropertyHandleImpl::AppendItem(QString inKey, QVariant inValue) {
	QVariant varList = mHandle->GetValue();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant key(inKey);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	const void* valueDataPtr = coercer.coerce(inValue, inValue.metaType());
	//metaAssociation.insertKey(containterPtr, keyDataPtr);
	mMetaAssociation.setMappedAtKey(containterPtr, keyDataPtr, valueDataPtr);
	mHandle->SetValue(varList, QString("%1 Insert: %2").arg(mHandle->GetPath()).arg(inKey));
	Q_EMIT mHandle->AsRequestRebuildRow();
}

void QAssociativePropertyHandleImpl::RemoveItem(QString inKey) {
	QVariant varList = mHandle->GetValue();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	const QMetaAssociation metaAssociation = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant key(inKey);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	metaAssociation.removeKey(containterPtr, keyDataPtr);
	mHandle->SetValue(varList, QString("%1 Remove: %2").arg(mHandle->GetPath()).arg(inKey));
	Q_EMIT mHandle->AsRequestRebuildRow();
}

