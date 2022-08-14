#ifndef QInstance_h__
#define QInstance_h__

#include <QMetaObject>
#include <QMetaMethod>
#include "QMetaDataDefine.h"

class QPropertyHandler;
class QInstance_Gadget;
class QInstance_Object;


class QInstance {
public:
	static QSharedPointer<QInstance_Object> CreateObjcet(QObject* inObject);
	static QSharedPointer<QInstance_Gadget> CreateGadget(void* inPtr, const QMetaObject* inMetaObject);

	virtual const QMetaObject* GetMetaObject() = 0;
	virtual QVariant GetProperty(const QMetaProperty& inProperty) = 0;
	virtual void SetProperty(const QMetaProperty& inProperty, QVariant inVar) = 0;
	virtual QObject* GetOuterObject() = 0;
	virtual bool Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue) = 0;
	virtual void UpdateDataPtr(void* inPtr) = 0;
	bool IsValid() { return GetOuterObject() != nullptr; }

	QPropertyHandler* CreatePropertyHandler(const QMetaProperty& inProperty);

	void SetPropertyChangedCallback(std::function<void()> val) { mPropertyChangedCallback = val; }

	QVariant GetMetaData(const QString& inKey);
	void SetMetaData(const QString& inKey, QVariant inVar);
	const QList<QString>& GetCategoryList();
	const QHash<QString,QVariantHash>& GetPropertiesMetaData();
private:
	void LoadMetaData();
protected:
	std::function<void()> mPropertyChangedCallback;
	QMetaData mMetaData;
};

class QInstance_Gadget: public QInstance {
public:
	QInstance_Gadget(void* inPtr, const QMetaObject* inMetaObject);
	~QInstance_Gadget();
	virtual const QMetaObject* GetMetaObject() override;
	virtual QVariant GetProperty(const QMetaProperty& inProperty) override;
	virtual void SetProperty(const QMetaProperty& inProperty, QVariant inVar) override;
	virtual QObject* GetOuterObject() override;
	virtual bool Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue) override;
	virtual void UpdateDataPtr(void* inPtr) override;
private:
	void* mPtr = nullptr;
	const QMetaObject* mMetaObject = nullptr;
	QObject* mOuter = nullptr;
	inline static QHash<void*, QObject*> mOuterCacheMap;
};

class QInstance_Object :public QInstance {
public:
	QInstance_Object(QObject* inObject);
	virtual const QMetaObject* GetMetaObject() override;
	virtual QVariant GetProperty(const QMetaProperty& inProperty) override;
	virtual void SetProperty(const QMetaProperty& inProperty, QVariant inVar) override;
	virtual QObject* GetOuterObject() override;
	virtual bool Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue) override;
	virtual void UpdateDataPtr(void* inPtr) override;
private:
	QObject* mObject = nullptr;
};

#endif // QInstance_h__
