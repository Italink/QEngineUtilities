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
	enum Flag {
		ShowChildren = 0x1
	};
	Q_DECLARE_FLAGS(Flags, Flag)

public:
	QDetailView();
	void SetObject(QObject* inObject);
	void SetObjects(const QObjectList& inObjects);
	void SetFlags(Flags inFlag);
	Flags GetFlags() const;
	void SearchByKeywords(QString inKeywords);
	void Undo();
	void Redo();
	void ForceRebuild();
	QDetailViewRow* AddTopLevelRow();

	QDetailViewRow* GetCurrentRow() const;
	void SetCurrentRow(QDetailViewRow* val);
	const QSet<QMetaType>& GetIgnoreMetaTypes() const;
	void SetIgnoreMetaTypes(QSet<QMetaType> val);
	const QSet<const QMetaObject*>& GetIgnoreMetaObjects() const;
	void SetIgnoreMetaObjects(QSet<const QMetaObject*> val);
Q_SIGNALS: 
	void AsCurrentRowChanged(QDetailViewRow* NewRow);
protected:
	void resizeEvent(QResizeEvent* event) override;
	void SetPage(QWidget* inPage);
	void Reset();
	void RefreshRowsState();
	void RefreshRowsSplitter();
private:
	QWidget* mView = nullptr;
	QDetailViewRow* mCurrentRow = nullptr;

	Flags mFlags;
	QObjectList mObjects;
	QList<QDetailViewRow*> mTopLevelRows;
	QSet<QMetaType> mIgnoreMetaTypes;
	QSet<const QMetaObject*> mIgnoreMetaObjects;
	QVBoxLayout* mLayout;
	int mValueWidgetWidth = 0;
	QSharedPointer<QDetailLayoutBuilder> mLayoutBuilder;
};

#endif // QDetailView_h__
