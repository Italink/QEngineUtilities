#include "DetailView/QPropertyHandle.h"
#include "QBoxLayout"
#include "QMetaObject"
#include "QMetaProperty"
#include "QRegularExpression"
#include "QTime"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/PropertyHandleImpl/QAssociativePropertyHandleImpl.h"
#include "DetailView/PropertyHandleImpl/QSequentialPropertyHandleImpl.h"
#include "Undo/QEngineUndoStack.h"
#include "Widgets/QElideLabel.h"
#include "DetailView/PropertyHandleImpl/QEnumPropertyHandleImpl.h"
#include "DetailView/PropertyHandleImpl/QObjectPropertyHandleImpl.h"

QPropertyHandle::QPropertyHandle(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData)
	: mType(inType)
	, mGetter(inGetter)
	, mSetter(inSetter)
	, mMetaData(inMetaData)
{
	setParent(inParent);
	setObjectName(inPropertyPath);
	mInitialValue = inGetter();
	if (QMetaType::canConvert(inType, QMetaType::fromType<QVariantList>())
		&& !QMetaType::canConvert(inType, QMetaType::fromType<QString>())
		) {
		mImpl.reset( new QSequentialPropertyHandleImpl(this));
	}
	else if(QMetaType::canConvert(inType, QMetaType::fromType<QVariantMap>())){
		mImpl.reset(new QAssociativePropertyHandleImpl(this));
	}
	else if(inType.flags() & QMetaType::IsEnumeration){
		mImpl.reset(new QEnumPropertyHandleImpl(this));
	}
	else{
		QRegularExpression reg("(QSharedPointer|std::shared_ptr|shared_ptr)\\<(.+)\\>");
		QRegularExpressionMatch match = reg.match(inType.name(), 0, QRegularExpression::MatchType::PartialPreferCompleteMatch, QRegularExpression::AnchorAtOffsetMatchOption);
		QStringList matchTexts = match.capturedTexts();
		QMetaType innerMetaType;
		if (!matchTexts.isEmpty()) {
			QString metaTypeName = matchTexts.back() + "*";
			innerMetaType = QMetaType::fromName(metaTypeName.toLocal8Bit());
		}
		if(innerMetaType.metaObject()||inType.metaObject()){
			mImpl.reset(new QObjectPropertyHandleImpl(this));
		}
		else{
			mImpl.reset(new IPropertyHandleImpl(this));
		}
	}
	QEngineUndoEntry* UndoEntry = inParent->findChild<QEngineUndoEntry*>(QString(), Qt::FindDirectChildrenOnly);
	if (UndoEntry != nullptr) {
		mUndoEntry = UndoEntry;
	}
	else {
		mUndoEntry = new QEngineUndoEntry(inParent);
	}
}

QPropertyHandle* QPropertyHandle::Find(const QObject* inParent, const QString& inPropertyPath) {
	for (QObject* child : inParent->children()) {
		QPropertyHandle* handler = qobject_cast<QPropertyHandle*>(child);
		if (handler && handler->GetPath() == inPropertyPath) {
			return handler;
		}
	}
	return nullptr;
}

QPropertyHandle* QPropertyHandle::FindOrCreate(QObject* inObject, const QString& inPropertyPath, const QVariantHash& InMetaData) {
	QPropertyHandle* handle = Find(inObject, inPropertyPath);
	if (handle)
		return handle;
	QStringList pathList = inPropertyPath.split(".");
	int currIndex = 0;
	QString toplevelProp = pathList[currIndex++].toLocal8Bit();
	handle = Find(inObject, toplevelProp);
	if(!handle){
		int index = inObject->metaObject()->indexOfProperty(toplevelProp.toLocal8Bit());
		if (index < 0)
			return nullptr;
		QMetaProperty metaProperty = inObject->metaObject()->property(index);
		QMetaType metaType = metaProperty.metaType();
		handle = new QPropertyHandle(
			inObject,
			metaProperty.metaType(),
			inPropertyPath,
			[inObject, metaProperty]() {return metaProperty.read(inObject); },
			[inObject, metaProperty](QVariant var) {  metaProperty.write(inObject, var); }
			, InMetaData
		);
	}
	while (currIndex < pathList.size()) {
		const QString& currPropName = pathList[currIndex++];
		QPropertyHandle* childHandle = handle->FindChildHandle(currPropName);
		if(!childHandle){
			childHandle = handle->CreateChildHandle(currPropName);
		}
		if(childHandle){
			handle = childHandle;
		}
		else{
			return nullptr;
		}
	}
	return handle;
}

