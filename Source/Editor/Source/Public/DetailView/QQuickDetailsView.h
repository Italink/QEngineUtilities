#ifndef QQuickDetailsView_h__
#define QQuickDetailsView_h__

#include "QQuickTreeViewEx.h"

class QQuickDetailsViewPrivate;

class QENGINEEDITOR_API QQuickDetailsView: public QQuickTreeViewEx {
	Q_OBJECT
	QML_NAMED_ELEMENT(DetailsView)
	Q_DISABLE_COPY(QQuickDetailsView)
	Q_DECLARE_PRIVATE(QQuickDetailsView)
	Q_PROPERTY(qreal SpliterPencent READ getSpliterPencent WRITE setSpliterPencent NOTIFY asSpliterPencentChanged FINAL)
	Q_PROPERTY(QObject* Object READ getObject WRITE setObject NOTIFY asObjectChanged FINAL)
public:
	QQuickDetailsView(QQuickItem* parent = nullptr);
	qreal getSpliterPencent() const;
	void setSpliterPencent(qreal val);
	void setObject(QObject* inObject);
	QObject* getObject() const;
Q_SIGNALS:
	void asSpliterPencentChanged(qreal);
	void asObjectChanged(QObject*);
protected:
	void componentComplete() override;
};

#endif // QQuickDetailsView_h__