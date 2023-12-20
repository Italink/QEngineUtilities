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
		isChildrenVisible = 0x1
	};
	Q_DECLARE_FLAGS(Flags, Flag)

public:
	QDetailView();
	~QDetailView();
	void setObject(QObject* inObject);
	void setObjects(const QObjectList& inObjects);
	void selectSubObject(QObject* inObject);

	void setFlags(Flags inFlag);
	Flags getFlags() const;
	void searchByKeywords(QString inKeywords);
	void undo();
	void redo();
	void forceRebuild();
	QDetailViewRow* addTopLevelRow();

	QDetailViewRow* getCurrentRow() const;
	void setCurrentRow(QDetailViewRow* val);
	const QSet<QMetaType>& getIgnoredTypes() const;
	void setIgnoredTypes(QSet<QMetaType> val);
	const QSet<const QMetaObject*>& getIgnoredObjects() const;
	void setIgnoreObjects(QSet<const QMetaObject*> val);
Q_SIGNALS: 
	void asCurrentRowChanged(QDetailViewRow* NewRow);
protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void setPage(QWidget* inPage);
	void reset();
	void refreshRowsState();
	void refreshRowsSplitter();
	void foreachRows(std::function<bool(QDetailViewRow*)> inProcessor);
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
	bool bNeedUpdateStyle = false;
};

#endif // QDetailView_h__
