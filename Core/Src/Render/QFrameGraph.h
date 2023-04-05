#ifndef QFrameGraph_h__
#define QFrameGraph_h__

#include "Render/RHI/QRhiEx.h"

class IRenderer;
class IRenderPass;
class QFrameGraphBuilder;

class QFrameGraph {
	friend class QFrameGraphBuilder;
	friend class FrameGraphView;
public:
	static QFrameGraphBuilder Begin();

	void compile(IRenderer* inRenderer);
	void render(QRhiCommandBuffer* inCmdBuffer);
	void resize(const QSize& size);
	QRhiTexture* getOutputTexture(const QString& inPassName, int inSlot);
	QRhiTexture* getOutputTexture(const QString& inPassName, const QString& inTexName);
	QRhiTexture* getOutputTexture();
private:
	void addPass(IRenderPass* inNode);
private:
	QPair<QString, int> mOutputSlot;
	QHash<QString, IRenderPass*> mRenderPassNodeMap;
	QList<IRenderPass*> mRenderPassTopology;
};

class QFrameGraphBuilder {
	friend class QFrameGraph;
public:
	QFrameGraphBuilder& addPass(IRenderPass* inNode);
	QSharedPointer<QFrameGraph> end(const QString& inOutputPass = QString(),const int & inSlot= 0);
private:
	QFrameGraphBuilder();
	QSharedPointer<QFrameGraph> mFrameGraph;
	QString mCurrentNodeName;
	QList<IRenderPass*> mRenderPassNodeList;
};

#endif // QFrameGraphBuilder_h__
