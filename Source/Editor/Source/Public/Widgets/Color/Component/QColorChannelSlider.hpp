#ifndef QColorChannelSlider_h__
#define QColorChannelSlider_h__

#include "Widgets\QNumberBox.h"
#include "QLinearGradient"
#include "QLabel"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QColorChannelSlider : public QWidget {
	Q_OBJECT
public:
	QColorChannelSlider(QString inName, float inDefault = 0.0f, float inMin = 0.0f, float inMax = 1.0f);

	void SetGradientStops(const QGradientStops& inStops);

	void SetChannelValue(float inValue);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

Q_SIGNALS:
	void asValueChanged(float);

private:
	QLabel mLbName;
	QNumberBox mValueBox;
	QGradientStops mGradientStops;

};

#endif // QColorChannelSlider_h__
