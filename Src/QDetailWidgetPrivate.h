#ifndef QDetailWidgetPrivate_h__
#define QDetailWidgetPrivate_h__

#include "QObject"
#include "QTreeWidget"


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
		Q_PROPERTY(QColor GridLineColor READ GetGridLineColor WRITE SetGridLineColor)
		Q_PROPERTY(QColor ShadowColor READ GetShadowColor WRITE SetShadowColor)
		Q_PROPERTY(QColor CategoryColor READ GetCategoryColor WRITE SetCategoryColor)
		Q_PROPERTY(QColor HoveredColor READ GetHoveredColor WRITE SetHoveredColor)
		Q_PROPERTY(QColor IconColor READ GetIconColor WRITE SetIconColor)
		Q_PROPERTY(QColor ArrowColor READ GetArrowColor WRITE SetArrowColor)
public:
	QDetailTreeWidget();

	void SetObjects(const QList<QObject*>& inObjects);

	void Recreate();

	void SearchByKeywords(QString inKeywords);

	QList<int> GetSplitterSizes() const;
	void SetSplitterSizes(int item0, int item1, int item2);

	QColor GetGridLineColor() const;
	void SetGridLineColor(QColor val);

	QColor GetShadowColor() const;
	void SetShadowColor(QColor val);

	QColor GetCategoryColor() const;
	void SetCategoryColor(QColor val);

	QColor GetHoveredColor() const;
	void SetHoveredColor(QColor val);

	QColor GetIconColor() const;
	void SetIconColor(QColor val);

	QColor GetArrowColor() const { return mArrowColor; }
	void SetArrowColor(QColor val) { mArrowColor = val; }

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
	QColor mArrowColor;
	
	QList<int> mSplitterSizes = { 200,400,30 };

};

#endif // QDetailWidget_h__
