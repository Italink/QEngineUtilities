#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "qbasicatomic.h"
#include <QQuickView>
#include <QQmlContext>
#include "DetailView/QQuickDetailsView.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	TestObject obj;
	qmlRegisterType<QQuickDetailsView>("Qt.DetailsView", 1, 0, "DetailsView");
	QQuickView view;
	view.setSource(QUrl::fromLocalFile("F:/EchoX/3rdParty/QEngineUtilities/Examples/DetailViewExample/Main.qml"));
	view.show();
	view.rootObject()->setProperty("Object", QVariant::fromValue(&obj));
	return app.exec();
}