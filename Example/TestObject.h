#ifndef TestObject_h__
#define TestObject_h__

#include "QObject"
#include "qvectornd.h"
#include "Core/QMetaDataDefine.h"
#include "QFile"
#include "QDir"
#include "QColor"

#define Q_PROPERTY_AUTO(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
		qDebug()<<#Name<<": "<<var; \
    } \
    Type Name


class TestInlineGadget{
	Q_GADGET
		Q_META_BEGIN(TestInlineGadget)
		Q_META_NUMBER_LIMITED(LimitedDouble, 0, 100)
		Q_META_END()
public:
	Q_PROPERTY_AUTO(double, LimitedDouble);
	Q_PROPERTY_AUTO(QString, Desc) = "This is inline gadget";
};

class TestInlineObject : public QObject {
	Q_OBJECT
public:
	Q_PROPERTY_AUTO(QString, Desc) = "This is inline Object";
};

static QDebug operator<<(QDebug debug, const std::string& str) {
	return debug<<QString::fromStdString(str);
}

static QDebug operator<<(QDebug debug, const TestInlineGadget& gadget) {
	return debug << gadget.LimitedDouble << gadget.Desc;
}

static QDebug operator<<(QDebug debug, const TestInlineObject* object) {
	return debug << object->Desc;
}


class TestObject :public QObject {
	Q_OBJECT

	Q_META_BEGIN(TestObject)
		Q_META_CATEGORY(Number, Int, Float, LimitedDouble, Vec2, Vec3, Vec4)
		Q_META_CATEGORY(Color,Color,Colors, ColorList, StdColorList,ColorMap)
		Q_META_CATEGORY(String, QtString, StdString, AsMultiLineString, AsPath, AsCombo)
		Q_META_CATEGORY(Inline, InlineGadget, InlineObject)
		Q_META_CATEGORY(Other,Bool)
		Q_META_NUMBER_LIMITED(LimitedDouble, 0, 100)
		Q_META_STRING_AS_LINE(QtString,"This is QString")
		Q_META_STRING_AS_LINE(StdString,"This is std::string")
		Q_META_STRING_AS_MULTI_LINE(AsMultiLineString,80,"This is MultiLine")
		Q_META_STRING_AS_FILE_PATH(AsPath)
		Q_META_STRING_AS_COMBO(AsCombo,A,B,C,D)
	Q_META_END()
public:
	enum TestEnum {
		One,
		Two,
		Three
	};
	Q_ENUM(TestEnum);

	Q_PROPERTY_AUTO(int, Int) = 0;
	Q_PROPERTY_AUTO(float, Float) = 1.23;
	Q_PROPERTY_AUTO(double, LimitedDouble) = 5;
	Q_PROPERTY_AUTO(TestEnum, Enum);
	Q_PROPERTY_AUTO(QString, QtString);
	Q_PROPERTY_AUTO(std::string, StdString);
	Q_PROPERTY_AUTO(QString, AsMultiLineString);
	Q_PROPERTY_AUTO(QString, AsPath);
	Q_PROPERTY_AUTO(QString, AsCombo) = "A";
	Q_PROPERTY_AUTO(QVector2D, Vec2);
	Q_PROPERTY_AUTO(QVector3D, Vec3);
	Q_PROPERTY_AUTO(QVector4D, Vec4);
	Q_PROPERTY_AUTO(QColor, Color);
	Q_PROPERTY_AUTO(QList<QColor>, ColorList) = { Qt::red,Qt::green,Qt::blue };
	Q_PROPERTY_AUTO(std::vector<QColor>, StdColorList) = { Qt::red,Qt::green,Qt::blue };

	Q_PROPERTY(QMap<QString, QColor> ColorMap READ GetColorMap WRITE SetColorMap)
		QMap<QString, QColor> GetColorMap() const { return ColorMap; }
	void SetColorMap(QMap<QString, QColor> val) { ColorMap = val; }
	QMap<QString, QColor> ColorMap = { {"Red",Qt::red},{"Green",Qt::green},{"Blue",Qt::blue} };

	Q_PROPERTY_AUTO(TestInlineGadget, InlineGadget);
	Q_PROPERTY_AUTO(TestInlineObject*, InlineObject) = new TestInlineObject;
};

#endif // TestObject_h__
