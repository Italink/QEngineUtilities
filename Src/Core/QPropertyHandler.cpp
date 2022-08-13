#include "QPropertyHandler.h"
#include "QMetaObject"
#include "QMetaProperty"
#include "QTime"
#include "Undo/QDetailUndoStack.h"

QPropertyHandler::QPropertyHandler(QObject* inParent, TypeId inTypeID, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData)
	: QObject(inParent)
	, mTypeID(inTypeID)
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
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QInstance* inInstance, QString inPropertyName, QVariantHash inMetaData)
{
	int index = inInstance->GetMetaObject()->indexOfProperty(inPropertyName.toLocal8Bit());
	QMetaProperty metaProperty = inInstance->GetMetaObject()->property(index);
	return FindOrCreate(inInstance->GetOuterObject(),
		metaProperty.typeId(),
		inPropertyName,
		[inInstance, metaProperty]() {return inInstance->GetProperty(metaProperty); },
		[inInstance, metaProperty](QVariant var) { inInstance->SetProperty(metaProperty, var); }
	);
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inOuter, TypeId inTypeID, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData /*= QVariantHash()*/)
{
	for (QObject* child : inOuter->children()) {
		QPropertyHandler* handler = qobject_cast<QPropertyHandler*>(child);
		if (handler && handler->GetPath() == inPropertyPath) {
			return handler;
		}
	}
	QPropertyHandler* handler = new QPropertyHandler(
		inOuter,
		inTypeID,
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
			if (mInitialValue.metaType().flags() & QMetaType::IsEnumeration)
				mIsChanged = inVar.toInt() != mInitialValue.toInt();
			else
				mIsChanged = (inVar != mInitialValue);
			mSetter(inVar);
			for (auto& binder : mBinderMap.values()) {
				QVariant var = binder.mGetter();
				if (var != inVar) {
					binder.mSetter(inVar);
				}
			}
			Q_EMIT AsValueChanged();
		};
		if (!isPushUndoStackWithDesc.isEmpty())
			mUndoEntry->Push(new QPropertyAssignCommand(isPushUndoStackWithDesc, last, inValue, AssignSetter, this));
		else
			AssignSetter(inValue);
	}
}

QVariant QPropertyHandler::GetValue()
{
	return mGetter();
}

void QPropertyHandler::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue, "Reset: " + GetName());
	}
}

QPropertyHandler::TypeId QPropertyHandler::GetTypeID() {
	return mTypeID;
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

