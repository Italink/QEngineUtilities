#ifndef QPropertyHandler_h__
#define QPropertyHandler_h__

#include "QObject"
#include "QVariant"
#include "BindingLayer/BindingLayer.h"

class QPropertyHandler : public QObject{
	Q_OBJECT
public:
	using Getter = std::function<BindingLayer::Variant()>;
	using Setter = std::function<void(BindingLayer::Variant)>;

	static QPropertyHandler* FindOrCreate(QObject* inObject, QString inPropertyName);
	static QPropertyHandler* FindOrCreate(QObject* inParent, BindingLayer::TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter);

	void SetValue(QVariant inValue);

	QVariant GetValue();

	void ResetValue();

	BindingLayer::TypeId GetTypeID();

	bool IsChanged() const { return mIsChanged; }

	QString GetName();

	struct QPropertyBinder{
		Getter mGetter;
		Setter mSetter;
	};

	template<typename OObjectType, typename... T>
	void Bind(OObjectType* inObject, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(GetValue());
		connect(inObject, inNotify, this, [this, inGetter]() {
			SetValue(inGetter());
		});
		mBinderMap[inObject] = QPropertyBinder{ inGetter,inSetter };
		connect(inObject, &QObject::destroyed, this,[this, inObject]() {
			mBinderMap.remove(inObject);
		});
	}

Q_SIGNALS:
	void AsValueChanged();
private:
	QPropertyHandler(QObject* inParent, BindingLayer::TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter);
private:
	BindingLayer::TypeId mTypeID = 0;
	QString mName;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	QMap<QObject*,QPropertyBinder> mBinderMap;
};

#endif // QPropertyHandler_h__


