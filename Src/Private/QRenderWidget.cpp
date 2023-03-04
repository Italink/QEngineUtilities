﻿#include "QRenderWidget.h"
#include "QBoxLayout"
#include "Render/Renderer/QWindowRenderer.h"
#include "QSplitter"
#include "qevent.h"

#ifdef QENGINE_WITH_EDITOR
#include "DetailView/QDetailView.h"
#include "Widgets/QObjectTreeView.h"
#include "Utils/QEngineUndoStack.h"
#endif


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
	void onResizeEvent(const QSize& inSize) {
		mWidget->onResizeEvent(inSize);
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
	, mObjectTreeView(new QObjectTreeView) 
#endif
{
#ifdef QENGINE_WITH_EDITOR
	inInitParams.enableStat = true;
#endif
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	mRhiWindow = new QInnerRhiWindow(inInitParams, this);
	QWidget* viewport = QWidget::createWindowContainer(mRhiWindow);
#ifdef QENGINE_WITH_EDITOR
	QSplitter* splitter = new QSplitter;
	viewport->setMinimumWidth(400);
	QSplitter* panel = new QSplitter(Qt::Vertical);
	panel->addWidget(mObjectTreeView);
	panel->addWidget(mDetailView);
	panel->setSizes({ 1, 10 });
	splitter->addWidget(viewport);
	splitter->addWidget(panel);
	connect(mObjectTreeView, &QObjectTreeView::AsObjecteSelected, mDetailView, &QDetailView::SetObject);
	hLayout->addWidget(splitter);
#else
	hLayout->addWidget(viewport);
#endif
}

QCamera* QRenderWidget::setupCamera() {
	mCamera->setupWindow(mRhiWindow);
	return mCamera;
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

void QRenderWidget::onInit() {
	mRenderer.reset(new QWindowRenderer(mRhiWindow));
	mRenderer->setCamera(mCamera);
#ifdef QENGINE_WITH_EDITOR
	connect(mRenderer.get(), &IRenderer::asCurrentObjectChanged, this, [this](QObject* object) {
		mObjectTreeView->SelectObjects({ object });
		mDetailView->SetObject(object);
	});
	connect(mObjectTreeView, &QObjectTreeView::AsObjecteSelected, this, [this](QObject* object) {
		if (mRenderer->getCurrentObject() != object) {
			mRenderer->setCurrentObject(object);
		}
	});
	mObjectTreeView->SetObjects({ mRenderer.get() });
	mObjectTreeView->expandAll();
#endif
}

void QRenderWidget::onRenderTick() {
	if (sigRecompileRenderer.receive()) {
		mRenderer->setFrameGraph(mFrameGraph);
		mRenderer->compile();
	}
	mRenderer->render();
}

void QRenderWidget::onResizeEvent(const QSize& inSize) {
	mRenderer->resize(inSize);
}

void QRenderWidget::onExit() {
	mRenderer.reset();
}

void QRenderWidget::keyPressEvent(QKeyEvent* event) {
#ifdef QENGINE_WITH_EDITOR
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ) {
		if (event->key() == Qt::Key_Z) {
			QEngineUndoStack::Instance()->Undo();
		}
		else if (event->key() == Qt::Key_Z) {
			QEngineUndoStack::Instance()->Redo();
		}
	}
#endif // QENGINE_WITH_EDITOR
}