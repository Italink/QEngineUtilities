#ifndef QPropertyHandle_h__
#define QPropertyHandle_h__

#include "QObject"
#include "QVariant"
#include "QUndoStack"
#include "QEngineEditorAPI.h"

class IPropertyHandleImpl;
class QRowLayoutBuilder;
class QEngineUndoEntry;
class QHBoxLayout;

class QENGINEEDITOR_API QPropertyHandle: public QObject{
	Q_OBJECT
	friend class QSequentialPropertyHandleImpl;
	friend class QAssociativePropertyHandleImpl;
	friend class QObjectPropertyHandleImpl;
public:
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;

	static QPropertyHandle* Find(const QObject* inParent, const QString& inPropertyPath);
	static QPropertyHandle* FindOrCreate(QObject* inObject, const QString& inPropertyPath);
	static QPropertyHandle* FindOrCreate(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter);

	void setValue(QVariant inValue,QString bIsPushUndoStackAndWithDesc = QString());
	QVariant getValue();
	void resetValue();

	QMetaType getType();
	QString getName();
	QString getPath();
	QString getSubPath(const QString& inSubName);

	bool hasMetaData(const QString& inName) const;
	QVariant getMetaData(const QString& inName) const;
	const QVariantHash& getMetaData() const;
	bool isChanged() const { return mIsChanged; }

	QPropertyHandle* findChildHandle(const QString& inSubName);
	QPropertyHandle* createChildHandle(const QString& inSubName);
	QWidget* generateNameWidget();
	QWidget* generateValueWidget();
	void generateChildrenRow(QRowLayoutBuilder* Builder);
	void generateAttachButtonWidget(QHBoxLayout* Layout);
	struct QPropertyBinder {
		Getter mGetter;
		Setter mSetter;
	};
	template<typename OObjectType, typename... T>
	void bind(OObjectType* inAdjuster, void (OObjectType::* inNotify)(T...), Getter inGetter, Setter inSetter) {
		inSetter(getValue());
		connect(inAdjuster, inNotify, this, [this, inGetter]() {
			setValue(inGetter(), "Assign: " + getPath());
		});
		mBinderMap[inAdjuster] = QPropertyBinder{ inGetter,inSetter };
		connect(inAdjuster, &QObject::destroyed, this, [this, inAdjuster]() {
			mBinderMap.remove(inAdjuster);
		});
	}
	void refreshBinder();

	QEngineUndoEntry* getUndoEntry() const { return mUndoEntry; }

	static QVariant createNewVariant(QMetaType inOutputType, QMetaType inRealType = QMetaType());

	void setAttachButtonWidgetCallback(std::function<void(QHBoxLayout*)> val) { mAttachButtonWidgetCallback = val; }

Q_SIGNALS:
	void asValueChanged();
	void asRequestRebuildRow();
	void asChildEvent(QChildEvent*);
protected:
	QPropertyHandle(QObject* inParent, QMetaType inType, QString inPropertyPath, Getter inGetter, Setter inSetter);
	void resloveMetaData();
	bool eventFilter(QObject* object, QEvent* event) override;
protected:
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


