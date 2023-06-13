#ifndef GRADIENT_BAR_HPP
#define GRADIENT_BAR_HPP

#include <QLinearGradient>
#include <QWidget>
#include "GradientHelper.hpp"
#include "QEngineEditorAPI.h"

/**
 * \brief A slider that moves on top of a gradient
 */
class GradientBarItem;

class QENGINEEDITOR_API GradientBar : public QWidget
{
	Q_OBJECT
public:
	explicit GradientBar(QWidget* parent = 0);
	~GradientBar();
	QGradientStops colors() const;
	void setColors(const QGradientStops& colors);
	void clear();
	QColor getCurrentColor() const;
	void setCurrentColor(QColor color);
	GradientBarItem* addItem(QGradientStop stop);
	void removeItem(GradientBarItem* item);
	void updateItemPositon(GradientBarItem* item);
	void setCurrentItem(GradientBarItem* item);
private:
	void flushGradinetBar();
Q_SIGNALS:
	void currentItemChanged(QColor color);
	void currentItemPositionChanged(qreal);
protected:
	void paintEvent(QPaintEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void resizeEvent(QResizeEvent* event) override;
private:
	bool lockSignal = false;
	class Private;
	Private* const p;
};

#endif // GRADIENT_SLIDER_HPP