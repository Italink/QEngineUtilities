#ifndef QInstance_h__
#define QInstance_h__

#include <QMetaObject>
#include <QMetaMethod>

class QInstance {
public:
	virtual const QMetaObject* GetMetaObject() = 0;
	virtual QVariant GetProperty(const QMetaProperty& inProperty) = 0;
	virtual void SetProperty(const QMetaProperty& inProperty, QVariant inVar) = 0;
	virtual QObject* GetOuterObject() = 0;
	virtual bool Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue) = 0;
};

class QInstance_Gadget: public QInstance {
public:
	QInstance_Gadget(void* inPtr,const QMetaObject* inMetaObject);
	~QInstance_Gadget();
	virtual const QMetaObject* GetMetaObject() override;
	virtual QVariant GetProperty(const QMetaProperty& inProperty) override;
	virtual void SetProperty(const QMetaProperty& inProperty, QVariant inVar) override;
	virtual QObject* GetOuterObject() override;
	virtual bool Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue) override;
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
private:
	QObject* mObject = nullptr;
};

#endif // QInstance_h__
