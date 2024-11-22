#ifndef QQuickTreeViewEx_h__
#define QQuickTreeViewEx_h__

#include "QEngineEditorAPI.h"
#include "private/qquicktableview_p.h"

class QQuickTreeViewExPrivate;

class QENGINEEDITOR_API QQuickTreeViewEx: public QQuickTableView {
	Q_OBJECT
public:
	QQuickTreeViewEx(QQuickItem* parent = nullptr);
	~QQuickTreeViewEx() override;

	QModelIndex rootIndex() const;
	void setRootIndex(const QModelIndex& index);
	void resetRootIndex();

	Q_INVOKABLE int depth(int row) const;

	Q_INVOKABLE bool isExpanded(int row) const;
	Q_INVOKABLE void expand(int row);
	Q_INVOKABLE void collapse(int row);
	Q_INVOKABLE void toggleExpanded(int row);

	Q_REVISION(6, 4) Q_INVOKABLE void expandRecursively(int row = -1, int depth = -1);
	Q_REVISION(6, 4) Q_INVOKABLE void collapseRecursively(int row = -1);
	Q_REVISION(6, 4) Q_INVOKABLE void expandToIndex(const QModelIndex& index);

	Q_INVOKABLE QModelIndex modelIndex(const QPoint& cell) const override;
	Q_INVOKABLE QPoint cellAtIndex(const QModelIndex& index) const override;

#if QT_DEPRECATED_SINCE(6, 4)
	QT_DEPRECATED_VERSION_X_6_4("Use index(row, column) instead")
	Q_REVISION(6, 4) Q_INVOKABLE QModelIndex modelIndex(int row, int column) const override;
#endif

Q_SIGNALS:
	void expanded(int row, int depth);
	void collapsed(int row, bool recursively);
	Q_REVISION(6, 6) void rootIndexChanged();
protected:
	QQuickTreeViewEx(QQuickTreeViewExPrivate& dd, QQuickItem* parent);
	void keyPressEvent(QKeyEvent* event) override;
private:
	Q_DISABLE_COPY(QQuickTreeViewEx)
	Q_DECLARE_PRIVATE(QQuickTreeViewEx)
};

QML_DECLARE_TYPE(QQuickTreeViewEx)

#endif // QQuickTreeViewEx_h__
