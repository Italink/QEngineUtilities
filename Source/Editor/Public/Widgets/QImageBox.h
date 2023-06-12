#ifndef QImageBox_h__
#define QImageBox_h__

#include <QImage>
#include "QHoverWidget.h"

class QImageBox : public QHoverWidget {
	Q_OBJECT
public:
	QImageBox(QImage value = QImage());
	QImage GetImage();
	void SetImage(const QImage& inImage);
	void OpenFileSelector();

	Q_SIGNAL void AsImageChanged(const QImage&);
protected:
	void paintEvent(QPaintEvent* event) override;
private:	
	QImage mImage;
};

#endif // QImageBox_h__
