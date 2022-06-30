#ifndef QtColorDialog_h__
#define QtColorDialog_h__

#include <QDialog>

#include "Component\ColorLineEdit.hpp"
#include "Component\ColorPreview.hpp"
#include "Component\ColorWheel.hpp"
#include "Component\QColorChannelSlider.hpp"
#include "QPushButton"

class QtColorDialog :public QDialog {
	Q_OBJECT
public:
	QtColorDialog();
	~QtColorDialog();
	void SetColor(QColor color);
	static int CreateAndShow(QColor color);
private:
	void CreateUI();
	void ConnectUI();
	void SetCurrentColorInternal(QColor color);
	void RefleshChannelGradiant();
Q_SIGNALS:
	void AsColorChanged(QColor);
private:
	ColorWheel* mColorWheel;
	ColorPreview* mColorPreview;
	QColorChannelSlider* mRedBox;
	QColorChannelSlider* mGreenBox;
	QColorChannelSlider* mBlueBox;
	QColorChannelSlider* mAlphaBox;
	QColorChannelSlider* mHueBox;
	QColorChannelSlider* mSaturationBox;
	QColorChannelSlider* mValueBox;
	QPushButton* mPbPick;
	QPushButton* mPbOk;
	QPushButton* mPbCancel;
	ColorLineEdit* mLeHex;
	QColor mCurrentColor;
	QColor mLastColor;
public:
	inline static QtColorDialog* Current = nullptr;
};

#endif // QtColorDialog_h__