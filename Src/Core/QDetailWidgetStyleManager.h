#ifndef QDetailWidgetStyleManager_h__
#define QDetailWidgetStyleManager_h__

#include "QHash"
#include "QString"
#include "QObject"
#include "QColor"

enum class QDetailWidgetStyle {
	Qt,
	Unreal
};

class QDetailWidgetStyleManager{
private:
	QColor mGridLineColor;
	QColor mShadowColor;
	QColor mCategoryColor;
	QColor mHoveredColor;
	QColor mSelectedColor;
	QColor mArrowColor;
	QDetailWidgetStyleManager();
	QDetailWidgetStyle mCurrentStyle = QDetailWidgetStyle::Unreal;
	QByteArray mStyleSheet;
public:
	static QDetailWidgetStyleManager* Instance();
	void SetStyle(QDetailWidgetStyle inStyle);
	QByteArray GetStylesheet();
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

	QColor GetSelectedColor() const { return mSelectedColor; }
	void SetSelectedColor(QColor val) { mSelectedColor = val; }
};

#endif // QDetailWidgetStyleManager_h__
