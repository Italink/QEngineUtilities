#ifndef QColorButton_h__
#define QColorButton_h__

#include "Widgets/QHoverWidget.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QColorButton :public QHoverWidget {
	Q_OBJECT
public:
	QColorButton(QColor color = Qt::red);
	void setColor(QColor color);
	QColor GetColor() const;
Q_SIGNALS:
	void asColorChanged(QColor color);
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	QColor mColor;
};

#endif // QColorButton_h__
