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
	void setNameValueWidget(QWidget* InName, QWidget* InValue) override {
		mHeaderRow->setupNameValueWidget(InName, InValue);
	}
	void setWholeContent(QWidget* InContent) override {
		mHeaderRow->setupContentWidget(InContent);
	}
protected:
	QDetailViewRow* mHeaderRow;
};


IDetailLayoutBuilder::IDetailLayoutBuilder(QDetailView* InDetailView)
	: mDetailView(InDetailView)
{
}

void IDetailLayoutBuilder::setPage(QWidget* InPage) {
	mDetailView->setPage(InPage);
}

IDetailLayoutBuilder* IDetailLayoutBuilder::addRowByWholeContent(QWidget* Wdiget)
{
	QDetailViewRow* row = newChildRow();
	row->setupContentWidget(Wdiget);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();
}

IDetailLayoutBuilder* IDetailLayoutBuilder::addRowByNameValueWidget(QWidget* InName, QWidget* InValue)
{
	QDetailViewRow* row = newChildRow();
	row->setupNameValueWidget(InName, InValue);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();
}

IDetailLayoutBuilder* IDetailLayoutBuilder::addRowByNameValueWidget(const QString& inName, QWidget* InValue) {
	QDetailViewRow* row = newChildRow();
	row->setupNameValueWidget(new QElideLabel(inName), InValue);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	return rowBuilder.get();
}

void IDetailLayoutBuilder::addProperty(QPropertyHandle* InPropertyHandle) {
	QDetailViewRow* row = newChildRow();
	row->setupPropertyHandle(InPropertyHandle);
	QSharedPointer<IPropertyTypeCustomization> customizationInstance = mPropertyTypeCustomizationMap.value(InPropertyHandle);
	const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(rowBuilder);
	if (customizationInstance.isNull()) {
		customizationInstance = QDetailViewManager::Instance()->getCustomPropertyType(InPropertyHandle->getType());
	}
	if (!customizationInstance.isNull()) {
		HeaderRowBuilder headerBuilder(row);
		customizationInstance->customizeHeader(InPropertyHandle, &headerBuilder);
		customizationInstance->customizeChildren(InPropertyHandle, rowBuilder.get());
		mPropertyTypeCustomizationMap.insert(InPropertyHandle, customizationInstance);
	}
	else{
		row->setupNameValueWidget(InPropertyHandle->generateNameWidget(), InPropertyHandle->generateValueWidget());
		InPropertyHandle->generateChildrenRow(rowBuilder.get());
	}
	QObject::connect(InPropertyHandle, &QPropertyHandle::asRequestRebuildRow, row, [this, row, InPropertyHandle]() {
		row->clear();
		QSharedPointer<IPropertyTypeCustomization> customizationInstance = mPropertyTypeCustomizationMap.value(InPropertyHandle);
		const auto rowBuilder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
		if (customizationInstance.isNull()) {
			customizationInstance = QDetailViewManager::Instance()->getCustomPropertyType(InPropertyHandle->getType());
		}
		if (!customizationInstance.isNull()) {
			HeaderRowBuilder headerBuilder(row);
			customizationInstance->customizeHeader(InPropertyHandle, &headerBuilder);
			customizationInstance->customizeChildren(InPropertyHandle, rowBuilder.get());
			mPropertyTypeCustomizationMap.insert(InPropertyHandle, customizationInstance);
		}
		else {
			row->setupNameValueWidget(InPropertyHandle->generateNameWidget(), InPropertyHandle->generateValueWidget());
			InPropertyHandle->generateChildrenRow(rowBuilder.get());
		}
		row->refresh();
		row->requestRefreshSplitter();
	});
}

void IDetailLayoutBuilder::addObject(QObject* InObject, QString InPrePath /*= QString()*/) {
	if (InObject) {
		IDetailLayoutBuilder::ObjectContext Context;
		Context.MetaObject = InObject->metaObject();
		Context.ObjectPtr = InObject;
		Context.OwnerObject = InObject;
		Context.PrePath = InPrePath;
		addObject(Context);
	}
	else {
		qWarning() << "IDetailLayoutBuilder::AddObject: QObject is nullptr";
	}
}

void IDetailLayoutBuilder::addObject(IDetailLayoutBuilder::ObjectContext Context) {
	QSharedPointer<IDetailCustomization> customizationInstance = mClassCustomizationMap.value(Context.ObjectPtr);
	if (customizationInstance.isNull()) {
		customizationInstance = QDetailViewManager::Instance()->getCustomDetailLayout(Context.MetaObject);
	}
	if (!customizationInstance.isNull()) {
		customizationInstance->customizeDetails(Context, this);
		mClassCustomizationMap.insert(Context.ObjectPtr, customizationInstance);
	}
	else{
		for (int i = 1; i < Context.MetaObject->propertyCount(); i++) {
			QMetaProperty prop = Context.MetaObject->property(i);
			if (prop.isUser())
				continue;
			QString propertyPath = prop.name();
			if (!Context.PrePath.isEmpty()) {
				propertyPath = Context.PrePath + "." + propertyPath;
			}
			QPropertyHandle* handler = QPropertyHandle::FindOrCreate(Context.OwnerObject, propertyPath);
			if (handler) 
				addProperty(handler);
			else 
				qWarning() << "property handle is null";
		}
	}
}


bool IDetailLayoutBuilder::isChildrenVisible() const
{
	return mDetailView->getFlags().testFlag(QDetailView::isChildrenVisible);
}


bool IDetailLayoutBuilder::isIgnoredType(const QMetaObject* inMetaObj)
{
	return mDetailView->mIgnoreMetaObjects.contains(inMetaObj);
}

IDetailLayoutBuilder* IDetailLayoutBuilder::findOrAddCategory(const QString& InName){
	QSharedPointer<IDetailLayoutBuilder> builder = mCategoryMap.value(InName);
	if (builder)
		return builder.get();
	QDetailViewRow* row = newChildRow();
	row->markIsCategory();
	row->setupContentWidget(new QElideLabel(InName));
	builder = QSharedPointer<QRowLayoutBuilder>::create(mDetailView, row);
	mChildren.append(builder);
	mCategoryMap[InName] = builder;
	return builder.get();
}
