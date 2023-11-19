#ifndef QSvgIcon_h__
#define QSvgIcon_h__

#include <functional>
#include <QList>
#include <QMap>
#include <QColor>
#include <QIcon>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QSvgIcon {
public:
	QSvgIcon(QString path, QString category, QColor initialColor = Qt::black);
	~QSvgIcon();

	using IconUpdateCallBack = std::function<void()>;

	const QIcon& getIcon();
	const QIcon& getIcon() const;

	void setUpdateCallBack(IconUpdateCallBack callback);

	void setPath(QString path);

	QColor getColor() const;
	void setColor(QColor val);

	static void setIconColor(QString category, QColor color);
private:
	void updateIcon();
private:
	QString mCategory;
	QString mPath;
	QColor mColor;
	QIcon mIcon;
	IconUpdateCallBack mCallBack;
	inline static QMap<QString,QList<QSvgIcon*>> IconMap;
};  

#endif // QSvgIcon_h__
