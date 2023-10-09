#include "Utils/QEngineUndoStack.h"
#include "Utils/QNotification.h"

QEngineUndoStack* QEngineUndoStack::Instance() {
	static QEngineUndoStack ins;
	return &ins;
}

void QEngineUndoStack::addEntry(QEngineUndoEntry* entry)
{
	mEntryList << entry;
	connect(entry, &QObject::destroyed, this, [this,entry]() {
		this->removeEntry(entry);
	});
}

void QEngineUndoStack::removeEntry(QEngineUndoEntry* entry)
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

void QEngineUndoStack::push(QEngineUndoEntry* entry, QUndoCommand* cmd)
{
	QUndoStack::push(cmd);
	mCommandToEntry[cmd] = entry;
}

void QEngineUndoStack::undo()
{
	if (canUndo()) {
		QNotification::ShowMessage("Undo", undoText(), 2000);
		undo();
		QEngineUndoEntry* entry = mCommandToEntry.value(command(index()));
		if (entry) {
			entry->asUndo();
		}
	}
}

void QEngineUndoStack::redo()
{
	if (canRedo()) {
		QNotification::ShowMessage("Redo", undoText(), 2000);
		QEngineUndoEntry* entry = mCommandToEntry.value(command(index()));
		redo();
		if (entry) {
			entry->asRedo();
		}
	}
}

QEngineUndoEntry::QEngineUndoEntry(QObject* inParent /*= nullptr*/) {
	QEngineUndoStack::Instance()->addEntry(this);
	setParent(inParent);
}

QEngineUndoEntry::~QEngineUndoEntry() {
	QEngineUndoStack::Instance()->removeEntry(this);
}

void QEngineUndoEntry::beginMacro(const QString& text)
{
	QEngineUndoStack::Instance()->beginMacro(text);
}

void QEngineUndoEntry::push(QUndoCommand* cmd)
{
	QEngineUndoStack::Instance()->push(this, cmd);
}

void QEngineUndoEntry::endMacro()
{
	QEngineUndoStack::Instance()->endMacro();
}