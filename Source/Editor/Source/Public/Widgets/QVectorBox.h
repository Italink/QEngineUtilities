#ifndef QVectorBox_h__
#define QVectorBox_h__

#include "QWidget"
#include "QNumberBox.h"
#include "qvectornd.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QVector4DBox : public QWidget{
	Q_OBJECT
public:
	QVector4DBox(QVector4D vec = QVector4D());
	void SetValue(QVector4D vec);
	QVector4D GetValue();
Q_SIGNALS:
	void AsValueChanged(QVector4D);
protected:
	void EmitValueChanged(QVariant);
	virtual void paintEvent(QPaintEvent* event) override;
private:
	QNumberBox mX;
	QNumberBox mY;
	QNumberBox mZ;
	QNumberBox mW;
};

class QVector3DBox : public QWidget {
	Q_OBJECT
public:
	QVector3DBox(QVector3D vec = QVector3D());
	void SetValue(QVector3D vec);
	QVector3D GetValue();
Q_SIGNALS:
	void AsValueChanged(QVector3D);
protected:
	void EmitValueChanged(QVariant);
	virtual void paintEvent(QPaintEvent* event) override;
private:
	QNumberBox mX;
	QNumberBox mY;
	QNumberBox mZ;
};

class QVector2DBox : public QWidget {
	Q_OBJECT
public:
	QVector2DBox(QVector2D vec = QVector2D());
	void SetValue(QVector2D vec);
	QVector2D GetValue();
Q_SIGNALS:
	void AsValueChanged(QVector2D);
protected:
	void EmitValueChanged(QVariant);
	virtual void paintEvent(QPaintEvent* event) override;
private:
	QNumberBox mX;
	QNumberBox mY;
};


#endif // QVectorBox_h__

