#ifndef QtColorDialog_h__
#define QtColorDialog_h__

#include <QDialog>
#include <QPushButton>
#include "QEngineEditorAPI.h"

class ColorLineEdit;
class ColorPreview;
class ColorWheel;
class QColorChannelSlider;

class QENGINEEDITOR_API QtColorDialog :public QDialog {
	Q_OBJECT
public:
	QtColorDialog();
	~QtColorDialog();
	void SetColor(QColor color);
	static int CreateAndShow(QColor color,QRect inButtonGemotry);
	void SetCloseWhenLoseFocus(bool val) { bCloseWhenLoseFocus = val; }
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
	bool bCloseWhenLoseFocus = false;
	using QWidget = QWidget;
public:
	inline static QtColorDialog* Current = nullptr;
protected:
	void focusOutEvent(QFocusEvent* event) override;

};

#endif // QtColorDialog_h__