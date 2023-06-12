#ifndef QPropertyHandle_h__
#define QPropertyHandle_h__

#include "QObject"
#include "QVariant"
#include "QUndoStack"

class IPropertyHandleImpl;
class QRowLayoutBuilder;
class QEngineUndoEntry;
class QHBoxLayout;

class QPropertyHandle: public QObject{
	Q_OBJECT
	friend class QSequentialPropertyHandleImpl;
	friend class QAssociativePropertyHandleImpl;
	friend class QObjectPropertyHandleImpl;
public:
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;

	static QPropertyHandle* Find(const QObject* inParent, const QString& inPropertyPath);
	static QPropertyHandle* FindOrCreate(QObject* inObject, const QString& inPropertyPath, const QVariantHash& InMetaData = QVariantHash());
	static QPropertyHandle* FindOrCreate(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData);

	void SetValue(QVariant inValue,QString bIsPushUndoStackAndWithDesc = QString());
	QVariant GetValue();
	void ResetValue();

	QMetaType GetType();
	QString GetName();
	QString GetPath();
	QString GetSubPath(const QString& inSubName);
	QVariant GetMetaData(const QString& Hash) const;
	const QVariantHash& GetMetaData() const;
	bool IsChanged() const { return mIsChanged; }

	QPropertyHandle* FindChildHandle(const QString& inSubName);
	QPropertyHandle* CreateChildHandle(const QString& inSubName);
	QWidget* GenerateNameWidget();
	QWidget* GenerateValueWidget();
	void GenerateChildrenRow(QRowLayoutBuilder* Builder);
	void GenerateAttachButtonWidget(QHBoxLayout* Layout);
	struct QPropertyBinder {
		Getter mGetter;
		Setter mSetter;
	};
	template<typename OObjectType, typename... T>
	void Bind(OObjectType* inAdjuster, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(GetValue());
		connect(inAdjuster, inNotify, this, [this, inGetter]() {
			SetValue(inGetter(), "Assign: " + GetPath());
		});
		mBinderMap[inAdjuster] = QPropertyBinder{ inGetter,inSetter };
		connect(inAdjuster, &QObject::destroyed, this, [this, inAdjuster]() {
			mBinderMap.remove(inAdjuster);
		});
	}
	void RefreshBinder();

	QEngineUndoEntry* GetUndoEntry() const { return mUndoEntry; }

	static QVariant CreateNewVariant(QMetaType inOutputType, QMetaType inRealType = QMetaType());

	void SetAttachButtonWidgetCallback(std::function<void(QHBoxLayout*)> val) { mAttachButtonWidgetCallback = val; }

Q_SIGNALS:
	void AsValueChanged();
	void AsRequestRebuildRow();
protected:
	QPropertyHandle(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter, QVariantHash inMetaData);
	QSharedPointer<IPropertyHandleImpl> mImpl;
	QMetaType mType;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	QVariantHash mMetaData;
	QEngineUndoEntry* mUndoEntry = nullptr;
	QMap<QObject*, QPropertyBinder> mBinderMap;
	std::function<void(QHBoxLayout*)> mAttachButtonWidgetCallback;
};

#endif // QPropertyHandle_h__


