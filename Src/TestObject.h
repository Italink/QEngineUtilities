#ifndef TestObject_h__
#define TestObject_h__

#include "QObject"
#include "Types/QColors.h"
#include "QObjectEx.h"
#include "qvectornd.h"
#include "QFile"
#include "QDir"

class TestObject :public QObject {
	Q_OBJECT
	Q_PROPERTY(int Int READ getInt WRITE setInt)
	Q_PROPERTY(float Float READ GetFloat WRITE SetFloat)
	Q_PROPERTY(double LimitedDouble READ GetLimitedDouble WRITE SetLimitedDouble)
	Q_PROPERTY(bool Bool READ GetBool WRITE SetBool)
	Q_PROPERTY(TestEnum Enum READ GetEnum WRITE SetEnum)

	Q_PROPERTY(QString QtString READ GetQtString WRITE SetQtString)
	Q_PROPERTY(std::string StdString READ GetStdString WRITE SetStdString)
	Q_PROPERTY(QString AsMultiLineString READ GetAsMultiLineString WRITE SetAsMultiLineString)
	Q_PROPERTY(QString AsPath READ GetAsPath WRITE SetAsPath)
	Q_PROPERTY(QString AsCombo READ GetAsCombo WRITE SetAsCombo)
	Q_PROPERTY(QVector2D Vec2 READ GetVec2 WRITE SetVec2)
	Q_PROPERTY(QVector3D Vec3 READ GetVec3 WRITE SetVec3)
	Q_PROPERTY(QVector4D Vec4 READ GetVec4 WRITE SetVec4)
	Q_PROPERTY(QColor Color READ GetColor WRITE SetColor)
	Q_PROPERTY(QColors Colors READ GetColors WRITE SetColors)
	Q_PROPERTY(QList<QColor> ColorList READ GetIntList WRITE SetIntList)
	Q_PROPERTY(std::vector<QColor> StdColorList READ GetStdColorList WRITE SetStdColorList)
	Q_PROPERTY(QMap<QString,QColor> ColorMap READ GetColorMap WRITE SetColorMap)


	Q_META_BEGIN(TestObject)
		Q_META_CATEGORY(Number, Int, Float, LimitedDouble, Vec2, Vec3, Vec4)
		Q_META_CATEGORY(Color,Color,Colors, ColorList, StdColorList,ColorMap)
		Q_META_CATEGORY(String, QtString, StdString, AsMultiLineString, AsPath, AsCombo)
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
	Q_ENUM(TestEnum)
private:
	QString String;
	int Int = 0;
	float Float = 0;
	double LimitedDouble = 5;
	bool Bool;
	TestEnum Enum;
	QString QtString;
	std::string StdString;
	QString AsMultiLineString;
	QString AsPath;
	QString AsCombo;
	QVector2D Vec2;
	QVector3D Vec3;
	QVector4D Vec4;
	QColor Color;
	QColors Colors;
	QList<QColor> ColorList{ Qt::red,Qt::green,Qt::blue };
	std::vector<QColor> StdColorList{ Qt::red,Qt::green,Qt::blue };
	QMap<QString, QColor> ColorMap = { {"Red",Qt::red},{"Green",Qt::green},{"Blue",Qt::blue} };
	QFile File;
	QDir Dir;
public:
	int getInt() const { return Int; }
	void setInt(int val) {
		Int = val; 
		qDebug() << Int;
	}

	QList<QColor> GetIntList() const { return ColorList; }
	void SetIntList(QList<QColor> val) {
		ColorList = val; 
		qDebug() << ColorList;
	}

	QColor GetColor() const { return Color; }
	void SetColor(QColor val) { Color = val; }

	QColors GetColors() const { return Colors; }
	void SetColors(QColors val) { Colors = val; }

	bool GetBool() const { return Bool; }
	void SetBool(bool val) { Bool = val; }

	TestObject::TestEnum GetEnum() const { return Enum; }
	void SetEnum(TestObject::TestEnum val) { Enum = val; }

	std::vector<QColor> GetStdColorList() const { return StdColorList; }
	void SetStdColorList(std::vector<QColor> val) { StdColorList = val; }

	double GetLimitedDouble() const { return LimitedDouble; }
	void SetLimitedDouble(double val) { LimitedDouble = val; }

	float GetFloat() const { return Float; }
	void SetFloat(float val) { Float = val; }

	QVector4D GetVec4() const { return Vec4; }
	void SetVec4(QVector4D val) { Vec4 = val; }

	QVector2D GetVec2() const { return Vec2; }
	void SetVec2(QVector2D val) { Vec2 = val; }

	QVector3D GetVec3() const { return Vec3; }
	void SetVec3(QVector3D val) { Vec3 = val; }

	QString GetQtString() const { return QtString; }
	void SetQtString(QString val) { QtString = val; }

	std::string GetStdString() const { return StdString; }
	void SetStdString(std::string val) { StdString = val; }

	QString GetAsMultiLineString() const { return AsMultiLineString; }
	void SetAsMultiLineString(QString val) { AsMultiLineString = val; }

	QString GetAsPath() const { return AsPath; }
	void SetAsPath(QString val) { AsPath = val; }

	QString GetAsCombo() const { return AsCombo; }
	void SetAsCombo(QString val) { AsCombo = val; }

	QMap<QString, QColor> GetColorMap() const { return ColorMap; }
	void SetColorMap(QMap<QString, QColor> val) { ColorMap = val; }
};

#endif // TestObject_h__
