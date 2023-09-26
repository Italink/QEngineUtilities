#ifndef IRenderPassBuilder_h__
#define IRenderPassBuilder_h__

#include "QRenderGraphBuilder.h"

#define QRP_INPUT_BEGIN(PassBuilderClass) \
			friend class Input; \
			friend class Output; \
			friend class QRenderGraphBuilder; \
			public: \
			struct Output;	\
			struct Input {	\
				friend class PassBuilderClass; \
				friend class QRenderGraphBuilder; \
				private: \
					PassBuilderClass* mPassBuilder; \
					QRenderGraphBuilder* mRGBuilder; \
				public: \
					Output setup() { \
						mPassBuilder->mInput = *this; \
						mPassBuilder->setup(*mRGBuilder); \
						mRGBuilder->addPass(std::bind(&PassBuilderClass::execute, mPassBuilder, std::placeholders::_1)); \
						return mPassBuilder->mOutput; \
					} \
					operator Output() { return setup();}

#define QRP_INPUT_ATTR(Type,Name) \
				public: Input& set##Name(Type val) { _##Name = val; return *this; } \
				private: Type _##Name

#define QRP_INPUT_END()	\
		}; \
		private: Input mInput;


#define QRP_OUTPUT_BEGIN(PassBuilderClass) public: struct Output {

#define QRP_OUTPUT_ATTR(Type,Name)  Type Name;

#define QRP_OUTPUT_END()	}; \
		private: Output mOutput;

class IRenderPassBuilder {
	friend class QRenderGraphBuilder;
protected:
	virtual void setup(QRenderGraphBuilder& builder) = 0;
	virtual void execute(QRhiCommandBuffer* cmdBuffer) = 0;
};

#endif // IRenderPassBuilder_h__
