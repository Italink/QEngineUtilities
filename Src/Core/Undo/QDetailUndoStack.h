#ifndef QDetailUndoStack_h__
#define QDetailUndoStack_h__

#include "QUndoStack"
#include "QHash"

class QDetailUndoEntry;

class QDetailUndoStack : public QUndoStack {
public:
	static QDetailUndoStack* Instance();
	void AddEntry(QDetailUndoEntry* entry);
	void RemoveEntry(QDetailUndoEntry* entry);
	void Push(QDetailUndoEntry* entry, QUndoCommand* cmd);
	void Undo();
	void Redo();
private:
	QList<QDetailUndoEntry*> mEntryList;
	QHash<QUndoCommand*, QDetailUndoEntry*> mCommandToEntry;
};

class QDetailUndoEntry: public QObject {
	friend class QDetailUndoStack;
	Q_OBJECT
public:
	QDetailUndoEntry(QObject *inParent = nullptr);
	~QDetailUndoEntry();
	void BeginMacro(const QString& text);
	void Push(QUndoCommand* cmd);
	void EndMacro();
};

#endif // QDetailUndoStack_h__
