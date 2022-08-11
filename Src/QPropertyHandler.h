#ifndef QPropertyHandler_h__
#define QPropertyHandler_h__

#include "QObject"
#include "QVariant"
#include "QUndoStack"

class QDetailUndoEntry;

class QPropertyHandler : public QObject{
	Q_OBJECT
public:
	using TypeId = unsigned int;;
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;

	static QPropertyHandler* FindOrCreate(QObject* inObject, QString inPropertyName);
	static QPropertyHandler* FindOrCreate(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter);

	void SetValue(QVariant inValue, bool bPushUndoStack = false);
	QVariant GetValue();
	void ResetValue();

	TypeId GetTypeID();
	bool IsChanged() const { return mIsChanged; }
	QString GetName();

	struct QPropertyBinder{
		Getter mGetter;
		Setter mSetter;
	};

	template<typename OObjectType, typename... T>
	void Bind(OObjectType* inAdjuster, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(GetValue());
		connect(inAdjuster, inNotify, this, [this, inGetter]() {
			SetValue(inGetter(), true);
		});
		mBinderMap[inAdjuster] = QPropertyBinder{ inGetter,inSetter };
		connect(inAdjuster, &QObject::destroyed, this,[this, inAdjuster]() {
			mBinderMap.remove(inAdjuster);
		});
	}

Q_SIGNALS:
	void AsValueChanged();
private:
	QPropertyHandler(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter);
private:
	TypeId mTypeID = 0;
	QString mName;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	QMap<QObject*,QPropertyBinder> mBinderMap;
	QDetailUndoEntry* mUndoEntry = nullptr;
};

#endif // QPropertyHandler_h__


