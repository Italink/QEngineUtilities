//#include "QColorsButton.hpp"
//#include <QPainter>
//#include <QHBoxLayout>
//
//QColorsButton::QColorsButton(QColors colors)
//	: mColors(colors)
//{
//	setMinimumWidth(100);
//	setFixedHeight(20);
//
//	SetColors(colors);
//	QObject::connect(this, &QPushButton::clicked, this, [&]() {
//		//ColorsDialog* dialog = new ColorsDialog;
//		//dialog->setAttribute(Qt::WA_DeleteOnClose);
//		//dialog->setColors(mColors.getStops());
//		//QObject::connect(dialog, &ColorsDialog::OnColorsChanged, this, [this](const QColors& color) {
//		//	SetColors(color);
//		//	Q_EMIT OnColorsChanged(mColors);
//		//	});
//		//dialog->show();
//	});
//}
//
//void QColorsButton::SetColors(QColors colors)
//{
//	mColors = colors;
//	update();
//}
//
//QColors QColorsButton::GetColors() const
//{
//	return mColors;
//}
//
//void QColorsButton::paintEvent(QPaintEvent* event)
//{
//	QPainter painter(this);
//	QLinearGradient linearGradient;
//	linearGradient.setStops(mColors.getStops());
//	linearGradient.setStart({ 0,0 });
//	linearGradient.setFinalStop({ (qreal)width(),0.0 });
//
//	painter.setRenderHint(QPainter::Antialiasing);
//	painter.setPen(Qt::NoPen);
//	painter.setBrush(linearGradient);
//	painter.drawRoundedRect(rect(), 2, 2);
//}