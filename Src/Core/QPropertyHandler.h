#ifndef QPropertyHandler_h__
#define QPropertyHandler_h__

#include "QObject"
#include "QVariant"
#include "QUndoStack"
#include "QInstance.h"

class QDetailUndoEntry;

class QPropertyHandler : public QObject{
	Q_OBJECT
public:
	using TypeId = unsigned int;;
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;

	static QPropertyHandler* FindOrCreate(QInstance* inInstance, QString inPropertyPath, QVariantHash inMetaData = QVariantHash());
	static QPropertyHandler* FindOrCreate(QObject* inOuter, TypeId inTypeID, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData = QVariantHash());

	void SetValue(QVariant inValue,QString isPushUndoStackWithDesc = QString());
	QVariant GetValue();
	void ResetValue();

	TypeId GetTypeID();
	QString GetName();
	QString GetPath();
	QString GetSubPath(const QString& inSubName);

	QVariant GetMetaData(const QString& inKey);
	const QVariantHash& GetMetaData() const;

	bool IsChanged() const { return mIsChanged; }

	struct QPropertyBinder{
		Getter mGetter;
		Setter mSetter;
	};

	template<typename OObjectType, typename... T>
	void Bind(OObjectType* inAdjuster, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(GetValue());
		connect(inAdjuster, inNotify, this, [this, inGetter]() {
			SetValue(inGetter(), "Assign: "+ GetName());
		});
		mBinderMap[inAdjuster] = QPropertyBinder{ inGetter,inSetter };
		connect(inAdjuster, &QObject::destroyed, this,[this, inAdjuster]() {
			mBinderMap.remove(inAdjuster);
		});
	}
Q_SIGNALS:
	void AsValueChanged();
private:
	QPropertyHandler(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter, QVariantHash inMetaData);
private:
	TypeId mTypeID = 0;
	QString mPath;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	QVariantHash mMetaData;
	QMap<QObject*,QPropertyBinder> mBinderMap;
	QDetailUndoEntry* mUndoEntry = nullptr;
};

#endif // QPropertyHandler_h__


