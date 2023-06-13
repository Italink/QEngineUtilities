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

	QString GetFontFilePath() const { return mFontFilePath; }
	void SetFontFilePath(QString val) { mFontFilePath = val; }

	QFont GetFont() const { return mFont; }
	void SetFont(QFont val) { mFont = val; }
};

#endif // QDetailWidgetStyleManager_h__
