#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"

class MyRenderer : public IRenderer {
	using IRenderer::IRenderer;
protected:
	void setupGraph(QRGBuilder& graphBuilder) override {
		graphBuilder.addPass([&](QRhiCommandBuffer* cmdBuffer) {
			cmdBuffer->beginPass(graphBuilder.mainRenderTarget(), QColor::fromRgbF(0.1f, 0.5f, 0.9f, 1.0f), { 1.0f, 0 });
			cmdBuffer->endPass();
		});
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;

	QRenderWidget widget(new MyRenderer(initParams, {800,600}));
	widget.showMaximized();
	return app.exec();
}

