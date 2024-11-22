#include "QQuickTreeViewEx.h"
#include "QQuickTreeViewExPrivate.h"
#include <QtCore/qobject.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <QtQmlModels/private/qqmltreemodeltotablemodel_p_p.h>

// Hard-code the tree column to be 0 for now
static const int kTreeColumn = 0;


QQuickTreeViewExPrivate::QQuickTreeViewExPrivate()
    : QQuickTableViewPrivate()
{
}

QQuickTreeViewExPrivate::~QQuickTreeViewExPrivate()
{
}

QVariant QQuickTreeViewExPrivate::modelImpl() const
{
    return m_assignedModel;
}

void QQuickTreeViewExPrivate::setModelImpl(const QVariant& newModel)
{
    Q_Q(QQuickTreeViewEx);

    if (newModel == m_assignedModel)
        return;

    m_assignedModel = newModel;
    QVariant effectiveModel = m_assignedModel;
    if (effectiveModel.userType() == qMetaTypeId<QJSValue>())
        effectiveModel = effectiveModel.value<QJSValue>().toVariant();

    if (effectiveModel.isNull())
        m_treeModelToTableModel.setModel(nullptr);
    else if (const auto qaim = qvariant_cast<QAbstractItemModel*>(effectiveModel))
        m_treeModelToTableModel.setModel(qaim);
    else
        qmlWarning(q) << "TreeView only accepts a model of type QAbstractItemModel";


    scheduleRebuildTable(QQuickTableViewPrivate::RebuildOption::All);
    emit q->modelChanged();
}

void QQuickTreeViewExPrivate::initItemCallback(int serializedModelIndex, QObject* object)
{
	Q_Q(QQuickTreeViewEx);
    updateRequiredProperties(serializedModelIndex, object, true);
    QQuickTableViewPrivate::initItemCallback(serializedModelIndex, object);
}

void QQuickTreeViewExPrivate::itemReusedCallback(int serializedModelIndex, QObject* object)
{
    updateRequiredProperties(serializedModelIndex, object, false);
    QQuickTableViewPrivate::itemReusedCallback(serializedModelIndex, object);
}

void QQuickTreeViewExPrivate::dataChangedCallback(
    const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    Q_Q(QQuickTreeViewEx);
    Q_UNUSED(roles);

    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
            const QPoint cell(column, row);
            auto item = q->itemAtCell(cell);
            if (!item)
                continue;

            const int serializedModelIndex = modelIndexAtCell(QPoint(column, row));
            updateRequiredProperties(serializedModelIndex, item, false);
        }
    }
}

void QQuickTreeViewExPrivate::updateRequiredProperties(int serializedModelIndex, QObject* object, bool init)
{
    Q_Q(QQuickTreeViewEx);
    const QPoint cell = cellAtModelIndex(serializedModelIndex);
    const int row = cell.y();
    const int column = cell.x();

    setRequiredProperty("treeView", QVariant::fromValue(q), serializedModelIndex, object, init);
    setRequiredProperty("isTreeNode", column == kTreeColumn, serializedModelIndex, object, init);
    setRequiredProperty("hasChildren", m_treeModelToTableModel.hasChildren(row), serializedModelIndex, object, init);
    setRequiredProperty("expanded", q->isExpanded(row), serializedModelIndex, object, init);
    setRequiredProperty("depth", m_treeModelToTableModel.depthAtRow(row), serializedModelIndex, object, init);
}

