#include "DetailView/QPropertyHandle.h"
#include <QBoxLayout>
#include <QEvent>
#include <QMetaObject>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QTime>
#include "DetailView/QDetailViewManager.h"
#include "DetailView/PropertyHandleImpl/QAssociativePropertyHandleImpl.h"
#include "DetailView/PropertyHandleImpl/QSequentialPropertyHandleImpl.h"
#include "Utils/QEngineUndoStack.h"
#include "Widgets/QElideLabel.h"
#include "DetailView/PropertyHandleImpl/QEnumPropertyHandleImpl.h"
#include "DetailView/PropertyHandleImpl/QObjectPropertyHandleImpl.h"

QPropertyHandle::QPropertyHandle(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter)
	: mType(inType)
	, mGetter(inGetter)
	, mSetter(inSetter)
{
	setParent(inParent);
	setObjectName(inPropertyPath);
	resloveMetaData();
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
		QRegularExpression reg("QSharedPointer\\<(.+)\\>");
		QRegularExpressionMatch match = reg.match(inType.name(), 0, QRegularExpression::MatchType::PartialPreferCompleteMatch, QRegularExpression::AnchorAtOffsetMatchOption);
		QStringList matchTexts = match.capturedTexts();
		QMetaType innerMetaType;
		if (!matchTexts.isEmpty()) {
			QString metaTypeName = matchTexts.back();
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

void QPropertyHandle::resloveMetaData() {
	auto metaObj = parent()->metaObject();
	auto firstField = getPath().split(".").first();
	for (int i = 0; i < metaObj->classInfoCount(); i++) {
		auto metaClassInfo = metaObj->classInfo(i);
		if (metaClassInfo.name() == firstField) {
			QStringList fields = QString(metaClassInfo.value()).split(",", Qt::SplitBehaviorFlags::SkipEmptyParts);
			for (auto field : fields) {
				QStringList pair = field.split("=");
				QString key, value;
				if (pair.size() > 0) {
					key = pair.first().trimmed();
				}
				if (pair.size() > 1) {
					value = pair[1].trimmed();
				}
				mMetaData[key] = value;
			}
			return;
		}
	}
}

bool QPropertyHandle::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved) {
		QChildEvent* childEvent = static_cast<QChildEvent*>(event);
		if(childEvent)
			Q_EMIT asChildEvent(childEvent);
	}
	return QObject::eventFilter(object, event);
}

QPropertyHandle* QPropertyHandle::Find(const QObject* inParent, const QString& inPropertyPath) {
	for (QObject* child : inParent->children()) {
		QPropertyHandle* handler = qobject_cast<QPropertyHandle*>(child);
		if (handler && handler->getPath() == inPropertyPath) {
			return handler;
		}
	}
	return nullptr;
}

QPropertyHandle* QPropertyHandle::FindOrCreate(QObject* inObject, const QString& inPropertyPath) {
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
		);
	}
	while (currIndex < pathList.size()) {
		const QString& currPropName = pathList[currIndex++];
		QPropertyHandle* childHandle = handle->findChildHandle(currPropName);
		if(!childHandle){
			childHandle = handle->createChildHandle(currPropName);
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

QPropertyHandle* QPropertyHandle::FindOrCreate(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter) {
	QPropertyHandle* handle = Find(inParent, inPropertyPath);
	if (handle)
		return handle;
	return new QPropertyHandle(
		inParent,
		inType,
		inPropertyPath,
		inGetter,
		inSetter
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

void QPropertyHandle::setValue(QVariant inValue, QString isPushUndoStackWithDesc){
	QVariant last = getValue();
	if (last != inValue) {
		QPropertyHandle::Setter AssignSetter = [this](QVariant inVar) {
			if (mInitialValue.metaType().flags().testFlag(QMetaType::IsEnumeration))
				mIsChanged = inVar.toInt() != mInitialValue.toInt();
			else if(!mInitialValue.metaType().flags().testFlag(QMetaType::IsGadget)){
				mIsChanged = !(inVar == mInitialValue);
			}
			mSetter(inVar);
			refreshBinder();
			Q_EMIT asValueChanged();
		};
		if (!isPushUndoStackWithDesc.isEmpty())
			mUndoEntry->push(new QPropertyAssignCommand(isPushUndoStackWithDesc, last, inValue, AssignSetter, this));
		else
			AssignSetter(inValue);
	}
}

QVariant QPropertyHandle::getValue(){
	return mGetter();
}

void QPropertyHandle::resetValue() {
	if (mIsChanged) {
		setValue(mInitialValue, "Reset: " + getPath());
	}
}

QMetaType QPropertyHandle::getType() {
	return mType;
}

QString QPropertyHandle::getName() {
	return getPath().split(".").back();
}

QString QPropertyHandle::getPath() {
	return objectName();
}

QString QPropertyHandle::getSubPath(const QString& inSubName){
	return getPath() + "." + inSubName;
}

bool QPropertyHandle::hasMetaData(const QString& inName) const
{
	return mMetaData.contains(inName);
}

QVariant QPropertyHandle::getMetaData(const QString& Hash) const {
	return mMetaData.value(Hash);
}

const QVariantHash& QPropertyHandle::getMetaData() const {
	return mMetaData;
}

QPropertyHandle* QPropertyHandle::findChildHandle(const QString& inSubName) {
	return mImpl->findChildHandle(inSubName);
}

QPropertyHandle* QPropertyHandle::createChildHandle(const QString& inSubName) {
	return mImpl->createChildHandle(inSubName);
}

QWidget* QPropertyHandle::generateNameWidget() {
	return mImpl->generateNameWidget();
}

QWidget* QPropertyHandle::generateValueWidget() {
	return mImpl->generateValueWidget();
}

void QPropertyHandle::generateChildrenRow(QRowLayoutBuilder* Builder) {
	mImpl->generateChildrenRow(Builder);
}

void QPropertyHandle::generateAttachButtonWidget(QHBoxLayout* Layout) {
	if(mAttachButtonWidgetCallback){
		mAttachButtonWidgetCallback(Layout);
	}
}

void QPropertyHandle::refreshBinder() {
	QVariant mVar = getValue();
	for (auto& binder : mBinderMap.values()) {
		QVariant var = binder.mGetter();
		if (var != mVar) {
			binder.mSetter(mVar);
		}
	}
}

QVariant QPropertyHandle::createNewVariant(QMetaType inOutputType){
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(inOutputType.name());
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		QMetaType innerMetaType = QMetaType::fromName((matchTexts.back()).toLocal8Bit());
		if (innerMetaType.isValid()) {
			void* ptr = innerMetaType.create();
			QVariant sharedPtr(inOutputType);
			memcpy(sharedPtr.data(), &ptr, sizeof(ptr));
			QtSharedPointer::ExternalRefCountData* data = ExternalRefCountWithMetaType::create(innerMetaType,ptr);
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
		QMetaType innerMetaType = QMetaType::fromName(QString(inOutputType.name()).remove("*").toLocal8Bit());
		if (innerMetaType.isValid()) {
			void* ptr = innerMetaType.create();
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

