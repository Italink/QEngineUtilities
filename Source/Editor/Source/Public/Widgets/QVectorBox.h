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
	void setValue(QVector4D vec);
	QVector4D getValue();
Q_SIGNALS:
	void asValueChanged(QVector4D);
protected:
	void emitValueChanged(QVariant);
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
	void setValue(QVector3D vec);
	QVector3D getValue();
Q_SIGNALS:
	void asValueChanged(QVector3D);
protected:
	void emitValueChanged(QVariant);
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
	void setValue(QVector2D vec);
	QVector2D getValue();
Q_SIGNALS:
	void asValueChanged(QVector2D);
protected:
	void emitValueChanged(QVariant);
	virtual void paintEvent(QPaintEvent* event) override;
private:
	QNumberBox mX;
	QNumberBox mY;
};


#endif // QVectorBox_h__