void QQuickTreeViewExPrivate::updateSelection(const QRect& oldSelection, const QRect& newSelection)
{
    Q_Q(QQuickTreeViewEx);

    const QRect oldRect = oldSelection.normalized();
    const QRect newRect = newSelection.normalized();

    if (oldSelection == newSelection)
        return;

    // Select the rows inside newRect that doesn't overlap with oldRect
    for (int row = newRect.y(); row <= newRect.y() + newRect.height(); ++row) {
        if (oldRect.y() != -1 && oldRect.y() <= row && row <= oldRect.y() + oldRect.height())
            continue;
        const QModelIndex startIndex = q->index(row, newRect.x());
        const QModelIndex endIndex = q->index(row, newRect.x() + newRect.width());
        selectionModel->select(QItemSelection(startIndex, endIndex), QItemSelectionModel::Select);
    }

    if (oldRect.x() != -1) {
        // Since oldRect is valid, this update is a continuation of an already existing selection!

        // Select the columns inside newRect that don't overlap with oldRect
        for (int column = newRect.x(); column <= newRect.x() + newRect.width(); ++column) {
            if (oldRect.x() <= column && column <= oldRect.x() + oldRect.width())
                continue;
            for (int row = newRect.y(); row <= newRect.y() + newRect.height(); ++row)
                selectionModel->select(q->index(row, column), QItemSelectionModel::Select);
        }

        // Unselect the rows inside oldRect that don't overlap with newRect
        for (int row = oldRect.y(); row <= oldRect.y() + oldRect.height(); ++row) {
            if (newRect.y() <= row && row <= newRect.y() + newRect.height())
                continue;
            const QModelIndex startIndex = q->index(row, oldRect.x());
            const QModelIndex endIndex = q->index(row, oldRect.x() + oldRect.width());
            selectionModel->select(QItemSelection(startIndex, endIndex), QItemSelectionModel::Deselect);
        }

        // Unselect the columns inside oldRect that don't overlap with newRect
        for (int column = oldRect.x(); column <= oldRect.x() + oldRect.width(); ++column) {
            if (newRect.x() <= column && column <= newRect.x() + newRect.width())
                continue;
            // Since we're not allowed to call select/unselect on the selectionModel with
            // indices from different parents, and since indicies from different parents are
            // expected when working with trees, we need to unselect the indices in the column
            // one by one, rather than the whole column in one go. This, however, can cause a
            // lot of selection fragments in the selectionModel, which eventually can hurt
            // performance. But large selections containing a lot of columns is not normally
            // the case for a treeview, so accept this potential corner case for now.
            for (int row = newRect.y(); row <= newRect.y() + newRect.height(); ++row)
                selectionModel->select(q->index(row, column), QItemSelectionModel::Deselect);
        }
    }
}

QQuickTreeViewEx::QQuickTreeViewEx(QQuickItem* parent)
    : QQuickTableView(*(new QQuickTreeViewExPrivate), parent)
{
    Q_D(QQuickTreeViewEx);

    setSelectionBehavior(SelectRows);
    setEditTriggers(EditKeyPressed);

    // Note: QQuickTableView will only ever see the table model m_treeModelToTableModel, and
    // never the actual tree model that is assigned to us by the application.
    const auto modelAsVariant = QVariant::fromValue(std::addressof(d->m_treeModelToTableModel));
    d->QQuickTableViewPrivate::setModelImpl(modelAsVariant);
    QObjectPrivate::connect(&d->m_treeModelToTableModel, &QAbstractItemModel::dataChanged,
        d, &QQuickTreeViewExPrivate::dataChangedCallback);
    QObject::connect(&d->m_treeModelToTableModel, &QQmlTreeModelToTableModel::rootIndexChanged,
        this, &QQuickTreeViewEx::rootIndexChanged);

    auto tapHandler = new QQuickTapHandler(this);
    tapHandler->setAcceptedModifiers(Qt::NoModifier);
    connect(tapHandler, &QQuickTapHandler::doubleTapped, [this, tapHandler] {
        if (!pointerNavigationEnabled())
            return;
        if (editTriggers() & DoubleTapped)
            return;

        const int row = cellAtPosition(tapHandler->point().pressPosition()).y();
        toggleExpanded(row);
    });
}

QQuickTreeViewEx::QQuickTreeViewEx(QQuickTreeViewExPrivate& dd, QQuickItem* parent)
    : QQuickTableView(dd, parent)
{
	Q_D(QQuickTreeViewEx);

	setSelectionBehavior(SelectRows);
	setEditTriggers(EditKeyPressed);

	// Note: QQuickTableView will only ever see the table model m_treeModelToTableModel, and
	// never the actual tree model that is assigned to us by the application.
	const auto modelAsVariant = QVariant::fromValue(std::addressof(d->m_treeModelToTableModel));
	d->QQuickTableViewPrivate::setModelImpl(modelAsVariant);
	QObjectPrivate::connect(&d->m_treeModelToTableModel, &QAbstractItemModel::dataChanged,
		d, &QQuickTreeViewExPrivate::dataChangedCallback);
	QObject::connect(&d->m_treeModelToTableModel, &QQmlTreeModelToTableModel::rootIndexChanged,
		this, &QQuickTreeViewEx::rootIndexChanged);

	auto tapHandler = new QQuickTapHandler(this);
	tapHandler->setAcceptedModifiers(Qt::NoModifier);
	connect(tapHandler, &QQuickTapHandler::doubleTapped, [this, tapHandler] {
		if (!pointerNavigationEnabled())
			return;
		if (editTriggers() & DoubleTapped)
			return;

		const int row = cellAtPosition(tapHandler->point().pressPosition()).y();
		toggleExpanded(row);
	});
    d_func()->init();
}

