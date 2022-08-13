#include "QInstanceDetail_QObject.h"

bool QInstanceDetail_QObject::Filter(const QSharedPointer<QInstance> inInstance) {
	return inInstance->GetMetaObject()->inherits(&QObject::staticMetaObject);
}

void QInstanceDetail_QObject::Build() {
	for (int i = 1; i < mInstance.lock()->GetMetaObject()->propertyCount(); i++) {
		QMetaProperty prop = mInstance.lock()->GetMetaObject()->property(i);
		QPropertyHandler* propHandler = mInstance.lock()->CreatePropertyHandler(prop);
		AddProperty(propHandler);
	}
}
