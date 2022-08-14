#include "QInstanceDetail_QObject.h"

bool QInstanceDetail_QObject::Filter(const QSharedPointer<QInstance> inInstance) {
	return inInstance->GetMetaObject()->inherits(&QObject::staticMetaObject);
}

void QInstanceDetail_QObject::Build() {
	for (int i = 1; i < mInstance->GetMetaObject()->propertyCount(); i++) {
		QMetaProperty prop = mInstance->GetMetaObject()->property(i);
		QPropertyHandler* propHandler = mInstance->CreatePropertyHandler(prop);
		AddProperty(propHandler);
	}
}
