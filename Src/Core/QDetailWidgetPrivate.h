#ifndef QDetailWidgetPrivate_h__
#define QDetailWidgetPrivate_h__

#include "QObject"
#include "QTreeWidget"
#include "Undo/QDetailUndoStack.h"
#include "QInstance.h"

class QPushButton;
class QHoverLineEdit;
class QDetailWidgetCategoryItem;
class QLabel;
class QSvgIcon;

class QDetailSearcher :public QWidget {
	Q_OBJECT
public:
	QDetailSearcher();
Q_SIGNALS:
	void AsRequestSearch(QString);
private:
	QLabel* mLbSearch;
	QSvgIcon* mIconSearch;
	QHoverLineEdit* mLeSearch;
	QPushButton* mPbSearch;
};

class QDetailTreeWidget: public QTreeWidget{
	Q_OBJECT
public:
	QDetailTreeWidget();

	void SetInstances(const QList<QSharedPointer<QInstance>>& inInstances);

	void SearchByKeywords(QString inKeywords);

	void Undo();

	void Redo();

	void Recreate();

	void SetSplitterSizes(int item0, int item1, int item2);

	QList<int> GetSplitterSizes() const;
protected:
	void UpdateSplitterFactor();
	virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
	virtual void showEvent(QShowEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	using QTreeWidget::QTreeWidget;

private:
	QList<int> mSplitterSizes = { 200,400,30 };
	QList<QSharedPointer<QInstance>> mInstanceList;
public:
	inline static QDetailUndoStack UndoStack;
};


#endif // QDetailWidget_h__