QQuickTreeViewEx::~QQuickTreeViewEx()
{
}

QModelIndex QQuickTreeViewEx::rootIndex() const
{
    return d_func()->m_treeModelToTableModel.rootIndex();
}

void QQuickTreeViewEx::setRootIndex(const QModelIndex& index)
{
    Q_D(QQuickTreeViewEx);
    d->m_treeModelToTableModel.setRootIndex(index);
    positionViewAtCell({ 0, 0 }, QQuickTableView::AlignTop | QQuickTableView::AlignLeft);
}

void QQuickTreeViewEx::resetRootIndex()
{
    Q_D(QQuickTreeViewEx);
    d->m_treeModelToTableModel.resetRootIndex();
    positionViewAtCell({ 0, 0 }, QQuickTableView::AlignTop | QQuickTableView::AlignLeft);
}

int QQuickTreeViewEx::depth(int row) const
{
    Q_D(const QQuickTreeViewEx);
    if (row < 0 || row >= d->m_treeModelToTableModel.rowCount())
        return -1;

    return d->m_treeModelToTableModel.depthAtRow(row);
}

bool QQuickTreeViewEx::isExpanded(int row) const
{
    Q_D(const QQuickTreeViewEx);
    if (row < 0 || row >= d->m_treeModelToTableModel.rowCount())
        return false;

    return d->m_treeModelToTableModel.isExpanded(row);
}

void QQuickTreeViewEx::expand(int row)
{
    if (row >= 0)
        expandRecursively(row, 1);
}

void QQuickTreeViewEx::expandRecursively(int row, int depth)
{
    Q_D(QQuickTreeViewEx);
    if (row >= d->m_treeModelToTableModel.rowCount())
        return;
    if (row < 0 && row != -1)
        return;
    if (depth == 0 || depth < -1)
        return;

    auto expandRowRecursively = [this, d, depth](int startRow) {
        d->m_treeModelToTableModel.expandRecursively(startRow, depth);
        // Update the expanded state of the startRow. The descendant rows that gets
        // expanded will get the correct state set from initItem/itemReused instead.
        for (int c = leftColumn(); c <= rightColumn(); ++c) {
            const QPoint treeNodeCell(c, startRow);
            if (const auto item = itemAtCell(treeNodeCell))
                d->setRequiredProperty("expanded", true, d->modelIndexAtCell(treeNodeCell), item, false);
        }
        };

    if (row >= 0) {
        // Expand only one row recursively
        const bool isExpanded = d->m_treeModelToTableModel.isExpanded(row);
        if (isExpanded && depth == 1)
            return;
        expandRowRecursively(row);
    }
    else {
        // Expand all root nodes recursively
        const auto model = d->m_treeModelToTableModel.model();
        for (int r = 0; r < model->rowCount(); ++r) {
            const int rootRow = d->m_treeModelToTableModel.itemIndex(model->index(r, 0));
            if (rootRow != -1)
                expandRowRecursively(rootRow);
        }
    }

    emit expanded(row, depth);
}

void QQuickTreeViewEx::expandToIndex(const QModelIndex& index)
{
    Q_D(QQuickTreeViewEx);

    if (!index.isValid()) {
        qmlWarning(this) << "index is not valid: " << index;
        return;
    }

    if (index.model() != d->m_treeModelToTableModel.model()) {
        qmlWarning(this) << "index doesn't belong to correct model: " << index;
        return;
    }

    if (rowAtIndex(index) != -1) {
        // index is already visible
        return;
    }

    int depth = 1;
    QModelIndex parent = index.parent();
    int row = rowAtIndex(parent);

    while (parent.isValid()) {
        if (row != -1) {
            // The node is already visible, since it maps to a row in the table!
            d->m_treeModelToTableModel.expandRow(row);

            // Update the state of the already existing delegate item
            for (int c = leftColumn(); c <= rightColumn(); ++c) {
                const QPoint treeNodeCell(c, row);
                if (const auto item = itemAtCell(treeNodeCell))
                    d->setRequiredProperty("expanded", true, d->modelIndexAtCell(treeNodeCell), item, false);
            }

            // When we hit a node that is visible, we know that all other nodes
            // up to the parent have to be visible as well, so we can stop.
            break;
        }
        else {
            d->m_treeModelToTableModel.expand(parent);
            parent = parent.parent();
            row = rowAtIndex(parent);
            depth++;
        }
    }

    emit expanded(row, depth);
}

