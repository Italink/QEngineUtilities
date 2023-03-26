#ifndef QDetailViewRow_h__
#define QDetailViewRow_h__

#include <QWidget>
#include <QBoxLayout>

class QDetailView;
class QDetailViewRowWidget;

class QDetailViewRow : public QObject{
	Q_OBJECT
		friend class QDetailView;
public:
	QDetailViewRow(QDetailView* inView);

	void SetupContentWidget(QWidget* inContent);
	void SetupNameValueWidget(QWidget* inNameWidget,QWidget* inValueWidget);

	int ChildCount() const;
	QDetailViewRow* ChildAt(int inIndex);
	QDetailViewRow* AddChildRow();

	void DeleteChild(QDetailViewRow* inChild);
	void DeleteChildren();

	void SetVisible(bool inVisiable);
	bool IsVisible() const;

	void SetExpanded(bool inExpanded);
	bool IsExpanded() const;

	void Refresh();
	void MarkIsCategory();
	bool IsCategory();

	QWidget* GetWidget();
private:
	void RefreshSplitter();
	void FixupSplitter();
Q_SIGNALS:
	void AsRequestRebuildView();
	void AsRequestRebuildParent();
protected:
	QDetailView* mView = nullptr;
	QDetailViewRowWidget* mWidget = nullptr;
	QList<QDetailViewRow*> mChildren;
	bool bExpanded = false;
	bool bIsCategory = false;
};

#endif // QDetailViewRow_h__