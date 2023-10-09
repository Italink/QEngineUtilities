#ifndef QEngineUndoStack_h__
#define QEngineUndoStack_h__

#include "QUndoStack"
#include "QHash"
#include "QMap"
#include "QEngineEditorAPI.h"

class QEngineUndoEntry;

class QENGINEEDITOR_API QEngineUndoStack : public QUndoStack {
	Q_OBJECT
public:
	static QEngineUndoStack* Instance();
	void addEntry(QEngineUndoEntry* entry);
	void removeEntry(QEngineUndoEntry* entry);
	void push(QEngineUndoEntry* entry, QUndoCommand* cmd);
	void undo();
	void redo();
private:
	QList<QEngineUndoEntry*> mEntryList;
	QMap<const QUndoCommand*, QEngineUndoEntry*> mCommandToEntry;
};

class QENGINEEDITOR_API QEngineUndoEntry: public QObject {
	friend class QEngineUndoStack;
	Q_OBJECT
public:
	QEngineUndoEntry(QObject *inParent = nullptr);
	~QEngineUndoEntry();
	void beginMacro(const QString& text);
	void push(QUndoCommand* cmd);
	void endMacro();
Q_SIGNALS:
	void asUndo();
	void asRedo();
};

#endif // QEngineUndoStack_h__
