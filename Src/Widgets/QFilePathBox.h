#ifndef QFilePathBox_h__
#define QFilePathBox_h__

#include "QWidget"

class QLineEdit; 
class QPushButton; 

class QFilePathBox :public QWidget
{
	Q_OBJECT
public:
	QFilePathBox(QString inPath);
Q_SIGNALS:
	void AsPathChanged(QString);
private:
	QLineEdit* mLePath;
	QPushButton* mPbOpen;
};


#endif // QFilePathBox_h__
