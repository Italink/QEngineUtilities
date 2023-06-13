#ifndef QFocusLineEdit_h__
#define QFocusLineEdit_h__

#include <QLineEdit>
#include "QPainter"
#include "QHoverLineEdit.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QLineEdit_HasFocusSignal : public QLineEdit {
	Q_OBJECT
public:
	Q_SIGNAL void loseFocus();
protected:
	virtual void focusOutEvent(QFocusEvent* e) {
		QLineEdit::focusOutEvent(e);
		Q_EMIT loseFocus();
	}
};


#endif // QFocusLineEdit_h__
