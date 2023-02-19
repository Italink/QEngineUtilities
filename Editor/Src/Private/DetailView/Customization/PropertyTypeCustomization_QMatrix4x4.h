#ifndef PropertyTypeCustomization_QMatrix4x4_h__
#define PropertyTypeCustomization_QMatrix4x4_h__

#include "DetailView/IPropertyTypeCustomization.h"

class PropertyTypeCustomization_QMatrix4x4 :public IPropertyTypeCustomization {
public:
	void CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) override;
	void CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) override;
};

#endif // PropertyTypeCustomization_QMatrix4x4_h__
