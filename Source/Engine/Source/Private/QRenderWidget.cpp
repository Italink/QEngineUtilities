﻿#include "QRenderWidget.h"
#include "QBoxLayout"
#include "Render/Renderer/QWindowRenderer.h"
#include "QSplitter"
#include "qevent.h"
#include "QEngineCoreEntry.h"

#ifdef QENGINE_WITH_EDITOR
#include "DetailView/QDetailView.h"
#include "Widgets/QObjectTreeView.h"
#include "Utils/QEngineUndoStack.h"
#endif
#include "../../Src/qtbase/src/widgets/kernel/qapplication.h"


class QInnerRhiWindow : public QRhiWindow {
public:
	QInnerRhiWindow(QRhiWindow::InitParams inInitParams ,QRenderWidget* inWidget) 
		: QRhiWindow(inInitParams) 
		, mWidget(inWidget)
	{
	}
	void onInit() override {
		mWidget->onInit();
	}
	void onRenderTick() override {
		mWidget->onRenderTick();
	}
	void onResize(const QSize& inSize) {
		mWidget->onResize(inSize);
	}
	void onExit() override {
		mWidget->onExit();
	}
private:
	QRenderWidget* mWidget = nullptr;
};

QRenderWidget::QRenderWidget(QRhiWindow::InitParams inInitParams)
	:  mCamera(new QCamera)
#ifdef QENGINE_WITH_EDITOR
	, mDetailView(new QDetailView)
#endif
{
#ifdef QENGINE_WITH_EDITOR
	inInitParams.enableStat = true;
#endif
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	setMinimumSize(800, 600);
	mRhiWindow = new QInnerRhiWindow(inInitParams, this);
	mViweport = QWidget::createWindowContainer(mRhiWindow);
#ifdef QENGINE_WITH_EDITOR
	QSplitter* splitter = new QSplitter;
	mViweport->setMinimumWidth(400);
	splitter->addWidget(mViweport);
	splitter->addWidget(mDetailView);
	splitter->setSizes({ 700,300 });
	mRhiWindow->installEventFilter(this);
	//connect(mObjectTreeView, &QObjectTreeView::AsObjecteSelected, mDetailView, &QDetailView::SetObject);
	hLayout->addWidget(splitter);
#else
	hLayout->addWidget(mViweport);
#endif
}

QCamera* QRenderWidget::setupCamera() {
	mCamera->setupWindow(mRhiWindow);
	return mCamera;
}

IRenderer* QRenderWidget::getRenderer() {
	return mRenderer.get();
}

void QRenderWidget::setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph) {
	mFrameGraph = inFrameGraph;
	requestCompileRenderer();
}

void QRenderWidget::requestCompileRenderer() {
	sigRecompileRenderer.request();
}

QWindow* QRenderWidget::getRhiWindow() {
	return mRhiWindow;
}

QWidget* QRenderWidget::getViweport() const {
	return mViweport;
}

void QRenderWidget::showAndWaitInitialized() {
	show();
	while (!bInitialized) {
		QApplication::processEvents();
	}
}

void QRenderWidget::onInit() {
	bInitialized = true;
	mRenderer.reset(new QWindowRenderer(mRhiWindow));
	mRenderer->setCamera(mCamera);
#ifdef QENGINE_WITH_EDITOR
	mDetailView->SetFlags(QDetailView::ShowChildren);
	//connect(mRenderer.get(), &IRenderer::asCurrentObjectChanged, this, [this](QObject* object) {
	//	mObjectTreeView->SelectObjects({ object });
	//	mDetailView->SetObject(object);
	//});
	//connect(mObjectTreeView, &QObjectTreeView::AsObjecteSelected, this, [this](QObject* object) {
	//	if (mRenderer->getCurrentObject() != object) {
	//		mRenderer->setCurrentObject(object);
	//	}
	//});
	//mObjectTreeView->SetObjects({ mRenderer.get() });
	//mObjectTreeView->expandAll();
#endif
}

void QRenderWidget::onRenderTick() {
	if (sigRecompileRenderer.ensure()) {
		mRenderer->setFrameGraph(mFrameGraph);
		mRenderer->compile();
#ifdef QENGINE_WITH_EDITOR
		mDetailView->SetObject(mRenderer.get());
#endif
	}
	mRenderer->render();
}

void QRenderWidget::onResize(const QSize& inSize) {
	mRenderer->resize(inSize);
}

void QRenderWidget::onExit() {
	mRenderer.reset();
}

#ifdef QENGINE_WITH_EDITOR
void QRenderWidget::keyPressEvent(QKeyEvent* event) {
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ) {
		if (event->key() == Qt::Key_Z) {
			QEngineUndoStack::Instance()->Undo();
		}
		else if (event->key() == Qt::Key_Y) {
			QEngineUndoStack::Instance()->Redo();
		}
	}
	Q_EMIT QEngineCoreEntry::Instance()->asViewportKeyPressEvent(event);
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