#ifndef PropertyTypeCustomization_QStaticMesh_h__
#define PropertyTypeCustomization_QStaticMesh_h__

#include "DetailView/IPropertyTypeCustomization.h"
#include "QComboBox"

class PropertyTypeCustomization_QStaticMesh :public IPropertyTypeCustomization {
public:
	void CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) override;
	void CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) override;
private:
	QComboBox* CurrComboBox = nullptr;
};

#endif // PropertyTypeCustomization_QStaticMesh_h__
