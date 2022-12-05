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
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;

	static void TryFlushProperty(QObject* inOuter, QString inPropertyPath);
	static QPropertyHandler* Find(QObject* inOuter, QString inPropertyPath);
	static QPropertyHandler* FindOrCreate(QInstance* inInstance, QString inPropertyPath, QVariantHash inMetaData = QVariantHash());
	static QPropertyHandler* FindOrCreate(QObject* inOuter, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData = QVariantHash());

	void SetValue(QVariant inValue,QString isPushUndoStackWithDesc = QString());
	QVariant GetValue();
	void ResetValue();
	void FlushValue();

	QMetaType GetType();
	QString GetName();
	QString GetPath();
	QString GetSubPath(const QString& inSubName);

	QVariant GetMetaData(const QString& inKey);
	const QVariantHash& GetMetaData() const;

	bool IsVisible() const;
	void SetVisible(bool val);

	bool IsChanged() const { return mIsChanged; }

	struct QPropertyBinder{
		Getter mGetter;
		Setter mSetter;
	};

	QDetailUndoEntry* GetUndoEntry() const { return mUndoEntry; }
	template<typename OObjectType, typename... T>
	void Bind(OObjectType* inAdjuster, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(GetValue());
		connect(inAdjuster, inNotify, this, [this, inGetter]() {
			SetValue(inGetter(), "Assign: "+ GetPath());
		});
		mBinderMap[inAdjuster] = QPropertyBinder{ inGetter,inSetter };
		connect(inAdjuster, &QObject::destroyed, this,[this, inAdjuster]() {
			mBinderMap.remove(inAdjuster);
		});
	}
	static QVariant CreateNewVariant(QMetaType inOutputType, QMetaType inRealType = QMetaType());
Q_SIGNALS:
	void AsValueChanged();
	void AsVisibleChanged();
private:
	QPropertyHandler(QObject* inParent, QMetaType inType, QString inName, Getter inGetter, Setter inSetter, QVariantHash inMetaData);
private:
	QMetaType mType;
	QString mPath;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	bool mIsVisible = true;
	QVariantHash mMetaData;
	QMap<QObject*,QPropertyBinder> mBinderMap;
	QDetailUndoEntry* mUndoEntry = nullptr;
};

#endif // QPropertyHandler_h__


