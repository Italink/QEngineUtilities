#ifndef QDetailView_h__
#define QDetailView_h__

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "QEngineEditorAPI.h"

class QDetailViewRow;
class QDetailLayoutBuilder;

class QENGINEEDITOR_API QDetailView: public QScrollArea {
	Q_OBJECT
	friend class QDetailViewRow;
	friend class IDetailLayoutBuilder;
public:
	QDetailView();
	void SetObject(QObject* inObject);
	void SetObjects(const QObjectList& inObjects);
	void SearchByKeywords(QString inKeywords);
	void Undo();
	void Redo();
	void ForceRebuild();
	QDetailViewRow* AddTopLevelRow();
protected:
	void SetPage(QWidget* inPage);
	void resizeEvent(QResizeEvent* event) override;
	void Reset();
	void RefreshRowsState();
	void RefreshRowsSplitter();
private:
	QWidget* mView = nullptr;
	QObjectList mObjects;
	QList<QDetailViewRow*> mTopLevelRows;
	QVBoxLayout* mLayout;
	int mValueWidgetWidth = 0;
	QSharedPointer<QDetailLayoutBuilder> mLayoutBuilder;
};

#endif // QDetailView_h__