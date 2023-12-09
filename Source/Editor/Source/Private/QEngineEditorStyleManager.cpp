#include "QEngineEditorStyleManager.h"
#include "QApplication"
#include "Widgets/QSvgIcon.h"
#include "QFontDatabase"

QEngineEditorStyleManager* QEngineEditorStyleManager::Instance() {
	static QEngineEditorStyleManager Ins;
	return &Ins;
}

void QEngineEditorStyleManager::registerPalettePreset(QString inName, Palette inPalette)
{
	mPalettePresetMap[inName] = inPalette;
	int id = QFontDatabase::addApplicationFont(mPalettePresetMap[inName].mFontFilePath);
	QStringList families = QFontDatabase::applicationFontFamilies(id);
	mPalettePresetMap[inName].mFont = QFont(families.first(), 12);
}

void QEngineEditorStyleManager::setPalette(QString inName)
{
	if (mPalettePresetMap.contains(inName)) {
		mCurrentPalette = mPalettePresetMap[inName];
		QSvgIcon::setIconColor("DetailView", mCurrentPalette.mIconColor);
		Q_EMIT asPaletteChanged();
	}
}

void QEngineEditorStyleManager::setPalette(Palette inPalette)
{
	mCurrentPalette = inPalette;
	QSvgIcon::setIconColor("DetailView", mCurrentPalette.mIconColor);
	Q_EMIT asPaletteChanged();
}

QEngineEditorStyleManager::QEngineEditorStyleManager()
{
	Palette UnrealStyle;
	UnrealStyle.mShadowColor = QColor(5, 5, 5);
	UnrealStyle.mGridLineColor = QColor(5, 5, 5);
	UnrealStyle.mCategoryColor = QColor(61, 61, 61, 100);
	UnrealStyle.mHoveredColor = QColor(79, 110, 242, 150);
	UnrealStyle.mSelectedColor = QColor(79, 110, 242, 200);
	UnrealStyle.mArrowColor = QColor(220, 220, 220);
	UnrealStyle.mIconColor = QColor(200, 200, 200);
	UnrealStyle.mStyleSheet = R"(
QElideLabel{
	color:rgb(220,220,220);
}
QWidget{
	color:rgb(220,220,220);
	background-color:rgb(36,36,36);
}
QtColorDialog,QNotificationBlock{
	border: 1px solid black;
}
QHoverWidget{
	background-color:rgb(10,10,10);
	qproperty-HoverColor:rgb(79, 110, 242); 
}
QSplitter::handle {background-color: rgb(5,5,5);}
QLineEdit,QTextEdit{
	background-color:rgb(5,5,5);
	border-radius: 3px;
	color: rgb(220,220,220);
	border: 1px solid transparent;
}
QPushButton,QComboBox {
	background-color:rgb(5,5,5);
	color: rgb(220,220,220);
	padding: 2px 5px 2px 5px; 
	border: 1px solid rgb(50,50,50); 
	border-radius: 3px; 
	font-family: "Microsoft YaHei";
	font-size: 10pt;
}
QPushButton:hover,QComboBox:hover,QPushButton:checked {
	background-color: rgb(79, 110, 242);
	color: #F2F2F2;
}
QPushButton:pressed,QComboBox:pressed { 
	background-color: #257FE6;
}
QComboBox QAbstractItemView {
	padding: 0px 0px 4px 0px;
    border: 0px solid transparent;
	border-radius: 0px;
	color: rgb(200,200,200);
    selection-color: rgb(255,255,255);
	background-color: rgb(26,26,26);
    selection-background-color: rgb(49,49,49); 
}
QScrollBar:vertical,
QScrollBar:horizontal  {
    width: 8px;
    background:  rgb(5,5,5);
}
QScrollBar::tryDo:vertical,
QScrollBar::tryDo:horizontal {
    background:  rgb(100,100,100);
    min-height: 30px;
}
QScrollBar::tryDo:vertical:hover,
QScrollBar::tryDo:horizontal:hover {
    background:  rgba(200,200,200,150);
}
QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical,
QScrollBar::sub-line:horizontal, QScrollBar::add-line:horizontal {
    width: 0;
    height: 0;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}

QScrollBar::handle:vertical {
	background: Gainsboro;
	min-height: 20px;
	border-radius: 2px;
	border: none;
}
)";

	registerPalettePreset("Unreal", UnrealStyle);

	Palette QtStyle;
	QtStyle.mShadowColor = QColor(220, 220, 220);
	QtStyle.mGridLineColor = QColor(220, 220, 220);
	QtStyle.mCategoryColor = QColor(240, 240, 240);
	QtStyle.mHoveredColor = QColor(245, 245, 245);
	QtStyle.mSelectedColor = QColor(80, 205, 130);
	QtStyle.mArrowColor = QColor(65, 205, 82);
	QtStyle.mIconColor = QColor(65, 205, 82);
	QtStyle.mStyleSheet = R"(
