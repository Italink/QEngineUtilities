#include "QEngineUndoStack.h"
#include "Utils/QNotification.h"

QEngineUndoStack* QEngineUndoStack::Instance() {
	static QEngineUndoStack ins;
	return &ins;
}

void QEngineUndoStack::AddEntry(QEngineUndoEntry* entry)
{
	mEntryList << entry;
	connect(entry, &QObject::destroyed, this, [this,entry]() {
		this->RemoveEntry(entry);
	});
}

void QEngineUndoStack::RemoveEntry(QEngineUndoEntry* entry)
{
	if (entry == nullptr)
		return;
	for (int i = 0; i < count(); i++) {
		 QUndoCommand* cmd = const_cast<QUndoCommand*>(command(i));
		if (mCommandToEntry[cmd] == entry) {
			cmd->setObsolete(true);
		}
	}
	mEntryList.removeOne(entry);
}

void QEngineUndoStack::Push(QEngineUndoEntry* entry, QUndoCommand* cmd)
{
	push(cmd);
	mCommandToEntry[cmd] = entry;
}

void QEngineUndoStack::Undo()
{
	if (canUndo()) {
		QNotification::ShowMessage("Undo", undoText(), 2000);
		undo();
		QEngineUndoEntry* entry = mCommandToEntry.value(command(index()));
		if (entry) {
			entry->AsUndo();
		}
	}
}

void QEngineUndoStack::Redo()
{
	if (canRedo()) {
		QNotification::ShowMessage("Redo", undoText(), 2000);
		QEngineUndoEntry* entry = mCommandToEntry.value(command(index()));
		redo();
		if (entry) {
			entry->AsRedo();
		}
	}
}

QEngineUndoEntry::QEngineUndoEntry(QObject* inParent /*= nullptr*/) {
	QEngineUndoStack::Instance()->AddEntry(this);
	setParent(inParent);
}

QEngineUndoEntry::~QEngineUndoEntry() {
	QEngineUndoStack::Instance()->RemoveEntry(this);
}

void QEngineUndoEntry::BeginMacro(const QString& text)
{
	QEngineUndoStack::Instance()->beginMacro(text);
}

void QEngineUndoEntry::Push(QUndoCommand* cmd)
{
	QEngineUndoStack::Instance()->Push(this, cmd);
}

void QEngineUndoEntry::EndMacro()
{
	QEngineUndoStack::Instance()->endMacro();
}