QPropertyHandle* QPropertyHandle::FindOrCreate(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData) {
	QPropertyHandle* handle = Find(inParent, inPropertyPath);
	if (handle)
		return handle;
	return new QPropertyHandle(
		inParent,
		inType,
		inPropertyPath,
		inGetter,
		inSetter,
		inMetaData
	);
}

class QPropertyAssignCommand : public QUndoCommand {
public:
	QPropertyAssignCommand(QString inDesc, QVariant inPreValue, QVariant inPostValue, QPropertyHandle::Setter inSetter, QPropertyHandle* inHandle)
		: mPreValue(inPreValue)
		, mPostValue(inPostValue)
		, mSetter(inSetter)
		, mHandle(inHandle)
{
		setText(inDesc);
		mAssignTime = QTime::currentTime().msecsSinceStartOfDay();
	}
protected:
	virtual void undo() override {
		mSetter(mPreValue);
	}
	virtual void redo() override {
		mSetter(mPostValue);
	}
	virtual int id() const override {
		return 100001;
	}

	virtual bool mergeWith(const QUndoCommand* other) override {
		if ( other->text() == other->text()) {
			const QPropertyAssignCommand* cmd = static_cast<const QPropertyAssignCommand*>(other);
			if (cmd&&mHandle == cmd->mHandle) {
				if (cmd->mAssignTime - mAssignTime < 500) {
					mAssignTime = cmd->mAssignTime;
					mPostValue = cmd->mPostValue;
					return true;
				}
			}
		}
		return false;
	}

	QVariant mPreValue;
	QVariant mPostValue;
	QPropertyHandle::Setter mSetter;
	int mAssignTime = 0;
	QPropertyHandle* mHandle = nullptr;
};

void QPropertyHandle::SetValue(QVariant inValue, QString isPushUndoStackWithDesc){
	QVariant last = GetValue();
	if (last != inValue) {
		QPropertyHandle::Setter AssignSetter = [this](QVariant inVar) {
			if (mInitialValue.metaType().flags().testFlag(QMetaType::IsEnumeration))
				mIsChanged = inVar.toInt() != mInitialValue.toInt();
			else if(!mInitialValue.metaType().flags().testFlag(QMetaType::IsGadget)){
				mIsChanged = !(inVar == mInitialValue);
			}
			mSetter(inVar);
			RefreshBinder();
			Q_EMIT AsValueChanged();
		};
		if (!isPushUndoStackWithDesc.isEmpty())
			mUndoEntry->Push(new QPropertyAssignCommand(isPushUndoStackWithDesc, last, inValue, AssignSetter, this));
		else
			AssignSetter(inValue);
	}
}

QVariant QPropertyHandle::GetValue(){
	return mGetter();
}

void QPropertyHandle::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue, "Reset: " + GetPath());
	}
}

QMetaType QPropertyHandle::GetType() {
	return mType;
}

QString QPropertyHandle::GetName() {
	return GetPath().split(".").back();
}

QString QPropertyHandle::GetPath() {
	return objectName();
}

QString QPropertyHandle::GetSubPath(const QString& inSubName){
	return GetPath() + "." + inSubName;
}

