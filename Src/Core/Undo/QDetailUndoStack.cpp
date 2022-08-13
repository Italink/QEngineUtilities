#include "QDetailUndoStack.h"
#include "Widgets/Toolkits/QNotification.h"

QDetailUndoStack* QDetailUndoStack::Instance() {
	static QDetailUndoStack ins;
	return &ins;
}

void QDetailUndoStack::AddEntry(QDetailUndoEntry* entry)
{
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

QDetailUndoEntry::QDetailUndoEntry(QObject* inParent /*= nullptr*/) :QObject(inParent) {
	QDetailUndoStack::Instance()->AddEntry(this);
}

QDetailUndoEntry::~QDetailUndoEntry() {
	QDetailUndoStack::Instance()->RemoveEntry(this);
}

void QDetailUndoEntry::BeginMacro(const QString& text)
{
	QDetailUndoStack::Instance()->beginMacro(text);
}

void QDetailUndoEntry::Push(QUndoCommand* cmd)
{
	QDetailUndoStack::Instance()->push(cmd);
}

void QDetailUndoEntry::EndMacro()
{
	QDetailUndoStack::Instance()->endMacro();
}