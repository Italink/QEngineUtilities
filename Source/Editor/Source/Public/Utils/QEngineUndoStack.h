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
	void AddEntry(QEngineUndoEntry* entry);
	void RemoveEntry(QEngineUndoEntry* entry);
	void Push(QEngineUndoEntry* entry, QUndoCommand* cmd);
	void Undo();
	void Redo();
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
	void BeginMacro(const QString& text);
	void Push(QUndoCommand* cmd);
	void EndMacro();
Q_SIGNALS:
	void AsUndo();
	void AsRedo();
};

#endif // QEngineUndoStack_h__
