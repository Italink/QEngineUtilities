#ifndef QFrameGraph_h__
#define QFrameGraph_h__

#include "Render/RHI/QRhiEx.h"

class IRenderer;
class IRenderPassBase;

class QFrameGraph {
	friend class QFrameGraphBuilder;
public:
	void rebuildTopology();
	QRhiTexture* getOutputTexture();
	const QList<IRenderPassBase*>& getRenderPassTopology() const { return mRenderPassTopology; }
	const QHash<QString, IRenderPassBase*>& getRenderPassMap() const { return mRenderPassMap; }
private:
	QHash<IRenderPassBase*, QList<IRenderPassBase*>> mDependMap;
	QHash<QString, IRenderPassBase*> mRenderPassMap;
	QList<IRenderPassBase*> mRenderPassTopology;
	QPair<QString, int> mOutput;
};

class QFrameGraphBuilder {
public:
	static QFrameGraphBuilder* begin();
	QFrameGraphBuilder* addPass(const QString& inName, IRenderPassBase* inRenderPass);
	QSharedPointer<QFrameGraph> end(const QString& outPass, const int& outPassSlot);
private:
	QFrameGraphBuilder();
	QSharedPointer<QFrameGraph> mFrameGraph;
	QString mCurrentNodeName;
	QList<IRenderPassBase*> mRenderPassNodeList;
};

#endif // QFrameGraphBuilder_h__
