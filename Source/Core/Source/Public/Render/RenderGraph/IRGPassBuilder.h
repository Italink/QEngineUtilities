#ifndef IRGPassBuilder_h__
#define IRGPassBuilder_h__

#include "QRGBuilder.h"

class IRGPassBuilder {
	friend class QRGBuilder;
protected:
	virtual void setup(QRGBuilder& builder) = 0;
	virtual void execute(QRhiCommandBuffer* cmdBuffer) = 0;
};

#endif // IRGPassBuilder_h__