QWidget{
	color:rgb(30,30,30);
	background-color:rgb(255,255,255);
}
QtColorDialog,QNotificationBlock{
	border: 1px solid white;
}
QHoverLineEdit{
	background-color:transparent;
	qproperty-PlaceholdColor:rgb(65,205,82); 
}
QHoverWidget{
	background-color:transparent;
	qproperty-HoverColor:rgb(65,205,82); 
}
QSplitter::handle {background-color: rgb(220,220,220);}
QLineEdit,QTextEdit{
	background-color:rgb(255,255,255);
	color: rgb(30,30,30);
	border: 1px soild transparent;
}
QLineEdit#NameEditor{
	background-color:transparent;
}
QPushButton,QComboBox {
	background-color:rgb(240,240,240);
	color: rgb(50,50,50);
	padding: 2px 5px 2px 5px; 
	border: 1px solid rgb(180,180,180); 
	border-radius: 3px; 
	font-family: "Microsoft YaHei";
	font-size: 10pt;
}
QPushButton:hover,QComboBox:hover,QPushButton:checked  {
	background-color: rgb(80, 205, 130);
	color: rgb(20,20,20);
	color: rgb(20,20,20);
}
QPushButton:pressed,QComboBox:pressed { 
	background-color: #257FE6;
}
QComboBox QAbstractItemView {
	padding: 0px 0px 4px 0px;
    border: 0px solid transparent;
	border-radius: 0px;
	color: rgb(50,50,50);
    selection-color: rgb(255,255,255);
	background-color: rgb(240,240,240);
    selection-background-color: rgb(250,250,250); 
}
QScrollBar:vertical,
QScrollBar:horizontal  {
    width: 8px;
    background:  rgb(240,240,240);
}
QScrollBar::tryDo:vertical,
QScrollBar::tryDo:horizontal {
    background:  rgb(65,205,82);
    min-height: 30px;
}
QScrollBar::tryDo:vertical:hover,
QScrollBar::tryDo:horizontal:hover {
    background:  rgb(100,230,102);
}
QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical,
QScrollBar::sub-line:horizontal, QScrollBar::add-line:horizontal {
    width: 0;
    height: 0;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}
)";
	registerPalettePreset("Qt", QtStyle);

	setPalette("Unreal");
}

QByteArray QEngineEditorStyleManager::getStylesheet() {
	return mCurrentPalette.mStyleSheet;
}

QColor QEngineEditorStyleManager::getGridLineColor() const {
	return mCurrentPalette.mGridLineColor;
}

QColor QEngineEditorStyleManager::getShadowColor() const {
	return mCurrentPalette.mShadowColor;
}


QColor QEngineEditorStyleManager::getCategoryColor() const {
	return mCurrentPalette.mCategoryColor;
}

QColor QEngineEditorStyleManager::getHoveredColor() const {
	return mCurrentPalette.mHoveredColor;
}

QColor QEngineEditorStyleManager::getArrowColor() const
{
	return mCurrentPalette.mArrowColor;
}

QColor QEngineEditorStyleManager::getSelectedColor() const
{
	return mCurrentPalette.mSelectedColor;
}

QString QEngineEditorStyleManager::getFontFilePath() const
{
	return mCurrentPalette.mFontFilePath;
}

QFont QEngineEditorStyleManager::getFont() const
{
	return mCurrentPalette.mFont;
}
