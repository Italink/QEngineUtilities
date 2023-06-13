#ifndef QColorPicker_h__
#define QColorPicker_h__

#include <QWidget>
#include <QDialog>
#include <QMouseEvent>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QColorPicker : public QDialog
{
	Q_OBJECT
public:
	static QColor Pick();

	explicit QColorPicker();
	QColor PickColor();
protected:
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
private:
	QPixmap mScreen;
	QPoint mMousePos;
	QColor mCurrentColor;
};

#endif // QColorPicker_h__
