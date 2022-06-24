#ifndef QPropertyHandler_h__
#define QPropertyHandler_h__

#include "QObject"
#include "QVariant"

class QPropertyHandler : public QObject{
	Q_OBJECT
public:
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;
	using TypeId = int;

	static QPropertyHandler* FindOrCreate(QObject* inObject, QString inPropertyName);
	static QPropertyHandler* FindOrCreate(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter);

	void SetValue(QVariant inValue);

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
	void Bind(OObjectType* inObject, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		connect(inObject, inNotify, this, [this, inGetter]() {
			SetValue(inGetter());
			});
		mBinderList << QPropertyBinder{ inGetter,inSetter };
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
	QList<QPropertyBinder> mBinderList;
};

#endif // QPropertyHandler_h__


