#include "QDetailWidget.h"
#include "QBoxLayout"
#include "Core\QDetailWidgetPrivate.h"
#include "Core\QDetailWidgetStyleManager.h"
#include "Core\QInstanceTreeWidget.h"

QDetailWidget::QDetailWidget(QDetailWidgetFlags inFlags)
	:  mTreeWidget(new QDetailTreeWidget)
	, mFlags(inFlags) {

	QVBoxLayout* v = new QVBoxLayout(this);
	v->setContentsMargins(0, 0, 0, 0);
	if (mFlags.testFlag(QDetailWidgetFlag::DisplayObjectTree)) {
		mInstanceTree = new QInstanceTreeWidget;
		v->addWidget(mInstanceTree);
	}

	if (mFlags.testFlag(QDetailWidgetFlag::DisplaySearcher)) {
		mSearcher =  new QDetailSearcher();
		v->addWidget(mSearcher);
		connect(mSearcher, &QDetailSearcher::AsRequestSearch, this, &QDetailWidget::SearchByKeywords);
	}
	v->addWidget(mTreeWidget);
	setStyleSheet(QDetailWidgetStyleManager::Instance()->GetStylesheet());
}

void QDetailWidget::SetInstanceList(const QList<QSharedPointer<QInstance>>& inInstances)
{
	if (mFlags.testFlag(QDetailWidgetFlag::DisplayObjectTree)) {
		mInstanceTree->SetInstances(inInstances);
		connect(mInstanceTree, &QInstanceTreeWidget::AsInstanceSelected, this ,[this](QSharedPointer<QInstance> inInstance) {
			mTreeWidget->SetInstances({ inInstance });
		});
	}
	else {
		mTreeWidget->SetInstances(inInstances);
	}
}

void QDetailWidget::SearchByKeywords(QString inKeywords) {
	mTreeWidget->SearchByKeywords(inKeywords);
}
