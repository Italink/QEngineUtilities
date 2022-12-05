#include "QPropertyHandler.h"
#include "QMetaObject"
#include "QMetaProperty"
#include "QTime"
#include "Undo/QDetailUndoStack.h"
#include "QRegularExpression"

QPropertyHandler::QPropertyHandler(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData)
	: QObject(inParent)
	, mType(inType)
	, mPath(inPropertyPath)
	, mGetter(inGetter)
	, mSetter(inSetter)
	, mMetaData(inMetaData)
{
	mInitialValue = inGetter();
	QDetailUndoEntry* UndoEntry = inParent->findChild<QDetailUndoEntry*>(QString(), Qt::FindDirectChildrenOnly);
	if (UndoEntry != nullptr) {
		mUndoEntry = UndoEntry;
	}
	else {
		mUndoEntry = new QDetailUndoEntry(inParent);
	}

	auto IsVisible = mMetaData.find("Visible");
	if (IsVisible != mMetaData.end()) {
		SetVisible(IsVisible->toBool());
	}
}

void QPropertyHandler::TryFlushProperty(QObject* inOuter, QString inPropertyPath) {
	QPropertyHandler* handler = QPropertyHandler::Find(inOuter, inPropertyPath);
	if (handler) {
		handler->FlushValue();
	}
}

QPropertyHandler* QPropertyHandler::Find(QObject* inOuter, QString inPropertyPath) {
	for (QObject* child : inOuter->children()) {
		QPropertyHandler* handler = qobject_cast<QPropertyHandler*>(child);
		if (handler && handler->GetPath() == inPropertyPath) {
			return handler;
		}
	}
	return nullptr;
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QInstance* inInstance, QString inPropertyName, QVariantHash inMetaData)
{
	int index = inInstance->GetMetaObject()->indexOfProperty(inPropertyName.toLocal8Bit());
	QMetaProperty metaProperty = inInstance->GetMetaObject()->property(index);
	return FindOrCreate(inInstance->GetOuterObject(),
		metaProperty.metaType(),
		inPropertyName,
		[inInstance, metaProperty]() {return inInstance->GetProperty(metaProperty); },
		[inInstance, metaProperty](QVariant var) { inInstance->SetProperty(metaProperty, var); }
	);
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inOuter, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData /*= QVariantHash()*/)
{
	for (QObject* child : inOuter->children()) {
		QPropertyHandler* handler = qobject_cast<QPropertyHandler*>(child);
		if (handler && handler->GetPath() == inPropertyPath) {
			return handler;
		}
	}
	QPropertyHandler* handler = new QPropertyHandler(
		inOuter,
		inType,
		inPropertyPath,
		inGetter,
		inSetter,
		inMetaData
	);
	return handler;
}

class QPropertyAssignCommand : public QUndoCommand {
public:
	QPropertyAssignCommand(QString inDesc, QVariant inPreValue, QVariant inPostValue, QPropertyHandler::Setter inSetter, QPropertyHandler* inHandler)
		: mPreValue(inPreValue)
		, mPostValue(inPostValue)
		, mSetter(inSetter)
		, mHandler(inHandler)
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
			if (cmd&&mHandler == cmd->mHandler) {
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
	QPropertyHandler::Setter mSetter;
	int mAssignTime = 0;
	QPropertyHandler* mHandler = nullptr;
};

void QPropertyHandler::SetValue(QVariant inValue, QString isPushUndoStackWithDesc)
{
	QVariant last = GetValue();
	if (last != inValue) {
		QPropertyHandler::Setter AssignSetter = [this](QVariant inVar) {
			if (mInitialValue.metaType().flags().testFlag(QMetaType::IsEnumeration))
				mIsChanged = inVar.toInt() != mInitialValue.toInt();
			else if(!mInitialValue.metaType().flags().testFlag(QMetaType::IsGadget)){
				mIsChanged = !(inVar == mInitialValue);
			}
			mSetter(inVar);
			Q_EMIT AsValueChanged();
		};
		if (!isPushUndoStackWithDesc.isEmpty())
			mUndoEntry->Push(new QPropertyAssignCommand(isPushUndoStackWithDesc, last, inValue, AssignSetter, this));
		else
			AssignSetter(inValue);
	}
	for (auto& binder : mBinderMap.values()) {
		if(binder.mGetter()!= inValue)
			binder.mSetter(inValue);
	}
}

QVariant QPropertyHandler::GetValue()
{
	return mGetter();
}

void QPropertyHandler::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue, "Reset: " + GetPath());
	}
}

void QPropertyHandler::FlushValue() {
	QVariant mVar = GetValue();
	for (auto& binder : mBinderMap.values()) {
		QVariant var = binder.mGetter();
		if (var != mVar) {
			binder.mSetter(mVar);
		}
	}
}

QMetaType QPropertyHandler::GetType() {
	return mType;
}

QString QPropertyHandler::GetName()
{
	return mPath.split(".").back();
}

QString QPropertyHandler::GetPath() {
	return mPath;
}

QString QPropertyHandler::GetSubPath(const QString& inSubName) {
	return mPath + "." + inSubName;
}

QVariant QPropertyHandler::GetMetaData(const QString& inKey) {
	return mMetaData.value(inKey);
}

const QVariantHash& QPropertyHandler::GetMetaData() const {
	return mMetaData;
}

bool QPropertyHandler::IsVisible() const
{
	return mIsVisible;
}

void QPropertyHandler::SetVisible(bool val)
{
	mIsVisible = val;
}

struct ExternalRefCountWithMetaType: public QtSharedPointer::ExternalRefCountData
{
	typedef ExternalRefCountData Parent;
	QMetaType mMetaType;
	void* mData;

	static void deleter(ExternalRefCountData* self)
	{
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

QVariant QPropertyHandler::CreateNewVariant(QMetaType inOutputType, QMetaType inRealType)
{
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

