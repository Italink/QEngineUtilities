#include "CodeEditor/QGLSLEditor.h"
#include "GLSLLexer.h"

QGLSLEditor::QGLSLEditor()
	:QCodeEditor(new QscilexerGLSL)
{
	for (int i = 1; i <= 4; i++) {
		for (auto& keyword : QString(mLexer->keywords(i)).split(" "))
			mApis->add(keyword);
	}
	QscilexerGLSL* lexer = dynamic_cast<QscilexerGLSL*>(mLexer);
	if (lexer != nullptr) {
		for (auto& func : lexer->functions()) {
			mApis->add(func);
		}
	}
	mApis->prepare();
}