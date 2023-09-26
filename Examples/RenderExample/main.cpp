#include <QApplication>
#include "QRenderWidget.h"
#include "Render/Pass/QBasePassForward.h"
#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/RenderGraph/PassBuilder/QMeshPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QTexturePassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QBlurPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QVideoPassBuilder.h"
#include "Render/RenderGraph/PassBuilder/QImGUIPassBuilder.h"
#include "QtConcurrent/qtconcurrentrun.h"

class MyRenderer : public IRenderer {
private:
	QStaticMeshRenderComponent comp;
	QMeshPassBuilder MeshPass;
	QBlurPassBuilder BlurPass;
	QTexutrePassBuilder TextureCopyPass;
	QVideoPassBuilder VideoPass;
	QImGUIPassBuilder ImGuiPass;
public:
	MyRenderer(): IRenderer(
			QRhiHelper::InitParamsBuilder()
			.backend(QRhi::Implementation::Vulkan)
		) {

		QtConcurrent::run([this]() {
			comp.setStaticMesh(QStaticMesh::CreateFromFile(R"(E:\MyProjects\ModernGraphicsEngineGuide\Source\Resources\Model\mandalorian\scene.gltf)"));
		});
		
		VideoPass.setVideoPath(QUrl::fromLocalFile(R"(E:\MyProjects\ModernGraphicsEngineGuide\Source\Resources\Video\BadApple.mp4)"));
	}
protected:
	void setupGraph(QRenderGraphBuilder& graphBuilder) override {
		//QMeshPassBuilder::Output aout
		//	= graphBuilder.addPassBuilder(&MeshPass)
		//		.setComponents({&comp});

		//QBlurPassBuilder::Output bout
		//	= graphBuilder.addPassBuilder(&BlurPass)
		//		.setBaseColorTexture(aout.BaseColor);

		//QVideoPassBuilder::Output bout
		//	= graphBuilder.addPassBuilder(&VideoPass);

		QImGUIPassBuilder::Output bout
			= graphBuilder.addPassBuilder(&ImGuiPass)
				.setPaintFunctor([](ImGuiContext* Ctx) {
					ImGui::SetCurrentContext(Ctx);
					ImGui::ShowFontSelector("Font");
					ImGui::ShowStyleSelector("Style");
					ImGui::ShowDemoWindow();
				});


		QTexutrePassBuilder::Output cout
			= graphBuilder.addPassBuilder(&TextureCopyPass)
				.setBaseColorTexture(bout.ImGuiTexture)
				.setDstRenderTarget(graphBuilder.mainRenderTarget());
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

