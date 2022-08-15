#include "QDetailWidget.h"
#include "QBoxLayout"
#include "Core\QDetailWidgetPrivate.h"
#include "Core\QDetailWidgetStyleManager.h"

QDetailWidget::QDetailWidget(QDetailWidgetFlags inFlags)
	: mSearcher(new QDetailSearcher)
	, mTreeWidget(new QDetailTreeWidget)
	, mFlags(inFlags) {

	QVBoxLayout* v = new QVBoxLayout(this);
	v->setContentsMargins(0, 5, 0, 5);
	if (mFlags.testFlag(QDetailWidgetFlag::DisplaySearcher))
		v->addWidget(mSearcher);
	v->addWidget(mTreeWidget);
	connect(mSearcher, &QDetailSearcher::AsRequestSearch, this, &QDetailWidget::SearchByKeywords);
	setStyleSheet(QDetailWidgetStyleManager::Instance()->GetStylesheet());
}

void QDetailWidget::SetInstanceList(const QList<QSharedPointer<QInstance>>& inInstances)
{
	mTreeWidget->SetInstances(inInstances);
}

void QDetailWidget::SearchByKeywords(QString inKeywords) {
	mTreeWidget->SearchByKeywords(inKeywords);
}