void QQuickTreeViewEx::collapse(int row)
{
    Q_D(QQuickTreeViewEx);
    if (row < 0 || row >= d->m_treeModelToTableModel.rowCount())
        return;

    if (!d->m_treeModelToTableModel.isExpanded(row))
        return;

    d_func()->m_treeModelToTableModel.collapseRow(row);

    for (int c = leftColumn(); c <= rightColumn(); ++c) {
        const QPoint treeNodeCell(c, row);
        if (const auto item = itemAtCell(treeNodeCell))
            d->setRequiredProperty("expanded", false, d->modelIndexAtCell(treeNodeCell), item, false);
    }

    emit collapsed(row, false);
}

void QQuickTreeViewEx::collapseRecursively(int row)
{
    Q_D(QQuickTreeViewEx);
    if (row >= d->m_treeModelToTableModel.rowCount())
        return;
    if (row < 0 && row != -1)
        return;

    auto collapseRowRecursive = [this, d](int startRow) {
        // Always collapse descendants recursively,
        // even if the top row itself is already collapsed.
        d->m_treeModelToTableModel.collapseRecursively(startRow);
        // Update the expanded state of the (still visible) startRow
        for (int c = leftColumn(); c <= rightColumn(); ++c) {
            const QPoint treeNodeCell(c, startRow);
            if (const auto item = itemAtCell(treeNodeCell))
                d->setRequiredProperty("expanded", false, d->modelIndexAtCell(treeNodeCell), item, false);
        }
        };

    if (row >= 0) {
        collapseRowRecursive(row);
    }
    else {
        // Collapse all root nodes recursively
        const auto model = d->m_treeModelToTableModel.model();
        for (int r = 0; r < model->rowCount(); ++r) {
            const int rootRow = d->m_treeModelToTableModel.itemIndex(model->index(r, 0));
            if (rootRow != -1)
                collapseRowRecursive(rootRow);
        }
    }

    emit collapsed(row, true);
}

void QQuickTreeViewEx::toggleExpanded(int row)
{
    if (isExpanded(row))
        collapse(row);
    else
        expand(row);
}

QModelIndex QQuickTreeViewEx::modelIndex(const QPoint& cell) const
{
    Q_D(const QQuickTreeViewEx);
    const QModelIndex tableIndex = d->m_treeModelToTableModel.index(cell.y(), cell.x());
    return d->m_treeModelToTableModel.mapToModel(tableIndex);
}

QPoint QQuickTreeViewEx::cellAtIndex(const QModelIndex& index) const
{
    const QModelIndex tableIndex = d_func()->m_treeModelToTableModel.mapFromModel(index);
    return QPoint(tableIndex.column(), tableIndex.row());
}

#if QT_DEPRECATED_SINCE(6, 4)
QModelIndex QQuickTreeViewEx::modelIndex(int row, int column) const
{
    static const bool compat6_4 = qEnvironmentVariable("QT_QUICK_TABLEVIEW_COMPAT_VERSION") == QStringLiteral("6.4");
    if (compat6_4) {
        // XXX Qt 7: Remove this compatibility path here and in QQuickTableView.
        // In Qt 6.4.0 and 6.4.1, a source incompatible change led to row and column
        // being documented to be specified in the opposite order.
        // QT_QUICK_TABLEVIEW_COMPAT_VERSION can therefore be set to force tableview
        // to continue accepting calls to modelIndex(column, row).
        return modelIndex({ row, column });
    }
    else {
        qmlWarning(this) << "modelIndex(row, column) is deprecated. "
            "Use index(row, column) instead. For more information, see "
            "https://doc.qt.io/qt-6/qml-qtquick-tableview-obsolete.html";
        return modelIndex({ column, row });
    }
}
#endif

void QQuickTreeViewEx::keyPressEvent(QKeyEvent* event)
{
    event->ignore();

    if (!keyNavigationEnabled())
        return;
    if (!selectionModel())
        return;

    const int row = cellAtIndex(selectionModel()->currentIndex()).y();
    switch (event->key()) {
    case Qt::Key_Left:
        collapse(row);
        event->accept();
        break;
    case Qt::Key_Right:
        expand(row);
        event->accept();
        break;
    default:
        break;
    }

    if (!event->isAccepted())
        QQuickTableView::keyPressEvent(event);
}