#ifndef QFilePathBox_h__
#define QFilePathBox_h__

#include "QWidget"
#include "QEngineEditorAPI.h"

class QHoverLineEdit;
class QPushButton; 

class QENGINEEDITOR_API QFilePathBox :public QWidget
{
	Q_OBJECT
public:
	QFilePathBox(QString inPath = QString());
	void setFilePath(QString inPath);
	QString getFilePath();
Q_SIGNALS:
	void asPathChanged(QString);
private:
	QHoverLineEdit* mLePath;
	QPushButton* mPbOpen;
};

#endif // QFilePathBox_h__
