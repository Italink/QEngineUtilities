#include <QApplication>
#include "QRenderWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/PassBuilder/IMeshPassBuilder.h"
#include "Render/PassBuilder/QTextureCopyPassBuilder.h"
#include "QtConcurrent/qtconcurrentrun.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent comp;
	IMeshPassBuilder MeshPass;
	QTexutreCopyPassBuilder TextureCopyPass;
public:
	MyRenderer()
		: IRenderer(
			QRhiHelper::InitParamsBuilder()
			.backend(QRhi::Implementation::Vulkan)
		) {

		QtConcurrent::run([this]() {
			comp.setStaticMesh(QStaticMesh::CreateFromFile(R"(E:\ModernGraphicsEngineGuide\Source\Resources\Model\mandalorian\scene.gltf)"));
		});
	}
public:
protected:
	void setupGraph(QRGBuilder& graphBuilder) override {
		IMeshPassBuilder::InputParams ia;
		ia.components = { &comp };
		auto oa = graphBuilder.addPassBuilder(&MeshPass, ia);
		QTexutreCopyPassBuilder::InputParams ib;
		ib.SrcTexture = oa.baseColor;
		ib.DstRenderTarget = graphBuilder.mainRenderTarget();
		graphBuilder.addPassBuilder(&TextureCopyPass, ib);
	}
};

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QRhiHelper::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;

	QRenderWidget widget(new MyRenderer());
	widget.showMaximized();
	return app.exec();
}

