#include "Widgets/Color/QtColorDialog.h"
#include "QBoxLayout"
#include "Component/QColorPicker.h"
#include "Component/ColorLineEdit.hpp"
#include "Component/ColorPreview.hpp"
#include "Component/ColorWheel.hpp"
#include "Component/QColorChannelSlider.hpp"
#include "QEngineEditorStyleManager.h"

QtColorDialog::QtColorDialog()
	: mColorWheel(new ColorWheel)
	, mColorPreview(new ColorPreview)
	, mRedBox(new QColorChannelSlider("R"))
	, mGreenBox(new QColorChannelSlider("G"))
	, mBlueBox(new QColorChannelSlider("B"))
	, mAlphaBox(new QColorChannelSlider("A"))
	, mHueBox(new QColorChannelSlider("H"))
	, mSaturationBox(new QColorChannelSlider("S"))
	, mValueBox(new QColorChannelSlider("V"))
	, mPbPick(new QPushButton("Pick"))
	, mPbOk(new QPushButton("OK"))
	, mPbCancel(new QPushButton("Cancel"))
	, mLeHex(new ColorLineEdit())
{
	createUI();
	connectUI();
}

QtColorDialog::~QtColorDialog()
{
	Current = nullptr;
}

void QtColorDialog::setColor(QColor color) {
	mLastColor = color;
	mColorPreview->setComparisonColor(color);
	setCurrentColorInternal(color);
}

int QtColorDialog::CreateAndShow(QColor color, QRect inButtonGemotry) {
	QtColorDialog* dialog = QtColorDialog::Current;
	dialog->disconnect();
	if (dialog == nullptr) {
		dialog = new QtColorDialog;
		QtColorDialog::Current = dialog;
		QtColorDialog::Current->setStyleSheet(QEngineEditorStyleManager::Instance()->getStylesheet());
		QObject::connect(QEngineEditorStyleManager::Instance(), &QEngineEditorStyleManager::asPaletteChanged, QtColorDialog::Current, []() {
			if (QtColorDialog::Current) {
				QtColorDialog::Current->setStyleSheet(QEngineEditorStyleManager::Instance()->getStylesheet());
			}
		});
	}
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setCloseWhenLoseFocus(true);
	dialog->setColor(color);
	dialog->show();
	dialog->activateWindow();
	dialog->setFocus();
	QRect mGeom = dialog->geometry();

	mGeom.moveCenter(inButtonGemotry.center());
	mGeom.moveBottom(inButtonGemotry.top());
	if (mGeom.top() < 0) {
		mGeom.moveTop(inButtonGemotry.bottom());
	}
	dialog->setGeometry(mGeom);
	return 0;
}

void QtColorDialog::createUI()
{
	setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip);
	setFocusPolicy(Qt::NoFocus);
	mColorWheel->setSelectorShape(ColorWheel::ShapeSquare);
	mColorWheel->setFocusPolicy(Qt::NoFocus);
	QVBoxLayout* v = new QVBoxLayout(this);
	QHBoxLayout* h = new QHBoxLayout();
	h->addWidget(mColorWheel,5);
	QVBoxLayout* tool = new QVBoxLayout;
	tool->setAlignment(Qt::AlignTop);
	tool->addSpacing(40);
	tool->addWidget(mColorPreview);
	tool->addWidget(mLeHex);
	tool->addWidget(mPbPick);
	h->addLayout(tool,2);

	v->addLayout(h);
	QHBoxLayout* sliderPanle = new QHBoxLayout();
	sliderPanle->setAlignment(Qt::AlignBottom);
	QVBoxLayout* RGBA = new QVBoxLayout;
	QVBoxLayout* HSV = new QVBoxLayout;

	RGBA->setAlignment(Qt::AlignTop);
	RGBA->addWidget(mRedBox);
	RGBA->addWidget(mGreenBox);
	RGBA->addWidget(mBlueBox);
	RGBA->addWidget(mAlphaBox);

	HSV->setAlignment(Qt::AlignTop);
	HSV->addWidget(mHueBox);
	HSV->addWidget(mSaturationBox);
	HSV->addWidget(mValueBox);
	QHBoxLayout* buttonPanel = new QHBoxLayout();
	buttonPanel->setAlignment(Qt::AlignRight);
	buttonPanel->addWidget(mPbOk);
	buttonPanel->addWidget(mPbCancel);
	HSV->addLayout(buttonPanel);

	sliderPanle->addLayout(RGBA);
	sliderPanle->addLayout(HSV);
	v->addLayout(sliderPanle);

	mLeHex->setFixedHeight(25);
	mColorPreview->setFixedHeight(40);
	mColorPreview->setDisplayMode(ColorPreview::SplitColor);
	mColorWheel->setMinimumSize(150, 150);
	mColorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mPbPick->setFocusPolicy(Qt::NoFocus);
	mPbOk->setFocusPolicy(Qt::NoFocus);
	mPbCancel->setFocusPolicy(Qt::NoFocus);
}

