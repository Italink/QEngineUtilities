#ifndef QDetailViewRow_h__
#define QDetailViewRow_h__

#include <QWidget>
#include <QBoxLayout>
#include "QEngineEditorAPI.h"

class QDetailView;
class QDetailViewRowWidget;
class QPropertyHandle;

class QENGINEEDITOR_API QDetailViewRow : public QObject{
	Q_OBJECT
	friend class QDetailView;
	friend class IDetailLayoutBuilder;
	friend class QDetailViewRowWidget;
public:
	QDetailViewRow(QDetailView* inView);

	void SetupContentWidget(QWidget* inContent);
	void SetupNameValueWidget(QWidget* inNameWidget,QWidget* inValueWidget);
	void SetupPropertyHandle(QPropertyHandle* val);

	int ChildCount() const;
	QDetailViewRow* ChildAt(int inIndex);
	QDetailViewRow* AddChildRow();

	void DeleteChild(QDetailViewRow* inChild);
	void DeleteChildren();

	void SetVisible(bool inVisiable);
	bool IsVisible() const;

	bool IsCurrent() const;

	void SetExpanded(bool inExpanded, bool bRecursive = false);
	bool IsExpanded() const;

	void UpdateWidget();
	void Refresh();
	void MarkIsCategory();
	bool IsCategory();
	void RequestRefreshSplitter();

	QWidget* GetWidget();
private:
	void FixupSplitter();
	void RefreshSplitter();
Q_SIGNALS:
	void RequestRebuildView();
protected:
	QDetailView* mView = nullptr;
	QDetailViewRowWidget* mWidget = nullptr;
	QPropertyHandle* mHandle = nullptr;
	QList<QDetailViewRow*> mChildren;
	bool bExpanded = false;
	bool bIsCategory = false;
	bool bNeedRefreshSplitter = false;
};

#endif // QDetailViewRow_h__