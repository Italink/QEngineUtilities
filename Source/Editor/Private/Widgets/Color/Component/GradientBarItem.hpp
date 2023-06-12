#ifndef GRADIENT_BAR_ITEM_HPP
#define GRADIENT_BAR_ITEM_HPP

#include <QWidget>

class GradientBarItem : public QWidget
{
	Q_OBJECT
public:
	explicit GradientBarItem(QGradientStop stop, QWidget* parent = 0);
	~GradientBarItem();
	QColor getColor() const;
	void SetColor(QColor val);
	qreal getPos() const;
	bool setPos(qreal val);
	bool isHighlight() const { return highlight; }
	void setHighlight(bool val) { highlight = val; }
protected:
	void paintEvent(QPaintEvent* ev) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QGradientStop gradientStop;
	bool highlight;
signals:
	void itemMoved(GradientBarItem*);
	void itemHighLight(GradientBarItem*);
	void itemRemoved(GradientBarItem*);
};

#endif // GRADIENT_SLIDER_HPP