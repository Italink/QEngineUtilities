#ifndef QDetailWidget_h__
#define QDetailWidget_h__

#include "QObject"
#include "QTreeWidget"

class QPushButton;
class QLineEdit;
class QDetailWidgetCategoryItem;

class QDetailSearcher :public QWidget {
	Q_OBJECT
public:
	QDetailSearcher();
Q_SIGNALS:
	void AsRequestSearch(QString);
private:
	QLineEdit* mLeSearch;
	QPushButton* mPbSearch;
};

class QDetailWidget: public QTreeWidget{
	Q_OBJECT
	Q_PROPERTY(QColor GridLineColor READ GetGridLineColor WRITE SetGridLineColor)
	Q_PROPERTY(QColor ShadowColor READ GetShadowColor WRITE SetShadowColor)
	Q_PROPERTY(QColor CategoryColor READ GetCategoryColor WRITE SetCategoryColor)
	Q_PROPERTY(QColor HoveredColor READ GetHoveredColor WRITE SetHoveredColor)
	Q_PROPERTY(QColor IconColor READ GetIconColor WRITE SetIconColor)
public:
	QDetailWidget();

	void SetObjects(const QList<QObject*>& inObjects);

	void Recreate();

	void SearchByKeywords(QString inKeywords);

	QList<int> GetSplitterSizes() const;
	void SetSplitterSizes(int item0, int item1, int item2);

	QColor GetGridLineColor() const;
	void SetGridLineColor(QColor val);

	QColor GetShadowColor() const;
	void SetShadowColor(QColor val);

	QColor GetCategoryColor() const { return mCategoryColor; }
	void SetCategoryColor(QColor val) { mCategoryColor = val; }

	QColor GetHoveredColor() const { return mHoveredColor; }
	void SetHoveredColor(QColor val) { mHoveredColor = val; }

	QColor GetIconColor() const;
	void SetIconColor(QColor val);

	QDetailWidgetCategoryItem* FindOrAddCategory(QString inName);

	using QTreeWidget::QTreeWidget;
protected:
	void UpdateSplitterFactor();
	virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
	virtual void showEvent(QShowEvent* event) override;
private:
	QList<QObject*> mObjects;

	QColor mGridLineColor;
	QColor mShadowColor;
	QColor mCategoryColor;
	QColor mHoveredColor;
	
	QList<int> mSplitterSizes = { 200,200,30 };

	QDetailSearcher* mSearcher = nullptr;
};

#endif // QDetailWidget_h__
