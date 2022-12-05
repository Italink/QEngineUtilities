#ifndef QDetailWidget_h__
#define QDetailWidget_h__

#include "QWidget"
#include "Core/QInstance.h"

class QDetailSearcher;
class QDetailTreeWidget;
class QUndoView;
class QInstanceTreeWidget;

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

enum class QDetailWidgetFlag {
	None = 0x0,
	DisplaySearcher = 0x1,
	DisplayObjectTree = 0x2,
	HideTopLevelObject = 0x4
};

Q_DECLARE_FLAGS(QDetailWidgetFlags, QDetailWidgetFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(QDetailWidgetFlags)

class QDetailWidget :public QWidget {
	Q_OBJECT
public:
	using QWidget = QWidget;

	QDetailWidget(QDetailWidgetFlags inFlags = QDetailWidgetFlag::DisplaySearcher);

	template<typename... Instances>
	void SetInstances(Instances... inInstances) {
		QList<QSharedPointer<QInstance>> InstanceList;
		GenerateGadgetOrObject(InstanceList, inInstances...);
		SetInstanceList(InstanceList);
	}

	void SetInstanceList(const QList<QSharedPointer<QInstance>>& inInstances);

	void SearchByKeywords(QString inKeywords);
private:
	QInstanceTreeWidget* mInstanceTree = nullptr;
	QDetailSearcher* mSearcher = nullptr;
	QDetailTreeWidget* mTreeWidget = nullptr;
	QDetailWidgetFlags mFlags;
};



#endif // QDetailWidget_h__
