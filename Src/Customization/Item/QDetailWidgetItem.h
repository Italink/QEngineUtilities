#ifndef QDetailWidgetItem_h__
#define QDetailWidgetItem_h__

#include "QTreeWidget"

class QDetailWidgetItem : public QTreeWidgetItem {
public:
	enum ItemType {
		None,
		Category,
		Property,
	};

	virtual QString GetKeywords() { return QString(); };

	virtual void BuildContentAndChildren() {};

	virtual void BuildMenu(QMenu& inMenu) {};

	virtual ItemType Type() const { return ItemType::None; }

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

	void Clear(){
		for (auto child : takeChildren()) {
			delete child;
		}
	}
};


#endif // QDetailWidgetItem_h__