void QtColorDialog::connectUI()
{
	connect(mColorWheel, &ColorWheel::OnColorChanged, this, [this](QColor color) {
		setCurrentColorInternal(color);
	});

	connect(mPbPick, &QPushButton::clicked, this, [this]() {
		bool flag = false;
		if (bCloseWhenLoseFocus) {
			flag = bCloseWhenLoseFocus;
			bCloseWhenLoseFocus = false;
		}
		QColor color = QColorPicker::Pick();
		if (color.isValid()) {
			setCurrentColorInternal(color);
		}
		this->activateWindow();
		this->setFocus();
		if (flag) {
			bCloseWhenLoseFocus = true;
		}
	});

	connect(mLeHex, &ColorLineEdit::OnColorChanged, this, [this](QColor color) {
		setCurrentColorInternal(color);
	});

	connect(mRedBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setRedF(var);
		setCurrentColorInternal(next);
	});
	connect(mGreenBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setGreenF(var);
		setCurrentColorInternal(next);
	});
	connect(mBlueBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setBlueF(var);
		setCurrentColorInternal(next);
	});
	connect(mAlphaBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setAlphaF(var);
		setCurrentColorInternal(next);
	});

	connect(mHueBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setHsvF(var, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF());
		setCurrentColorInternal(next);
	});
	connect(mSaturationBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setHsvF(mCurrentColor.hsvHueF(), var, mCurrentColor.valueF());
		setCurrentColorInternal(next);
	});
	connect(mValueBox, &QColorChannelSlider::asValueChanged, this, [this](float var) {
		QColor next = mCurrentColor;
		next.setHsvF(mCurrentColor.hsvHueF(), mCurrentColor.hsvSaturationF(), var);
		setCurrentColorInternal(next);
	});

	connect(mPbOk, &QPushButton::clicked, this, [this]() {
		close();
	});
	connect(mPbCancel, &QPushButton::clicked, this, [this]() {
		setCurrentColorInternal(mLastColor);
		close();
	});
}

void QtColorDialog::setCurrentColorInternal(QColor color)
{
	if (mCurrentColor == color)
		return;
	mCurrentColor = color;
	refleshChannelGradiant();

	mColorWheel->blockSignals(true);
	mColorPreview->blockSignals(true);
	mRedBox->blockSignals(true);
	mGreenBox->blockSignals(true);
	mBlueBox->blockSignals(true);
	mAlphaBox->blockSignals(true);
	mHueBox->blockSignals(true);
	mSaturationBox->blockSignals(true);
	mValueBox->blockSignals(true);
	mLeHex->blockSignals(true);

	mColorWheel->setColor(color);
	mColorPreview->setColor(color);
	mLeHex->setColor(color);

	mRedBox->SetChannelValue(color.redF());
	mGreenBox->SetChannelValue(color.greenF());
	mBlueBox->SetChannelValue(color.blueF());
	mAlphaBox->SetChannelValue(color.alphaF());
	mHueBox->SetChannelValue(color.hsvHueF());
	mSaturationBox->SetChannelValue(color.hsvSaturationF());
	mValueBox->SetChannelValue(color.valueF());

	mColorWheel->blockSignals(false);
	mColorPreview->blockSignals(false);
	mRedBox->blockSignals(false);
	mGreenBox->blockSignals(false);
	mBlueBox->blockSignals(false);
	mAlphaBox->blockSignals(false);
	mHueBox->blockSignals(false);
	mSaturationBox->blockSignals(false);
	mValueBox->blockSignals(false);
	mLeHex->blockSignals(false);

	Q_EMIT asColorChanged(mCurrentColor);
}

void QtColorDialog::refleshChannelGradiant()
{
	QGradientStops stops;
	QColor begin = mCurrentColor;
	QColor end = mCurrentColor;

	begin.setRedF(0.0f);
	end.setRedF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mRedBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setGreenF(0.0f);
	end.setGreenF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mGreenBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setBlueF(0.0f);
	end.setBlueF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mBlueBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setAlphaF(0.0f);
	end.setAlphaF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mAlphaBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setHsvF(0.0f, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF());
	end.setHsvF(1.0f, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF());
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mHueBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setHsvF(mCurrentColor.hsvHueF(), 0.0f, mCurrentColor.valueF());
	end.setHsvF(mCurrentColor.hsvHueF(), 1.0f, mCurrentColor.valueF());
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mSaturationBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor;
	end = mCurrentColor;
	begin.setHsvF(mCurrentColor.hsvHueF(),mCurrentColor.hsvSaturationF() , 0.0f);
	end.setHsvF(mCurrentColor.hsvHueF(), mCurrentColor.hsvSaturationF(), 1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mValueBox->SetGradientStops(stops);
}

void QtColorDialog::focusOutEvent(QFocusEvent* event) {
	if (bCloseWhenLoseFocus&& event->reason()==Qt::FocusReason::ActiveWindowFocusReason) {
		close();
	}
}
