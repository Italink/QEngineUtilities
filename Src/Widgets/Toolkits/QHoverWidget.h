#ifndef QHoverWidget_h__
#define QHoverWidget_h__

#include "QWidget"
#include "QPainter"

class QHoverWidget :public QWidget {
	Q_OBJECT
		Q_PROPERTY(QColor HoverColor READ GetHoverColor WRITE SetHoverColor)
public:
	using QWidget::QWidget;
	QColor GetHoverColor() const { return mHoverColor; }
	void SetHoverColor(QColor val) { mHoverColor = val; }
protected:
	virtual void SetHoverd(bool hoverd) {
		if (hoverd != mHoverd) {
			mHoverd = hoverd;
			update();
		}
	}

	virtual void enterEvent(QEnterEvent* event) override {
		QWidget::enterEvent(event);
		SetHoverd(true);
	}
	virtual void leaveEvent(QEvent* event) override {
		SetHoverd(false);
	}

	void paintEvent(QPaintEvent* event) override {
		QWidget::paintEvent(event);
		QPainter painter(this);
		if (mHoverd) {
			painter.setPen(QPen(QColor(mHoverColor), 1));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect().adjusted(1, 1, -1, -1));
		}
	}
	void focusOutEvent(QFocusEvent* event) override {
		QWidget::focusOutEvent(event);
		SetHoverd(false);
	}

protected:
	QColor mHoverColor;
	bool mHoverd = false;
};


#endif // QHoverWidget_h__
