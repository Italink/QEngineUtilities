#ifndef QDetailWidgetStyleManager_h__
#define QDetailWidgetStyleManager_h__

#include "QObject"
#include "QColor"
#include "QFont"
#include "QEngineEditorAPI.h"

enum class QDetailWidgetStyle {
	Qt,
	Unreal
};

class QENGINEEDITOR_API QEngineEditorStyleManager {
	QEngineEditorStyleManager();
private:
	QDetailWidgetStyle mCurrentStyle = QDetailWidgetStyle::Unreal;
	QColor mGridLineColor;
	QColor mShadowColor;
	QColor mCategoryColor;
	QColor mHoveredColor;
	QColor mSelectedColor;
	QColor mArrowColor;
	QString mFontFilePath = ":/Resources/DroidSans.ttf";
	QFont mFont;
	QByteArray mStyleSheet;
public:
	static QEngineEditorStyleManager* Instance();

	void setStyle(QDetailWidgetStyle inStyle);
	QByteArray getStylesheet();

	QColor getGridLineColor() const;
	void setGridLineColor(QColor val);

	QColor getShadowColor() const;
	void setShadowColor(QColor val);

	QColor getCategoryColor() const;
	void setCategoryColor(QColor val);

	QColor getHoveredColor() const;
	void setHoveredColor(QColor val);

	QColor getIconColor() const;
	void setIconColor(QColor val);

	QColor getArrowColor() const { return mArrowColor; }
	void setArrowColor(QColor val) { mArrowColor = val; }

	QColor getSelectedColor() const { return mSelectedColor; }
	void setSelectedColor(QColor val) { mSelectedColor = val; }

	QString getFontFilePath() const { return mFontFilePath; }
	void setFontFilePath(QString val) { mFontFilePath = val; }

	QFont getFont() const { return mFont; }
	void setFont(QFont val) { mFont = val; }
};

#endif // QDetailWidgetStyleManager_h__
