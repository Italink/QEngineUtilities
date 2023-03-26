#include "DetailCustomization_QGlslSandboxRenderPass.h"
#include "DetailView/QPropertyHandle.h"
#include "DetailView/QDetailViewManager.h"
#include "CodeEditor/QGLSLEditor.h"
#include "DetailView/QDetailViewRow.h"

void DetailCustomization_QGlslSandboxRenderPass::CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QGlslSandboxRenderPass* pass = (QGlslSandboxRenderPass*)Context.ObjectPtr;
	QGLSLEditor* editor = new QGLSLEditor;
	editor->setText(pass->getShaderCode());
	Builder->SetPage(editor);
}
