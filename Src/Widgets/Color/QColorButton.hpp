#ifndef QColorButton_h__
#define QColorButton_h__

#include <QPushButton>

class QColorButton :public QPushButton {
	Q_OBJECT
public:
	QColorButton(QColor color);
	void SetColor(QColor color);
	QColor GetColor() const;
Q_SIGNALS:
	void AsColorChanged(QColor color);
protected:
	void paintEvent(QPaintEvent* event) override;
private:
	QColor mColor;
};

#endif // QColorButton_h__
