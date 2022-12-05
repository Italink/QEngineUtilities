#include "QDetailWidgetStyleManager.h"
#include "Widgets\Toolkits\QSvgIcon.h"
#include "QApplication"

QDetailWidgetStyleManager* QDetailWidgetStyleManager::Instance() {
	static QDetailWidgetStyleManager Ins;
	return &Ins;
}

QByteArray QDetailWidgetStyleManager::GetStylesheet() {
	return mStyleSheet;
}

QColor QDetailWidgetStyleManager::GetGridLineColor() const {
	return mGridLineColor;
}

void QDetailWidgetStyleManager::SetGridLineColor(QColor val) {
	mGridLineColor = val;
}

QColor QDetailWidgetStyleManager::GetShadowColor() const {
	return mShadowColor;
}

void QDetailWidgetStyleManager::SetShadowColor(QColor val) {
	mShadowColor = val;
}

QColor QDetailWidgetStyleManager::GetCategoryColor() const {
	return mCategoryColor;
}

void QDetailWidgetStyleManager::SetCategoryColor(QColor val) {
	mCategoryColor = val;
}

QColor QDetailWidgetStyleManager::GetHoveredColor() const {
	return mHoveredColor;
}

void QDetailWidgetStyleManager::SetHoveredColor(QColor val) {
	mHoveredColor = val;
}

QColor QDetailWidgetStyleManager::GetIconColor() const {
	return QSvgIcon::GetIconColor();
}

void QDetailWidgetStyleManager::SetIconColor(QColor val) {
	QSvgIcon::setIconColor(val);
}

QDetailWidgetStyleManager::QDetailWidgetStyleManager() {
	SetStyle(mCurrentStyle);
}

void QDetailWidgetStyleManager::SetStyle(QDetailWidgetStyle inStyle) {
	if (inStyle == QDetailWidgetStyle::Unreal) {
		mShadowColor = QColor(5, 5, 5);
		mGridLineColor = QColor(5, 5, 5);
		mCategoryColor = QColor(61, 61, 61);
		mHoveredColor = QColor(51, 51, 51);
		mSelectedColor = QColor(79, 110, 242);
		mArrowColor = QColor(220, 220, 220);
		QSvgIcon::setIconColor(QColor(200, 200, 200));
		mStyleSheet = R"(
QDetailWidget{
	background-color:rgb(36,36,36);
}
QNameLabel{
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
QDetailWidget{
	background-color:rgb(36,36,36);
}
QSplitter::handle {background-color: rgb(5,5,5);}
QLineEdit,QTextEdit{
	background-color:rgb(5,5,5);
	border-radius: 3px;
	color: rgb(220,220,220);
	border: 1px solid transparent;
}

QPushButton,QComboBox{
	background-color:rgb(5,5,5);
	color: rgb(220,220,220);
	padding: 2px 5px 2px 5px; 
	border: 1px outset rgb(50,50,50);
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
)";
	}
	else if (inStyle == QDetailWidgetStyle::Qt) {
		mShadowColor = QColor(220, 220, 220);
		mGridLineColor = QColor(220, 220, 220);
		mCategoryColor = QColor(255, 255, 255);
		mHoveredColor = QColor(245, 245, 245);
		mSelectedColor = QColor(80, 205, 130);
		mArrowColor = QColor(65, 205, 82);
		QSvgIcon::setIconColor(QColor(65, 205, 82));
		mStyleSheet = R"(
QDetailWidget{
	background-color:rgb(255,255,255);
}
QWidget{
	color:rgb(30,30,30);
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
QDetailWidget{
	background-color:rgb(240,240,240);
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

QPushButton,QComboBox{
	background-color:rgb(240,240,240);
	color: rgb(50,50,50);
	padding: 2px 5px 2px 5px; 
	border: 2px outset rgb(200,200,200);
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

