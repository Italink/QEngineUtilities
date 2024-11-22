#ifndef QQuickDetailsViewModel_h__
#define QQuickDetailsViewModel_h__

#include <QObject>
#include <QMap>
#include <QAbstractItemModel>
#include <QMetaProperty>
#include <QQuickItem>
#include "QEngineEditorAPI.h"

class QDetailsViewRow_Root;

class QENGINEEDITOR_API QQuickDetailsViewModel : public QAbstractItemModel {
    Q_OBJECT
        enum Roles {
        name = 0,
    };
public:
    QQuickDetailsViewModel(QObject* parent = 0);
    ~QQuickDetailsViewModel() {}
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setObject(QObject* inObject);
    QObject* getObject() const;
    void rebuildNode();
private:
    QSharedPointer<QDetailsViewRow_Root> mRoot;
    QObject* mObject;
};

#endif // QQuickDetailsViewModel_h__
