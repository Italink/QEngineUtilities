#include "DetailCustomization_QObject.h"
#include <QMetaProperty>
#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"

void DetailCustomization_QObject::CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context ,IDetailLayoutBuilder* Builder) {
	//bool bCategoryEnabled = metaData->mClassMetaData.value("CategoryEnabled").toBool();
	//if(bCategoryEnabled){
	//	for(const auto& category : metaData->mCategories.asKeyValueRange()){
	//		if (!category.second)
	//			Builder->FindOrAddCategory(category.first);
	//	}
	//}
	for (int i = 1; i < Context.MetaObject->propertyCount(); i++) {
		QMetaProperty prop = Context.MetaObject->property(i);
		QString propertyPath = prop.name();
		if(!Context.PrePath.isEmpty()){
			propertyPath = Context.PrePath + "." + propertyPath;
		}
		QPropertyHandle* handler = QPropertyHandle::FindOrCreate(Context.OwnerObject, propertyPath);
		if(handler){
			//if (bCategoryEnabled){
			//	QString category = handler->GetMetaData("Category").toString();
			//	if (category.isEmpty())
			//		category = "Other";
			//	Builder->FindOrAddCategory(category)->AddProperty(handler);
			//}
			//else{
				Builder->AddProperty(handler);
			//}
		}
		else{
			qWarning() << "property handle is null";
		}
	}
}
