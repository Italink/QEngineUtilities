#ifndef QFilePathBox_h__
#define QFilePathBox_h__

#include "QWidget"

class QHoverLineEdit;
class QPushButton; 

class QFilePathBox :public QWidget
{
	Q_OBJECT
public:
	QFilePathBox(QString inPath);
	void SetFilePath(QString inPath);
	QString GetFilePath();
Q_SIGNALS:
	void AsPathChanged(QString);
private:
	QHoverLineEdit* mLePath;
	QPushButton* mPbOpen;
};


#endif // QFilePathBox_h__
