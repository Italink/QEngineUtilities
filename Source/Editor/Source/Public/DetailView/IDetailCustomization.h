#ifndef IDETAILCUSTOMIZATION_H
#define IDETAILCUSTOMIZATION_H

#include <QSharedPointer>
#include "QEngineEditorAPI.h"

class QQuickDetailsViewLayoutBuilder;

class QENGINEEDITOR_API IDetailCustomization :public QEnableSharedFromThis<IDetailCustomization> {
public:
	virtual ~IDetailCustomization(){}

	virtual void customizeDetails(const QObject* inObject, QQuickDetailsViewLayoutBuilder* inBuilder) = 0;
};

#endif // IDETAILCUSTOMIZATION_H
