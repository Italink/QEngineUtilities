#include "QQuickDetailsViewModel.h"
#include "QQuickDetailsViewRow.h"
#include "QQuickDetailsViewLayoutBuilder.h"

QQuickDetailsViewModel::QQuickDetailsViewModel(QObject* parent)
	: QAbstractItemModel(parent) 
    , mRoot(new QDetailsViewRow_Root)
{
}

QVariant QQuickDetailsViewModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    IDetailsViewRow* node = static_cast<IDetailsViewRow*>(index.internalPointer());
    return node->name();
}

Qt::ItemFlags QQuickDetailsViewModel::flags(const QModelIndex& index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex QQuickDetailsViewModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    if (!parent.isValid()) {
        return createIndex(row, column, mRoot->mChildren[row].get());
    }
    return createIndex(row, column, mRoot->mChildren[parent.row()]->mChildren[row].get());
}

QModelIndex QQuickDetailsViewModel::parent(const QModelIndex& index) const {
    if (!index.isValid())
        return QModelIndex();
    IDetailsViewRow* node = static_cast<IDetailsViewRow*>(index.internalPointer());
    if (node->mParent) {
        return createIndex(mRoot->mChildren.indexOf(node->mParent), 0, node->mParent);
    }
    return QModelIndex();
}

int QQuickDetailsViewModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid())
        return mRoot->mChildren.size();
    IDetailsViewRow* node = static_cast<IDetailsViewRow*>(parent.internalPointer());
    return node->mChildren.count();
}

void QQuickDetailsViewModel::setObject(QObject* inObject)
{
    mObject = inObject;
    rebuildNode();
}

QObject* QQuickDetailsViewModel::getObject() const
{
    return mObject;
}

void QQuickDetailsViewModel::rebuildNode()
{
    beginResetModel();
    mRoot->clear();
    mRoot->setObject(mObject);
    mRoot->attachChildren();
    endResetModel();
}

int QQuickDetailsViewModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QHash<int, QByteArray> QQuickDetailsViewModel::roleNames() const {
    return {
        { Roles::name,"name" },
    };
}
