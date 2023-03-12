#ifndef QNumberBox_h__
#define QNumberBox_h__

#include "QWidget"
#include "QLabel"
#include "QFocusLineEdit.h"
#include "QSvgIcon.h"
#include "QHoverWidget.h"

class QNumberBoxAdaptorBase {
public:
	virtual QString GetText() = 0;
	virtual QVariant GetVar() = 0;
	virtual void SetVar(QVariant var) = 0;
	virtual void SetText(QString text) = 0;
	virtual void MoveOffset(QPointF offset) = 0;
	virtual double GetLimitedFactor() = 0;
};

template<typename NumericType>
class QNumberBoxAdaptor: public QNumberBoxAdaptorBase {
public:
	virtual QString GetText() override{
		if (std::is_floating_point<NumericType>::value) {
			return QString::number(mValue);
		}
		return QString::number(mValue);
	}

	virtual void SetText(QString inText) override{
		NumericType number = QVariant(inText).value<NumericType>();
		SetValue(number);
	}

	struct Limited{
		bool enabled = false;
		NumericType min;
		NumericType max;
	};

	void SetValue(NumericType value) {
		if (mLimited.enabled) {
			mValue = qBound(mLimited.min, value, mLimited.max);
		}
		else {
			mValue = value;
		}
	}

	NumericType GetValue() {
		return mValue;
	}

	void SetLimited(bool enabled ,NumericType min, NumericType max) {
		if (min > max) 
			qSwap(min, max);
		mLimited.enabled = enabled;
		mLimited.min = min;
		mLimited.max = max;
	}

	virtual double GetLimitedFactor() override {
		if (mLimited.enabled) {
			return (mValue - mLimited.min) / (double)(mLimited.max - mLimited.min);
		}
		return 0;
	}

	virtual QVariant GetVar() override
	{
		return mValue;
	}

	virtual void SetVar(QVariant var) override
	{
		SetValue(var.value<NumericType>());
	}

	virtual void MoveOffset(QPointF offset) override
	{
		if (mLimited.enabled) {
			if (std::is_integral<NumericType>::value) {
				SetValue(GetValue() + offset.x() * qMax((NumericType)1, (mLimited.max - mLimited.min) / (NumericType)2000));
			}
			else {
				SetValue(GetValue() + offset.x() *(mLimited.max - mLimited.min) /(NumericType)1000.0);
			}
		}
		else {
			if (std::is_integral<NumericType>::value) {
				SetValue(GetValue() + offset.x() * qMax(qAbs(GetValue() /(NumericType) 200.0), (NumericType)1.0));
			}
			else {
				double adjuster = GetValue() + offset.x() * qMax(qAbs(GetValue() /(NumericType) 200.0), (NumericType)0.01);
				adjuster = int(adjuster * 100) / 100.0;
				SetValue(adjuster);
			}
		}
	}

private:
	Limited mLimited;
	NumericType mValue;
};

class QNumberBox :public QHoverWidget{
	Q_OBJECT
public:
	template<typename NumericType>
	QNumberBox(NumericType inValue, bool inLimitedEnabled = false, NumericType inMin = 0, NumericType inMax = 100) {
		QSharedPointer<QNumberBoxAdaptor<NumericType>> adaptor = QSharedPointer<QNumberBoxAdaptor<NumericType>>::create();
		adaptor->SetLimited(inLimitedEnabled, inMin, inMax);
		adaptor->SetValue(inValue);
		mNumberAdaptor = adaptor;
		CreateUI();
		ConnectUI();
	}
	void SetEditEnabled(bool enable);
	bool GetEditEnabled();

	QString GetText();
	QVariant GetVar();
	void SetVar(QVariant var);
Q_SIGNALS:
	void AsValueChanged(QVariant);
protected:
	void CreateUI();
	void ConnectUI();
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
	virtual QSize sizeHint() const override;
private:
	QSharedPointer<QNumberBoxAdaptorBase> mNumberAdaptor;
	QLineEdit_HasFocusSignal* mLeValue;
	QLabel* mLbArrow;
	QPointF mClickPosition;
};

#endif // QNumberBox_h__
