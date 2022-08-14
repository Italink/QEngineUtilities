#ifndef QDetailWidgetStyleManager_h__
#define QDetailWidgetStyleManager_h__

#include "QHash"
#include "QString"

enum class QDetailWidgetStyle {
	Qt,
	Unreal
};

class QDetailWidgetStyleManager {
public:
	static QDetailWidgetStyleManager* Instance();
	QByteArray GetStylesheet();
private:
	QDetailWidgetStyleManager();
	QDetailWidgetStyle mCurrentStyle = QDetailWidgetStyle::Qt;
	QHash<QDetailWidgetStyle, QByteArray> mStylesheetMap;
};

#endif // QDetailWidgetStyleManager_h__
