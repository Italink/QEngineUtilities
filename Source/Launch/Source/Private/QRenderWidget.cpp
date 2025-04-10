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
	mViewport = QWidget::createWindowContainer(renderer->maybeWindow());
	renderer->maybeWindow()->installEventFilter(this);
#ifdef QENGINE_WITH_EDITOR
	QSplitter* splitter = new QSplitter;
	mViewport->setMinimumWidth(400);
	splitter->addWidget(mViewport);
	splitter->addWidget(mDetailView);
	splitter->setSizes({ 700,300 });
	hLayout->addWidget(splitter);
	mDetailView->setFlags(QDetailView::isChildrenVisible);
	mDetailView->setObject(mRenderer->getCurrentObject()? mRenderer->getCurrentObject() : mRenderer);
	connect(mRenderer, &IRenderer::currentObjectChanged, mDetailView, [this](QObject* Object) {
		if (Object == nullptr) {
			mDetailView->setObject(mRenderer);
		}
		else {
			mDetailView->setObject(Object);
		}
	});
#else
	hLayout->addWidget(mViewport);
#endif
}

void QRenderWidget::keyPressEvent(QKeyEvent* event) {
#ifdef QENGINE_WITH_EDITOR
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
		if (event->key() == Qt::Key_Z) {
			QEngineUndoStack::Instance()->undo();
		}
		else if (event->key() == Qt::Key_Y) {
			QEngineUndoStack::Instance()->redo();
		}
	}
#endif // QENGINE_WITH_EDITOR
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