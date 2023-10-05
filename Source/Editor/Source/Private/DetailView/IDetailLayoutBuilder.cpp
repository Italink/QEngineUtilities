#include "DetailView/IDetailLayoutBuilder.h"
#include "DetailView/QDetailViewRow.h"
#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/QPropertyHandle.h"
#include "QMetaProperty"
#include "Widgets/QElideLabel.h"
#include "DetailView/QDetailView.h"

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

void IDetailLayoutBuilder::SetPage(QWidget* InPage) {
	mDetailView->SetPage(InPage);
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

IDetailLayoutBuilder* IDetailLayoutBuilder::AddRowByNameValueWidget(const QString& inName, QWidget* InValue) {
	QDetailViewRow* row = NewChildRow();
	row->SetupNameValueWidget(new QElideLabel(inName), InValue);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();
}

void IDetailLayoutBuilder::AddProperty(QPropertyHandle* InPropertyHandle) {
	QDetailViewRow* row = NewChildRow();
	row->SetupPropertyHandle(InPropertyHandle);
	QSharedPointer<IPropertyTypeCustomization> customizationInstance = mPropertyTypeCustomizationMap.value(InPropertyHandle);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
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
		else {
			row->SetupNameValueWidget(InPropertyHandle->GenerateNameWidget(), InPropertyHandle->GenerateValueWidget());
			InPropertyHandle->GenerateChildrenRow(rowBuilder.get());
		}
		row->Refresh();
		row->RequestRefreshSplitter();
	});
}

void IDetailLayoutBuilder::AddObject(QObject* InObject, QString InPrePath /*= QString()*/) {
	if (InObject) {
		IDetailLayoutBuilder::ObjectContext Context;
		Context.MetaObject = InObject->metaObject();
		Context.ObjectPtr = InObject;
		Context.OwnerObject = InObject;
		Context.PrePath = InPrePath;
		AddObject(Context);
	}
	else {
		qWarning() << "IDetailLayoutBuilder::AddObject: QObject is nullptr";
	}
}

void IDetailLayoutBuilder::AddObject(IDetailLayoutBuilder::ObjectContext Context) {
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


bool IDetailLayoutBuilder::ShowChildren() const
{
	return mDetailView->GetFlags().testFlag(QDetailView::ShowChildren);
}


bool IDetailLayoutBuilder::IsIgnoreMetaObject(const QMetaObject* inMetaObj)
{
	return mDetailView->mIgnoreMetaObjects.contains(inMetaObj);
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
