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
	~QDetailViewRow();

	void setupContentWidget(QWidget* inContent);
	void setupNameValueWidget(QWidget* inNameWidget,QWidget* inValueWidget);
	void setupPropertyHandle(QPropertyHandle* val);

	int childrenCount() const;
	QDetailViewRow* childAt(int inIndex);
	QDetailViewRow* addChildRow();

	void removeChild(QDetailViewRow* inChild);
	void clear();

	void setVisible(bool inVisiable);
	bool isVisible() const;

	bool isCurrent() const;

	void setExpanded(bool inExpanded, bool bRecursive = false);
	bool isExpanded() const;

	void updateWidget();
	void refresh();
	void markIsCategory();
	bool isCategory();
	void requestRefreshSplitter();

	QWidget* getWidget();
	QPropertyHandle* getPropertyHandle() const { return mHandle; }
private:
	void fixupSplitter();
	void refreshSplitter();
Q_SIGNALS:
	void requestRebuildView();
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