#include "QDetailWidgetCategoryItem.h"

QDetailWidgetCategoryItem::QDetailWidgetCategoryItem(QString inCategoryName)
	: mCategoryName(inCategoryName)
{
}

QString QDetailWidgetCategoryItem::GetKeywords() {
	return GetCategoryName();
}

void QDetailWidgetCategoryItem::BuildContentAndChildren() {
	SetCategoryName(mCategoryName);
}


void QDetailWidgetCategoryItem::BuildMenu(QMenu& inMenu) {

}

QString QDetailWidgetCategoryItem::GetCategoryName() const {
	return mCategoryName;
}

void QDetailWidgetCategoryItem::SetCategoryName(QString val) {
	mCategoryName = val;
	setText(0, mCategoryName);
}


