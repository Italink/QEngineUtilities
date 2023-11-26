#ifndef QDetailWidgetStyleManager_h__
#define QDetailWidgetStyleManager_h__

#include <QObject>
#include <QColor>
#include <QFont>
#include <QMap>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QEngineEditorStyleManager {
	QEngineEditorStyleManager();
public:
	struct Palette {
		QColor mGridLineColor;
		QColor mShadowColor;
		QColor mCategoryColor;
		QColor mHoveredColor;
		QColor mSelectedColor;
		QColor mArrowColor;
		QColor mIconColor;
		QString mFontFilePath = ":/Resources/DroidSans.ttf";
		QFont mFont;
		QByteArray mStyleSheet;
	};

	static QEngineEditorStyleManager* Instance();
	void registerPalette(QString inName, Palette inPalette);
	void setCurrentPalette(QString inName);

	QByteArray getStylesheet();
	QColor getGridLineColor() const;
	QColor getShadowColor() const;
	QColor getCategoryColor() const;
	QColor getHoveredColor() const;
	QColor getArrowColor() const;; 
	QColor getSelectedColor() const;
	QString getFontFilePath() const;
	QFont getFont() const;
private:
	QString mCurrentPalette;
	QMap<QString, Palette> mPaletteMap;
};

#endif // QDetailWidgetStyleManager_h__
