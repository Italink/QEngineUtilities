#ifndef QInstanceDetail_QObject_h__
#define QInstanceDetail_QObject_h__

#include "QInstanceDetail.h"

class QInstanceDetail_QObject: public QInstanceDetail {
	Q_OBJECT
public:
	static bool Filter(const QSharedPointer<QInstance> inInstance);
	virtual void Build() override;
};

#endif // QInstanceDetail_QObject_h__
