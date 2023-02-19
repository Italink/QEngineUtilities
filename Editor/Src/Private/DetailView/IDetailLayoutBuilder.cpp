#include "DetailView/IDetailLayoutBuilder.h"
#include "QDetailViewRow.h"
#include "QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/QPropertyHandle.h"
#include "QMetaProperty"
#include "Widgets/QElideLabel.h"

class HeaderRowBuilder: public IHeaderRowBuilder{
public:
	HeaderRowBuilder(QDetailViewRow* inRow): mHeaderRow(inRow){}
	void AsNameValueWidget(QWidget* InName, QWidget* InValue) override {
		mHeaderRow->SetupNameValueWidget(InName, InValue);
	}
	void AsWholeContent(QWidget* InContent) override {
		mHeaderRow->SetupContentWidget(InContent);
	}
protected:
	QDetailViewRow* mHeaderRow;
};


IDetailLayoutBuilder::IDetailLayoutBuilder(QDetailView* InDetailView)
	: mDetailView(InDetailView)
{
}

IDetailLayoutBuilder* IDetailLayoutBuilder::AddRowByWholeContent(QWidget* Wdiget)
{
	QDetailViewRow* row = NewChildRow();
	row->SetupContentWidget(Wdiget);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();

}

IDetailLayoutBuilder* IDetailLayoutBuilder::AddRowByNameValueWidget(QWidget* InName, QWidget* InValue)
{
	QDetailViewRow* row = NewChildRow();
	row->SetupNameValueWidget(InName, InValue);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();
}

void IDetailLayoutBuilder::AddProperty(QPropertyHandle* InPropertyHandle) {
;	QDetailViewRow* row = NewChildRow();
	QSharedPointer<IPropertyTypeCustomization> customizationInstance = mPropertyTypeCustomizationMap.value(InPropertyHandle);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	if (customizationInstance.isNull()) {
		customizationInstance = QDetailViewManager::Instance()->GetCustomPropertyType(InPropertyHandle->GetType());
	}
	if (!customizationInstance.isNull()) {
		HeaderRowBuilder headerBuilder(row);
		customizationInstance->CustomizeHeader(InPropertyHandle, &headerBuilder);
		customizationInstance->CustomizeChildren(InPropertyHandle, rowBuilder.get());
		mPropertyTypeCustomizationMap.insert(InPropertyHandle, customizationInstance);
	}
	else{
		row->SetupNameValueWidget(InPropertyHandle->GenerateNameWidget(), InPropertyHandle->GenerateValueWidget());
		InPropertyHandle->GenerateChildrenRow(rowBuilder.get());
	}
	QObject::connect(InPropertyHandle, &QPropertyHandle::AsRequestRebuildRow, row, [this, row, InPropertyHandle]() {
		row->DeleteChildren();
		row->SetupNameValueWidget(InPropertyHandle->GenerateNameWidget(), InPropertyHandle->GenerateValueWidget());
		const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
		InPropertyHandle->GenerateChildrenRow(rowBuilder.get());
		row->Refresh();
	});
}

void IDetailLayoutBuilder::AddObject(QObject* InObject, QString InPrePath /*= QString()*/, bool HideHeader /*= true*/) {
	IDetailLayoutBuilder::ObjectContext Context;
	Context.MetaObject = InObject->metaObject();
	Context.ObjectPtr = InObject;
	Context.OwnerObject = InObject;
	Context.PrePath = InPrePath;
	AddObject(Context, HideHeader);
}


void IDetailLayoutBuilder::AddObject(IDetailLayoutBuilder::ObjectContext Context, bool HideHeader /*= true*/) {
	QSharedPointer<IDetailCustomization> customizationInstance = mClassCustomizationMap.value(Context.ObjectPtr);
	if (customizationInstance.isNull()) {
		customizationInstance = QDetailViewManager::Instance()->GetCustomDetailLayout(Context.MetaObject);
	}
	if (!customizationInstance.isNull()) {
		customizationInstance->CustomizeDetails(Context, this);
		mClassCustomizationMap.insert(Context.ObjectPtr, customizationInstance);
	}
	else{
		for (int i = 0; i < Context.MetaObject->propertyCount(); i++) {
			QMetaProperty prop = Context.MetaObject->property(i);
			QString propertyPath = prop.name();
			if (!Context.PrePath.isEmpty()) {
				propertyPath = Context.PrePath + "." + propertyPath;
			}
			QPropertyHandle* handler = QPropertyHandle::FindOrCreate(Context.OwnerObject, propertyPath);
			if (handler) 
				AddProperty(handler);
			else 
				qWarning() << "property handle is null";
		}
	}
}

IDetailLayoutBuilder* IDetailLayoutBuilder::FindOrAddCategory(const QString& InName){
	QSharedPointer<IDetailLayoutBuilder> builder = mCategoryMap.value(InName);
	if (builder)
		return builder.get();
	QDetailViewRow* row = NewChildRow();
	row->MarkIsCategory();
	row->SetupContentWidget(new QElideLabel(InName));
	builder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(builder);
	mCategoryMap[InName] = builder;
	return builder.get();
}
