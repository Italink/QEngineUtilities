#ifndef QHoverWidget_h__
#define QHoverWidget_h__

#include "QWidget"
#include "QPainter"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QHoverWidget :public QWidget {
	Q_OBJECT
	Q_PROPERTY(QColor HoverColor READ getHoverColor WRITE setHoverColor)
public:
	using QWidget::QWidget;
	QColor getHoverColor() const { return mHoverColor; }
	void setHoverColor(QColor val) { mHoverColor = val; }

	bool isHoverEnabled() const { return mHoverEnabled; }
	void setHoverEnabled(bool val) { mHoverEnabled = val; }
protected:
	virtual void setHoverd(bool hoverd) {
		if (hoverd != mHoverd) {
			mHoverd = hoverd;
			update();
		}
	}

	virtual void enterEvent(QEnterEvent* event) override {
		QWidget::enterEvent(event);
		setHoverd(true);
	}
	virtual void leaveEvent(QEvent* event) override {
		setHoverd(false);
	}

	void paintEvent(QPaintEvent* event) override {
		QWidget::paintEvent(event);
		if (mHoverd && mHoverEnabled) {
			QPainter painter(this);
			painter.setPen(QPen(QColor(mHoverColor), 1));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(rect().adjusted(0, 0, -1, -1));
		}
	}
	void focusOutEvent(QFocusEvent* event) override {
		QWidget::focusOutEvent(event);
		setHoverd(false);
	}
protected:
	QColor mHoverColor;
	bool mHoverd = false;
	bool mHoverEnabled = true;
};


#endif // QHoverWidget_h__
