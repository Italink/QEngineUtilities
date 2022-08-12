#include "QDetailUndoStack.h"
#include "Widgets/Toolkits/QNotification.h"

void QDetailUndoStack::AddEntry(QDetailUndoEntry* entry)
{
	entry->mStack = this;
	mEntryList << entry;
	connect(entry, &QObject::destroyed, this, [this,entry]() {
		this->RemoveEntry(entry);
	});
}

void QDetailUndoStack::RemoveEntry(QDetailUndoEntry* entry)
{
	if (entry == nullptr)
		return;
	for (int i = 0; i < count(); i++) {
		 QUndoCommand* cmd = const_cast<QUndoCommand*>(command(i));
		if (mCommandToEntry[cmd] == entry) {
			cmd->setObsolete(true);
		}
	}
	entry->mStack = nullptr;
	mEntryList.removeOne(entry);
}

void QDetailUndoStack::Push(QDetailUndoEntry* entry, QUndoCommand* cmd)
{
	push(cmd);
	mCommandToEntry[cmd] = entry;
}

void QDetailUndoStack::Undo()
{
	if (canUndo()) {
		QNotification::ShowMessage("Undo", undoText(), 2000);
		undo();
	}
}

void QDetailUndoStack::Redo()
{
	if (canRedo()) {
		QNotification::ShowMessage("Redo", undoText(), 2000);
		redo();
	}
}

void QDetailUndoEntry::BeginMacro(const QString& text)
{
	if (mStack)
		mStack->beginMacro(text);
}

void QDetailUndoEntry::Push(QUndoCommand* cmd)
{
	if (mStack)
		mStack->push(cmd);
}

void QDetailUndoEntry::EndMacro()
{
	if (mStack)
		mStack->endMacro();
}