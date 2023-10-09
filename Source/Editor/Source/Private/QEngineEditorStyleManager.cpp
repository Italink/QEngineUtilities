#include "QEngineEditorStyleManager.h"
#include "QApplication"
#include "Widgets/QSvgIcon.h"
#include "QFontDatabase"

QEngineEditorStyleManager* QEngineEditorStyleManager::Instance() {
	static QEngineEditorStyleManager Ins;
	return &Ins;
}

QEngineEditorStyleManager::QEngineEditorStyleManager()
{
	int id = QFontDatabase::addApplicationFont(mFontFilePath);
	QStringList families = QFontDatabase::applicationFontFamilies(id);
	mFont = QFont(families.first(), 12);
	qApp->setFont(mFont);
	setStyle(mCurrentStyle);
}

QByteArray QEngineEditorStyleManager::getStylesheet() {
	return mStyleSheet;
}

QColor QEngineEditorStyleManager::getGridLineColor() const {
	return mGridLineColor;
}

void QEngineEditorStyleManager::setGridLineColor(QColor val) {
	mGridLineColor = val;
}

QColor QEngineEditorStyleManager::getShadowColor() const {
	return mShadowColor;
}

void QEngineEditorStyleManager::setShadowColor(QColor val) {
	mShadowColor = val;
}

QColor QEngineEditorStyleManager::getCategoryColor() const {
	return mCategoryColor;
}

void QEngineEditorStyleManager::setCategoryColor(QColor val) {
	mCategoryColor = val;
}

QColor QEngineEditorStyleManager::getHoveredColor() const {
	return mHoveredColor;
}

void QEngineEditorStyleManager::setHoveredColor(QColor val) {
	mHoveredColor = val;
}

QColor QEngineEditorStyleManager::getIconColor() const {
	return QSvgIcon::getIconColor();
}

void QEngineEditorStyleManager::setIconColor(QColor val) {
	QSvgIcon::setIconColor(val);
}

void QEngineEditorStyleManager::setStyle(QDetailWidgetStyle inStyle) {
	if (inStyle == QDetailWidgetStyle::Unreal) {
		mShadowColor = QColor(5, 5, 5);
		mGridLineColor = QColor(5, 5, 5);
		mCategoryColor = QColor(61, 61, 61, 100);
		mHoveredColor = QColor(79, 110, 242, 150);
		mSelectedColor = QColor(79, 110, 242, 200);
		mArrowColor = QColor(220, 220, 220);
		QSvgIcon::setIconColor(QColor(200, 200, 200));
		mStyleSheet = R"(
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
	}
	else if (inStyle == QDetailWidgetStyle::Qt) {
		mShadowColor = QColor(220, 220, 220);
		mGridLineColor = QColor(220, 220, 220);
		mCategoryColor = QColor(240, 240, 240);
		mHoveredColor = QColor(245, 245, 245);
		mSelectedColor = QColor(80, 205, 130);
		mArrowColor = QColor(65, 205, 82);
		QSvgIcon::setIconColor(QColor(65, 205, 82));
		mStyleSheet = R"(
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

	}
}