QVariant QPropertyHandle::GetMetaData(const QString& Hash) const {
	return mMetaData.value(Hash);
}

const QVariantHash& QPropertyHandle::GetMetaData() const {
	return mMetaData;
}

QPropertyHandle* QPropertyHandle::FindChildHandle(const QString& inSubName) {
	return mImpl->FindChildHandle(inSubName);
}

QPropertyHandle* QPropertyHandle::CreateChildHandle(const QString& inSubName) {
	return mImpl->CreateChildHandle(inSubName);
}

QWidget* QPropertyHandle::GenerateNameWidget() {
	return mImpl->GenerateNameWidget();
}

QWidget* QPropertyHandle::GenerateValueWidget() {
	return mImpl->GenerateValueWidget();
}

void QPropertyHandle::GenerateChildrenRow(QRowLayoutBuilder* Builder) {
	mImpl->GenerateChildrenRow(Builder);
}

void QPropertyHandle::GenerateAttachButtonWidget(QHBoxLayout* Layout) {
	if(mAttachButtonWidgetCallback){
		mAttachButtonWidgetCallback(Layout);
	}
}

struct ExternalRefCountWithMetaType: public QtSharedPointer::ExternalRefCountData{
	typedef ExternalRefCountData Parent;
	QMetaType mMetaType;
	void* mData;

	static void deleter(ExternalRefCountData* self){
		ExternalRefCountWithMetaType* that =
			static_cast<ExternalRefCountWithMetaType*>(self);
		that->mMetaType.destroy(that->mData);
		Q_UNUSED(that); // MSVC warns if T has a trivial destructor
	}

	static inline ExternalRefCountData* create(QMetaType inMetaType,void* inPtr)
	{
		ExternalRefCountWithMetaType* d = static_cast<ExternalRefCountWithMetaType*>(::operator new(sizeof(ExternalRefCountWithMetaType)));

		// initialize the d-pointer sub-object
		// leave d->data uninitialized
		new (d) Parent(ExternalRefCountWithMetaType::deleter); // can't throw
		d->mData = inPtr;
		d->mMetaType = inMetaType;
		return d;
	}
};

void QPropertyHandle::RefreshBinder() {
	QVariant mVar = GetValue();
	for (auto& binder : mBinderMap.values()) {
		QVariant var = binder.mGetter();
		if (var != mVar) {
			binder.mSetter(mVar);
		}
	}
}

QVariant QPropertyHandle::CreateNewVariant(QMetaType inOutputType, QMetaType inRealType){
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(inOutputType.name());
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		if(!inRealType.isValid())
			inRealType = QMetaType::fromName((matchTexts.back()).toLocal8Bit());
		if (inRealType.isValid()) {
			void* ptr = inRealType.create();
			QVariant sharedPtr(inOutputType);
			memcpy(sharedPtr.data(), &ptr, sizeof(ptr));
			QtSharedPointer::ExternalRefCountData* data = ExternalRefCountWithMetaType::create(inRealType,ptr);
			memcpy((char*)sharedPtr.data() + sizeof(ptr), &data, sizeof(data));
			return sharedPtr;
		}
	}
	else if (inOutputType.flags().testFlag(QMetaType::IsPointer)) {
		const QMetaObject* metaObject = inOutputType.metaObject();
		if (metaObject && metaObject->inherits(&QObject::staticMetaObject)) {
			QObject* obj = metaObject->newInstance();
			if (obj)
				return QVariant::fromValue(obj);
		}
		if (!inRealType.isValid()) {
			inRealType = QMetaType::fromName(QString(inOutputType.name()).remove("*").toLocal8Bit());
		}
		if (inRealType.isValid()) {
			void* ptr = inRealType.create();
			QVariant var(inOutputType, ptr);
			memcpy(var.data(), &ptr, sizeof(ptr));
			return var;
		}
		else {
			return QVariant();
		}
	}
	return QVariant(inOutputType);
}

