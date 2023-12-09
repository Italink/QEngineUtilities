#ifndef QNumberBox_h__
#define QNumberBox_h__

#include "QWidget"
#include "QLabel"
#include "QFocusLineEdit.h"
#include "QSvgIcon.h"
#include "QHoverWidget.h"

class QENGINEEDITOR_API QNumberBoxAdaptorBase {
public:
	virtual QString getDisplayText() = 0;
	virtual QVariant getVar() = 0;
	virtual void setVar(QVariant var) = 0;
	virtual void setDisplayText(QString text) = 0;
	virtual void moveOffset(QPointF offset) = 0;
	virtual double getLimitedFactor() = 0;
};

template<typename NumericType>
class QNumberBoxAdaptor: public QNumberBoxAdaptorBase {
public:
	virtual QString getDisplayText() override{
		if (std::is_floating_point<NumericType>::value) {
			return QString::number(mValue);
		}
		return QString::number(mValue);
	}

	virtual void setDisplayText(QString inText) override{
		NumericType number = QVariant(inText).value<NumericType>();
		setValue(number);
	}

	struct Limited{
		bool enabled = false;
		NumericType min;
		NumericType max;
	};

	void setValue(NumericType value) {
		if (mLimited.enabled) {
			mValue = qBound(mLimited.min, value, mLimited.max);
		}
		else {
			mValue = value;
		}
	}

	NumericType getValue() {
		return mValue;
	}

	void SetLimited(bool enabled ,NumericType min, NumericType max) {
		if (min > max) 
			qSwap(min, max);
		mLimited.enabled = enabled;
		mLimited.min = min;
		mLimited.max = max;
	}

	virtual double getLimitedFactor() override {
		if (mLimited.enabled) {
			return (mValue - mLimited.min) / (double)(mLimited.max - mLimited.min);
		}
		return 0;
	}

	virtual QVariant getVar() override
	{
		return mValue;
	}

	virtual void setVar(QVariant var) override
	{
		setValue(var.value<NumericType>());
	}

	virtual void moveOffset(QPointF offset) override
	{
		if (mLimited.enabled) {
			if (std::is_integral<NumericType>::value) {
				setValue(getValue() + offset.x() * qMax((NumericType)1, (mLimited.max - mLimited.min) / (NumericType)2000));
			}
			else {
				setValue(getValue() + offset.x() *(mLimited.max - mLimited.min) /(NumericType)1000.0);
			}
		}
		else {
			if (std::is_integral<NumericType>::value) {
				setValue(getValue() + offset.x() * qMax(qAbs(getValue() /(NumericType) 200.0), (NumericType)1.0));
			}
			else {
				double adjuster = getValue() + offset.x() * qMax(qAbs(getValue() /(NumericType) 200.0), (NumericType)0.01);
				adjuster = int(adjuster * 100) / 100.0;
				setValue(adjuster);
			}
		}
	}

private:
	Limited mLimited;
	NumericType mValue;
};

class QENGINEEDITOR_API QNumberBox :public QHoverWidget{
	Q_OBJECT
public:
	template<typename NumericType>
	QNumberBox(NumericType inValue, bool inLimitedEnabled = false, NumericType inMin = 0, NumericType inMax = 100) {
		QSharedPointer<QNumberBoxAdaptor<NumericType>> adaptor = QSharedPointer<QNumberBoxAdaptor<NumericType>>::create();
		adaptor->SetLimited(inLimitedEnabled, inMin, inMax);
		adaptor->setValue(inValue);
		mNumberAdaptor = adaptor;
		createUI();
		connectUI();
	}
	void setEditEnabled(bool enable);
	bool getEditEnabled();

	QString getDisplayText();
	QVariant getVar();
	void setVar(QVariant var);
Q_SIGNALS:
	void asValueChanged(QVariant);
protected:
	void createUI();
	void connectUI();
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
	virtual QSize sizeHint() const override;
private:
	QSharedPointer<QNumberBoxAdaptorBase> mNumberAdaptor;
	QLineEdit_HasFocusSignal* mLeValue;
	QPointF mClickPosition;
};

#endif // QNumberBox_h__
