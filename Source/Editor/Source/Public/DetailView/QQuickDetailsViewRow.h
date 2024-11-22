#ifndef QQuickDetailsViewRow_h__
#define QQuickDetailsViewRow_h__

#include <QObject>
#include <QMap>
#include <QAbstractItemModel>
#include <QMetaProperty>
#include <QQuickItem>
#include "QPropertyHandle.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailCustomization.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API IDetailsViewRow {
    friend class QQuickDetailsViewModel;
public:
    virtual QString name() = 0;
    virtual void setupItem(QQuickItem* inParent){}
    virtual void attachChildren() {}
    virtual void addChild(QSharedPointer<IDetailsViewRow> inChild);
    void clear();
protected:
    IDetailsViewRow* mParent = nullptr;
    QList<QSharedPointer<IDetailsViewRow>> mChildren;
};

class QENGINEEDITOR_API QDetailsViewRow_Root : public IDetailsViewRow {
	friend class QQuickDetailsViewModel;
public:
    virtual QString name() { return "Root"; }
	virtual void setupItem(QQuickItem* inParent) {}
	void addChild(QSharedPointer<IDetailsViewRow> inChild);
    void setObject(QObject* inObject);
    void attachChildren();
protected:
    QObject* mObject;
    QSharedPointer<IDetailCustomization> mClassLayoutCustomization;
};

class QENGINEEDITOR_API QDetailsViewRow_Property : public IDetailsViewRow {
public:
    QDetailsViewRow_Property(QPropertyHandle* inHandle);
protected:
    QString name() override { return mHandle->getName(); }
    void setupItem(QQuickItem* inParent) override;
    void attachChildren();
protected:
    QPropertyHandle* mHandle = nullptr;
    QSharedPointer<IPropertyTypeCustomization> mPropertyTypeCustomization;
    QSharedPointer<IDetailCustomization> mClassLayoutCustomization;
};

class QENGINEEDITOR_API QDetailsViewRow_Custom : public IDetailsViewRow {
public:
    QDetailsViewRow_Custom(QQuickItem* inItem);
protected:
	QString name() override { return "Custom"; }
	void setupItem(QQuickItem* inParent) override;
private:
    QQuickItem* mRowItem = nullptr;
};

#endif // QQuickDetailsViewRow_h__
