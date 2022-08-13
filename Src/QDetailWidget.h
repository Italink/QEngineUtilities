#ifndef QDetailWidget_h__
#define QDetailWidget_h__

#include "QWidget"
#include "Core/QInstance.h"

class QDetailSearcher;
class QDetailTreeWidget;
class QUndoView;

template<typename Instance>
typename std::enable_if<QtPrivate::IsPointerToGadgetHelper<Instance>::IsGadgetOrDerivedFrom>::type
 GenerateGadgetOrObject(QList<QSharedPointer<QInstance>>& inInstanceList, Instance inInstance) {
	inInstanceList << QInstance::CreateGadget(inInstance,  &std::remove_pointer<Instance>::type::staticMetaObject);
}

template<typename Instance>
typename std::enable_if<QtPrivate::IsPointerToTypeDerivedFromQObject<Instance>::Value>::type
GenerateGadgetOrObject(QList<QSharedPointer<QInstance>>& inInstanceList, Instance inInstance) {
	inInstanceList << QInstance::CreateObjcet(inInstance);
}

template<typename... Instances>
void GenerateGadgetOrObject(QList<QSharedPointer<QInstance>>& inInstanceList, Instances... inInstance) {
	(..., GenerateGadgetOrObject(inInstanceList, inInstance));
}

class QDetailWidget :public QWidget {
	Q_OBJECT
public:
	using QWidget = QWidget;
	enum Flag {
		DisplaySearcher = 0x1,
		DisplayCategory = 0x2,
	};
	Q_DECLARE_FLAGS(Flags, Flag);

	enum Style {
		Unreal = 0,
		Qt,
	};

	QDetailWidget(QDetailWidget::Flags inFlags = Flags(QDetailWidget::DisplaySearcher | QDetailWidget::DisplayCategory), QDetailWidget::Style inStyle = QDetailWidget::Qt);

	template<typename... Instances>
	void SetInstances(Instances... inInstances) {
		QList<QSharedPointer<QInstance>> InstanceList;
		GenerateGadgetOrObject(InstanceList, inInstances...);
		SetInstanceList(InstanceList);
	}

	void SetInstanceList(const QList<QSharedPointer<QInstance>>& inInstances);

	void SetStyle(QDetailWidget::Style inStyle);
	void SearchByKeywords(QString inKeywords);
private:
	QDetailSearcher* mSearcher = nullptr;
	QDetailTreeWidget* mTreeWidget = nullptr;
	Flags mFlags;
};

#endif // QDetailWidget_h__
