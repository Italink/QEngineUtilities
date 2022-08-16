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
		qDebug()<<"Set "<<#Name<<": "<<var; \
    } \
    Type Name


class TestInlineGadget{
	Q_GADGET
		Q_META_BEGIN(TestInlineGadget)
		Q_META_P_NUMBER_LIMITED(LimitedDouble, 0, 100)
		Q_META_END()
public:
	TestInlineGadget() { qDebug() << "Create"; }
	~TestInlineGadget() { qDebug() << "Release"; }
	Q_PROPERTY_AUTO(double, LimitedDouble) = 1;
	Q_PROPERTY_AUTO(QString, Desc) = "This is inline Gadget";
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

static QDebug operator<<(QDebug debug, const std::shared_ptr<TestInlineGadget>& gadget) {
	return debug << gadget->LimitedDouble << gadget->Desc;
}

//static QDebug operator<<(QDebug debug, const TestInlineObject* object) {
//	return debug << object->Desc;
//}


class TestObject :public QObject {
	Q_OBJECT
		Q_META_BEGIN(TestObject)
		Q_META_CATEGORY_ENABLED()
		Q_META_CATEGORY_DEFINE(Number, Int, Float, LimitedDouble, Vec2, Vec3, Vec4)
		Q_META_CATEGORY_DEFINE(Color, Color, Colors, ColorList, StdColorList, ColorMap)
		Q_META_CATEGORY_DEFINE(String, QtString, StdString, AsMultiLineString, AsPath, AsCombo)
		Q_META_CATEGORY_DEFINE(Inline, InlineGadget, InlineGadgetPtr, InlineGadgetSPtr, InlineGadgetStdSPtr, InlineObject, InlineObjectSPtr, InlineObjectList, InlineGadgetList, InlineGadgetPtrList, InlineGadgetSPtrList, InlineGadgetStdPtrList, InlineGadgetSPtrMap)
		Q_META_CATEGORY_DEFINE(Other, Bool)
		//Q_META_CATEGORY_VISIBLE(Inline, false)
		Q_META_P_NUMBER_LIMITED(LimitedDouble, 0, 100)
		Q_META_P_STRING_AS_LINE(QtString, "This is QString")
		Q_META_P_STRING_AS_LINE(StdString, "This is std::string")
		Q_META_P_STRING_AS_MULTI_LINE(AsMultiLineString, 80, "This is MultiLine")
		Q_META_P_STRING_AS_FILE_PATH(AsPath)
		Q_META_P_STRING_AS_COMBO(AsCombo, A, B, C, D)
		Q_META_P_ARRAY_FIXED_SIZE(ColorList, true)
		Q_META_P_ARRAY_FIXED_ORDER(ColorList, true)
		Q_META_P_MAP_FIXED_KEY(ColorMap, true)
		Q_META_P_MAP_FIXED_SIZE(ColorMap, true)
		Q_META_P_INS_PTR_CHOSE_SUBTYPE_BY_META(InlineGadgetSPtr, TestInlineGadget, C, D);
	Q_META_END()
public:
	enum TestEnum {
		One,
		Two,
		Three
	};
	Q_ENUM(TestEnum);

	Q_PROPERTY_AUTO(int, Int) = 0;
	Q_PROPERTY_AUTO(float, Float) = 1.23f;
	Q_PROPERTY_AUTO(double, LimitedDouble) = 5;
	Q_PROPERTY_AUTO(TestEnum, Enum) = TestEnum::One;
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
	void SetColorMap(QMap<QString, QColor> val) {
		ColorMap = val;
		qDebug() << "ColorMap: " << ColorMap;
	}
	QMap<QString, QColor> ColorMap = { {"Red",Qt::red},{"Green",Qt::green},{"Blue",Qt::blue} };

	Q_PROPERTY_AUTO(TestInlineGadget, InlineGadget);
	Q_PROPERTY_AUTO(TestInlineGadget*, InlineGadgetPtr) = new TestInlineGadget;
	Q_PROPERTY_AUTO(QSharedPointer<TestInlineGadget>, InlineGadgetSPtr) = QSharedPointer<TestInlineGadget>::create();
	Q_PROPERTY_AUTO(TestInlineObject*, InlineObject) = new TestInlineObject;
	Q_PROPERTY_AUTO(QSharedPointer<TestInlineObject>, InlineObjectSPtr) = QSharedPointer<TestInlineObject>::create();
	Q_PROPERTY_AUTO(QList<TestInlineObject*>, InlineObjectList) = { };
	Q_PROPERTY_AUTO(QList<TestInlineGadget>, InlineGadgetList) = { };
	Q_PROPERTY_AUTO(QList<TestInlineGadget*>, InlineGadgetPtrList) = { };
	Q_PROPERTY_AUTO(QList<QSharedPointer<TestInlineGadget>>, InlineGadgetSPtrList) = { };

	Q_PROPERTY(QMap<QString, QSharedPointer<TestInlineGadget>> InlineGadgetSPtrMap READ GetInlineGadgetSPtrMap WRITE SetInlineGadgetSPtrMap)
	QMap<QString, QSharedPointer<TestInlineGadget>> GetInlineGadgetSPtrMap() const { return InlineGadgetSPtrMap; }
	void SetInlineGadgetSPtrMap(QMap<QString,QSharedPointer<TestInlineGadget>> val) { InlineGadgetSPtrMap = val; }
	QMap<QString, QSharedPointer<TestInlineGadget>> InlineGadgetSPtrMap;
};

Q_DECLARE_METATYPE(QSharedPointer<TestInlineGadget>);
Q_DECLARE_METATYPE(std::shared_ptr<TestInlineGadget>);

#endif // TestObject_h__