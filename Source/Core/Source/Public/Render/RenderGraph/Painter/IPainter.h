#ifndef IPainter_h__
#define IPainter_h__

#include "Render/RHI/QRhiHelper.h"	
#include "Render/RenderGraph/QRenderGraphBuilder.h"

class IPainter {
public:
	virtual void setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt) = 0;
	virtual void resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi) {}
	virtual void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) = 0;
};


#endif // IPainter_h__
