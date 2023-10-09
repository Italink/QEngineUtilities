#ifndef QImageBox_h__
#define QImageBox_h__

#include <QImage>
#include "QHoverWidget.h"

class QENGINEEDITOR_API QImageBox : public QHoverWidget {
	Q_OBJECT
public:
	QImageBox(QImage value = QImage());
	QImage getImage();
	void setImage(const QImage& inImage);
	void openFileSelector();

	Q_SIGNAL void asImageChanged(const QImage&);
protected:
	void paintEvent(QPaintEvent* event) override;
private:	
	QImage mImage;
};

#endif // QImageBox_h__
