#include "QRenderWidget.h"
#include "QBoxLayout"
#include "QSplitter"
#include "qevent.h"
#include "QEngineCoreSignals.h"
#include "Render/IRenderer.h"
#include <QWindow>

#ifdef QENGINE_WITH_EDITOR
#include "DetailView/QDetailView.h"
#include "Utils/QEngineUndoStack.h"
#endif


QRenderWidget::QRenderWidget(IRenderer* renderer)
	: mRenderer (renderer)
#ifdef QENGINE_WITH_EDITOR
	, mDetailView(new QDetailView)
#endif
{
	Q_ASSERT(renderer->maybeWindow());
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	setMinimumSize(800, 600);
	mViweport = QWidget::createWindowContainer(renderer->maybeWindow());
#ifdef QENGINE_WITH_EDITOR
	QSplitter* splitter = new QSplitter;
	mViweport->setMinimumWidth(400);
	splitter->addWidget(mViweport);
	splitter->addWidget(mDetailView);
	splitter->setSizes({ 700,300 });
	renderer->maybeWindow()->installEventFilter(this);
	hLayout->addWidget(splitter);
	mDetailView->setFlags(QDetailView::isChildrenVisible);
	mDetailView->setObject(mRenderer);
	connect(mRenderer, &IRenderer::currentObjectChanged, mDetailView, &QDetailView::selectSubObject);
#else
	hLayout->addWidget(mViweport);
#endif

}

#ifdef QENGINE_WITH_EDITOR
void QRenderWidget::keyPressEvent(QKeyEvent* event) {
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ) {
		if (event->key() == Qt::Key_Z) {
			QEngineUndoStack::Instance()->undo();
		}
		else if (event->key() == Qt::Key_Y) {
			QEngineUndoStack::Instance()->redo();
		}
	}
	Q_EMIT QEngineCoreSignals::Instance()->asViewportKeyPressEvent(event);
}

bool QRenderWidget::eventFilter(QObject* obj, QEvent* event) {
	if (obj != nullptr) {
		switch (event->type()) {
		case QEvent::KeyPress:
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			QRenderWidget::keyPressEvent(keyEvent);
		}
	}
	return false;
}
#endif // QENGINE_WITH_EDITOR