#include "QRenderWidget.h"
#include "QBoxLayout"
#include "Render/Renderer/QWindowRenderer.h"
#include "QSplitter"

#ifdef WITH_EDITOR
#include "DetailView/QDetailView.h"
#include "Widgets/QObjectTreeView.h"
#endif

class QInnerRhiWindow : public QRhiWindow {
public:
	QInnerRhiWindow(QRhiWindow::InitParams inInitParams) 
		:QRhiWindow(inInitParams) {
	}
	void setTickFunctor(std::function<void()> inFunctor) {
		mTickFunctor = inFunctor;
	}
	void setResizeFunctor(std::function<void(const QSize&)> inFunctor) {
		mResizeFunctor = inFunctor;
	}
	void onRenderTick() override {
		mTickFunctor();
	}
	void onResizeEvent(const QSize& inSize) {
		mResizeFunctor(inSize);
	}
private:
	std::function<void()> mTickFunctor = []() {};
	std::function<void(const QSize&)> mResizeFunctor = [](const QSize&) {};
};

QRenderWidget::QRenderWidget(QRhiWindow::InitParams inInitParams)
	: mRhiWindow(new QInnerRhiWindow(inInitParams))
	, mCamera(new QCamera)
#ifdef WITH_EDITOR
	, mDetailView(new QDetailView)
	, mObjectTreeView(new QObjectTreeView) 
#endif
{
	mRhiWindow->setTickFunctor([this]() {
		if (sigRecompileRenderer.receive()) {
			if (mRenderer == nullptr) {
				mRenderer = new QWindowRenderer(mRhiWindow);
				mRenderer->setCamera(mCamera);
#ifdef WITH_EDITOR
				connect(mRenderer, &IRenderer::asCurrentObjectChanged, this, [this](QObject* object) {
					//mObjectTreeView->SetSelectedInstance(QInstance::CreateObjcet(object));
				});
				connect(mObjectTreeView, &QObjectTreeView::AsObjecteSelected, this, [this](QObject* object) {
					if (mRenderer->getCurrentObject() != object) {
						mRenderer->setCurrentObject(object);
					}
				});
				mObjectTreeView->SetObjects({ mRenderer });
#endif
			}
			mRenderer->setFrameGraph(mFrameGraph);
			mRenderer->complie();
		}
		mRenderer->render();
	});
	mRhiWindow->setResizeFunctor([this](const QSize& inSize) {
		if (mRenderer) {
			mRenderer->resize(inSize);
		}
	});
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	QWidget* viewport = QWidget::createWindowContainer(mRhiWindow);
#ifdef WITH_EDITOR
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
