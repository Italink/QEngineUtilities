#include "QDetailWidgetPropertyInstanceItem.h"
#include "Core\QInstance.h"
#include "Core\Undo\QDetailUndoStack.h"
#include "Customization\QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "QComboBox"
#include "QMetaType"

bool QDetailWidgetPropertyInstanceItem::FilterType(QMetaType inID) {
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(inID.name(),0,QRegularExpression::MatchType::PartialPreferCompleteMatch,QRegularExpression::AnchorAtOffsetMatchOption);
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		QString metaTypeName = matchTexts.back() + "*";
		QMetaType innerMetaType = QMetaType::fromName(metaTypeName.toLocal8Bit());
		if (innerMetaType.isValid()) {
			return innerMetaType.metaObject()!=nullptr;
		}
		else {
			qWarning() << QString("You will have to register %1 with Q_DECLARE_METATYPE() and qRegisterMetaType().").arg(metaTypeName);
		}
	}
	return inID.metaObject() != nullptr;
}

void QDetailWidgetPropertyInstanceItem::SetHandler(QPropertyHandler* inHandler)
{
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	connect(GetHandler()->GetUndoEntry(), &QDetailUndoEntry::AsUndo, this, &QDetailWidgetPropertyInstanceItem::RecreateChildren);
	connect(GetHandler()->GetUndoEntry(), &QDetailUndoEntry::AsRedo, this, &QDetailWidgetPropertyInstanceItem::RecreateChildren);
}

void QDetailWidgetPropertyInstanceItem::ResetValue()
{
	QDetailWidgetPropertyItem::ResetValue();
	RecreateChildren();
}

void QDetailWidgetPropertyInstanceItem::RecreateInstance() {
	mInstanceVar = GetValue();
	QMetaType metaType(GetHandler()->GetType());
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(metaType.name());
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		QMetaType innerMetaType = QMetaType::fromName((matchTexts.back() + "*").toLocal8Bit());
		mMetaObject = innerMetaType.metaObject();
		const void* ptr = *(const void**)mInstanceVar.data();
		bIsSharedPointer = true;
		bIsPointer = true;
		if (ptr) {
			mInstanceVar = QVariant(innerMetaType, mInstanceVar.data());
		}
		else {
			mInstanceVar = QVariant();
			mInstance.reset();
		}
	}
	else {
		bIsPointer = metaType.flags().testFlag(QMetaType::IsPointer);
		mMetaObject = metaType.metaObject();
	}
	if (mInstanceVar.isValid()) {
		if (mMetaObject->inherits(&QObject::staticMetaObject)) {
			QObject* object = mInstanceVar.value<QObject*>();
			if (object) {
				if (mInstance)
					mInstance->UpdateDataPtr(object);
				else
					mInstance = QInstance::CreateObjcet(object);
			}
		}
		else {
			void* ptr = mInstanceVar.data();
			if(bIsPointer)
				ptr = *(void**)mInstanceVar.data();
			if (mInstance)
				mInstance->UpdateDataPtr(ptr);
			else
				mInstance = QInstance::CreateGadget(ptr, mMetaObject);
			if (!bIsPointer) {
				mInstance->SetPropertyChangedCallback([this]() {
					GetHandler()->SetValue(mInstanceVar);
				});
			}
		}
	}
}

void QDetailWidgetPropertyInstanceItem::RecreateChildren()
{
	RecreateInstance();
	ReflushInstanceComboBox();
	while (childCount() > 0) {
		delete takeChild(0);
	}
	if (mInstance && mInstance->IsValid()) {
		QInstanceDetail* detail = QInstanceDetail::FindOrCreate(mInstance, this);
		detail->Build();
	}

}

void QDetailWidgetPropertyInstanceItem::ReflushInstanceComboBox()
{
	if (mInstanceComboBox) {
		mInstanceComboBox->disconnect();
		mInstanceComboBox->clear();
		QStringList SubTypeList = GetHandler()->GetMetaData("SubTypeList").toStringList();
		for (int i = 0; i < SubTypeList.size(); i++) {
			if (QMetaType::fromName(SubTypeList[i].toLocal8Bit()).isValid()) {
				mInstanceComboBox->addItem(SubTypeList[i]);
			}
			else {
				qWarning() << "An invalid type:" << SubTypeList[i];
			}
		}
		mInstanceComboBox->setCurrentText(GetValue().metaType().name());
		connect(mInstanceComboBox, &QComboBox::currentTextChanged, this, [this](const QString& inText) {
			QMetaType type = QMetaType::fromName(mInstanceComboBox->currentText().toLocal8Bit());
			QVariant Var;
			if (type.isValid()) {
				Var = QPropertyHandler::CreateNewVariant(GetHandler()->GetType(), type);
			}
			SetValue(Var, GetHandler()->GetPath() + "Set Instance :" + mInstanceComboBox->currentText());
			RecreateChildren();
			});
		connect(GetHandler(), &QPropertyHandler::AsValueChanged, mInstanceComboBox, [this]() {
			mInstanceComboBox->setCurrentText(GetValue().metaType().name());
			});
	}
}

QWidget* QDetailWidgetPropertyInstanceItem::GenerateValueWidget() {

	QStringList SubTypeList = GetHandler()->GetMetaData("SubTypeList").toStringList();
	if (!SubTypeList.isEmpty()) {
		if (!mInstanceComboBox) {
			mInstanceComboBox = new QComboBox();
		}
		return mInstanceComboBox;
	}
	return nullptr;
}

void QDetailWidgetPropertyInstanceItem::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	RecreateChildren();
}