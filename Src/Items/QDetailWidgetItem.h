#ifndef QDetailWidgetItem_h__
#define QDetailWidgetItem_h__

#include "QTreeWidget"

class QDetailWidgetItem : public QTreeWidgetItem {
public:
	virtual QString GetKeywords() = 0;

	virtual void BuildContentAndChildren() = 0;

	virtual void OnCreateMenu(QMenu& inMenu) = 0;

	virtual void InsertChild(int index, QDetailWidgetItem* inItem) {
		inItem->AttachTo(this, index);
	}

	void AddChild(QDetailWidgetItem* inItem) {
		InsertChild(childCount(), inItem);
	}

	void AttachTo(QDetailWidgetItem* inParent,int index =-1) {
		if (index < 0)
			index = inParent->childCount();
		inParent->insertChild(index, this);
		this->BuildContentAndChildren();
	}
	void AttachTo(QTreeWidget* inParent, int index = -1) {
		if (index < 0)
			index = inParent->topLevelItemCount();
		inParent->insertTopLevelItem(index, this);
		this->BuildContentAndChildren();
	}

};


#endif // QDetailWidgetItem_h__
