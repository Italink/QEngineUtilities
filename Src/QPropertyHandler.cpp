#include "QPropertyHandler.h"
#include "QMetaObject"
#include "QMetaProperty"
#include "QTime"
#include "Undo/QDetailUndoStack.h"

QPropertyHandler::QPropertyHandler(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter)
	:QObject(inParent)
{
	mTypeID = inTypeID;
	mName = inName;
	mGetter = inGetter;
	mSetter = inSetter;
	mInitialValue = inGetter();
	QDetailUndoEntry* UndoEntry = inParent->findChild<QDetailUndoEntry*>(QString(), Qt::FindDirectChildrenOnly);
	if (UndoEntry != nullptr) {
		mUndoEntry = UndoEntry;
	}
	else {
		mUndoEntry = new QDetailUndoEntry(inParent);
	}
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inObject, QString inPropertyName)
{
	int index = inObject->metaObject()->indexOfProperty(inPropertyName.toLocal8Bit());
	QMetaProperty metaProperty = inObject->metaObject()->property(index);
	return FindOrCreate(inObject,
		metaProperty.typeId(),
		inPropertyName,
		[Object = inObject, metaProperty]() {return metaProperty.read(Object); },
		[Object = inObject, metaProperty](QVariant var) { metaProperty.write(Object, var); }
	);
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter)
{
	for (QObject* child : inParent->children()) {
		QPropertyHandler* handler = qobject_cast<QPropertyHandler*>(child);
		if (handler && handler->GetName() == inName) {
			return handler;
		}
	}
	QPropertyHandler* handler = new QPropertyHandler(
		inParent,
		inTypeID,
		inName,
		inGetter,
		inSetter
	);
	return handler;
}

class QPropertyAssignCommand : public QUndoCommand {
public:
	QPropertyAssignCommand(QString inPropertyName, QVariant inPreValue, QVariant inPostValue, QPropertyHandler::Setter inSetter)
		: mPreValue(inPreValue)
		, mPostValue(inPostValue)
		, mSetter(inSetter)
	{
		setText("Assign: " + inPropertyName);
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
		if (other->text() == other->text()) {
			const QPropertyAssignCommand* cmd = static_cast<const QPropertyAssignCommand*>(other);
			if (cmd->mAssignTime - mAssignTime < 200) {
				mAssignTime = cmd->mAssignTime;
				mPostValue = cmd->mPostValue;
				return true;
			}
		}
		return false;
	}

	QVariant mPreValue;
	QVariant mPostValue;
	QPropertyHandler::Setter mSetter;
	int mAssignTime = 0;
};

void QPropertyHandler::SetValue(QVariant value, bool bPushUndoStack)
{
	QVariant last = GetValue();
	if (last != value) {
		if (mInitialValue.metaType().flags() & QMetaType::IsEnumeration ) 
			mIsChanged = value.toInt() != mInitialValue.toInt();
		else 
			mIsChanged = (value != mInitialValue);
		
		QPropertyHandler::Setter AssignSetter = [this](QVariant inVar) {
			mSetter(inVar);
			for (auto& binder : mBinderMap.values()) {
				QVariant var = binder.mGetter();
				if (var != inVar) {
					binder.mSetter(inVar);
				}
			}
		};
		if (bPushUndoStack) 
			mUndoEntry->Push(new QPropertyAssignCommand(GetName(), last, value, AssignSetter));
		else 
			AssignSetter(value);
		Q_EMIT AsValueChanged();
	}
}

QVariant QPropertyHandler::GetValue()
{
	return mGetter();
}

void QPropertyHandler::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue);
	}
}

QPropertyHandler::TypeId QPropertyHandler::GetTypeID() {
	return mTypeID;
}

QString QPropertyHandler::GetName()
{
	return mName;
}

