#ifndef QQuickTreeViewExPrivate_h__
#define QQuickTreeViewExPrivate_h__

#include "private/qquicktreeview_p_p.h"
#include "QQuickTreeViewEx.h"

class QQuickTreeViewExPrivate : public QQuickTableViewPrivate
{
public:
	Q_DECLARE_PUBLIC(QQuickTreeViewEx)

	QQuickTreeViewExPrivate();
	~QQuickTreeViewExPrivate() override;

	static inline QQuickTreeViewExPrivate* get(QQuickTreeViewEx* q) { return q->d_func(); }

	QVariant modelImpl() const override;
	void setModelImpl(const QVariant& newModel) override;

	void initItemCallback(int serializedModelIndex, QObject* object) override;
	void itemReusedCallback(int serializedModelIndex, QObject* object) override;
	void dataChangedCallback(const QModelIndex& topLeft,
		const QModelIndex& bottomRight,
		const QVector<int>& roles);

	virtual void updateRequiredProperties(int serializedModelIndex, QObject* object, bool init);
	void updateSelection(const QRect& oldSelection, const QRect& newSelection) override;

public:
	QQmlTreeModelToTableModel m_treeModelToTableModel;
	QVariant m_assignedModel;
};

#endif // QQuickTreeViewExPrivate_h__