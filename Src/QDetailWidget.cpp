#include "QDetailWidget.h"
#include "QBoxLayout"
#include "Core\QDetailWidgetPrivate.h"
#include "QUndoView"

QDetailWidget::QDetailWidget(Flags inFlags /*= DisplaySearcher*/, Style inStyle /*= Unreal*/)
	: mSearcher(new QDetailSearcher)
	, mTreeWidget(new QDetailTreeWidget)
	, mFlags(inFlags)
{
	QVBoxLayout* v = new QVBoxLayout(this);
	v->setContentsMargins(0, 5, 0, 5);
	v->addWidget(new QUndoView(QDetailUndoStack::Instance()));
	if(mFlags.testFlag(DisplaySearcher))
		v->addWidget(mSearcher);
	v->addWidget(mTreeWidget);
	SetStyle(inStyle);
	connect(mSearcher, &QDetailSearcher::AsRequestSearch, this, &QDetailWidget::SearchByKeywords);
}


void QDetailWidget::SetInstanceList(const QList<QSharedPointer<QInstance>>& inInstances)
{
	if (mFlags.testFlag(DisplayCategory)) {
		for (auto& Instance : inInstances) {
			Instance->SetMetaData("DisplayCategory",true);
		}
	}
	mTreeWidget->SetInstances(inInstances);
}

void QDetailWidget::SetStyle(Style inStyle) {
	switch (inStyle) {
	case QDetailWidget::Unreal: {
		setStyleSheet(R"(
QDetailWidget{
	background-color:rgb(36,36,36);
}
QWidget{
	color:rgb(220,220,220);
}
QHoverWidget{
	background-color:rgb(10,10,10);
	qproperty-HoverColor:rgb(79, 110, 242); 
}
QDetailTreeWidget{
	background-color:rgb(36,36,36);
	qproperty-ShadowColor:rgb(5,5,5);
	qproperty-GridLineColor:rgb(5,5,5);
	qproperty-CategoryColor:rgb(61,61,61);  
	qproperty-HoveredColor:rgb(51,51,51); 
	qproperty-IconColor:rgb(251,251,251); 
	qproperty-ArrowColor:rgb(220,220,220); 
}
QSplitter::handle {background-color: rgb(5,5,5);}
QLineEdit,QTextEdit{
	background-color:rgb(5,5,5);
	border-radius: 3px;
	color: rgb(220,220,220);
	border: 1px solid transparent;
}

QLineEdit:hover,QTextEdit:hover,QPushButton:hover,QComboBox:hover{
	border: 1px solid rgb(79, 110, 242);
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

QScrollBar::handle:vertical,
QScrollBar::handle:horizontal {
    background:  rgb(100,100,100);
    min-height: 30px;
}

QScrollBar::handle:vertical:hover,
QScrollBar::handle:horizontal:hover {
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
)"
);
		break;
	}
	case Qt: {
		setStyleSheet(R"(
QDetailWidget{
	background-color:rgb(255,255,255);
}
QWidget{
	color:rgb(30,30,30);
}
QHoverLineEdit{
	background-color:transparent;
	qproperty-PlaceholdColor:rgb(65,205,82); 
}
QHoverWidget{
	background-color:transparent;
	qproperty-HoverColor:rgb(65,205,82); 
}
QDetailTreeWidget{
	background-color:rgb(240,240,240);
	qproperty-ShadowColor:rgb(220,220,220);
	qproperty-GridLineColor: rgb(220,220,220);
	qproperty-CategoryColor:rgb(255,255,255);
	qproperty-HoveredColor:rgb(245,245,245); 
	qproperty-IconColor:rgb(65,205,82); 
	qproperty-ArrowColor:rgb(65,205,82); 
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

QScrollBar::handle:vertical,
QScrollBar::handle:horizontal {
    background:  rgb(65,205,82);
    min-height: 30px;
}

QScrollBar::handle:vertical:hover,
QScrollBar::handle:horizontal:hover {
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
)"
);
		break;
	}
	default:
		break;
	}
}

void QDetailWidget::SearchByKeywords(QString inKeywords) {
	mTreeWidget->SearchByKeywords(inKeywords);
}
