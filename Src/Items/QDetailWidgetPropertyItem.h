#ifndef QDetailWidgetPropertyItem_h__
#define QDetailWidgetPropertyItem_h__

#include "QDetailWidgetItem.h"
#include "QBoxLayout"
#include "QSplitter"
#include "QJsonObject"

class QDetailWidgetPropertyItem;
class QPushButton;

class QDetailWidgetPropertyItemWidget : public QSplitter {
	friend class QDetailWidgetRow;
private:
	QWidget* mNameContent = nullptr;
	QHBoxLayout* mNameContentLayout = nullptr;
	QWidget* mNameWidget = nullptr;

	QWidget* mValueContent = nullptr;
	QHBoxLayout* mValueContentLayout = nullptr;
	QWidget* mValueWidget = nullptr;

	QDetailWidgetPropertyItem* mRow = nullptr;
	QPushButton* mResetButton = nullptr;

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
public:
	QDetailWidgetPropertyItemWidget(QDetailWidgetPropertyItem* inRow);

	void RefleshSplitterFactor();

	void SetNameWidget(QWidget* inWidget);
	void SetNameWidgetByText(QString inName);
	void SetValueWidget(QWidget* inWidget);
	QHBoxLayout* GetNameContentLayout() const;
	QHBoxLayout* GetValueContentLayout() const;

};

template<typename Aux_type>
void GetIDListFromTypeInternal(QList<int>& inIDList) {}

template<typename Aux_type, typename Head, typename... Tail>
void GetIDListFromTypeInternal(QList<int>& inIDList) {
	if (QMetaTypeId2<Head>::Defined) {
		inIDList << QMetaTypeId2<Head>::qt_metatype_id();
	}
	GetIDListFromTypeInternal<Aux_type, Tail...>(inIDList);
}

template<typename... Args>
QList<int> GetIDListFromType() {
	QList<int> idList;
	GetIDListFromTypeInternal<void, Args...>(idList);
	return idList;
}

#define Q_DETAIL_SUPPORTED_TYPES(...)	\
	inline static QList<int> SupportedTypes() {	\
		return GetIDListFromType<__VA_ARGS__>(); \
	}


class QDetailWidgetPropertyItem : public QObject, public QDetailWidgetItem {
public:
	using Getter = std::function<QVariant()>;
	using Setter = std::function<void(QVariant)>;
	using TypeId = int;

	QDetailWidgetPropertyItem();

	void Initialize(TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter,QJsonObject inMetaData = QJsonObject());

	void SetValue(QVariant inValue);

	QVariant GetValue();

	void ResetValue();

	TypeId GetTypeID();

	template<typename Type>
	bool canConvert() {
		return QMetaType::canConvert(QMetaType(GetTypeID()), QMetaType::fromType<Type>());
	}

	QString GetName();

	virtual QString GetKeywords() override;

	virtual void BuildContentAndChildren() override;

	virtual void OnCreateMenu(QMenu& inMenu) override {}

	virtual QWidget* GenerateValueWidget() = 0;

	QDetailWidgetPropertyItemWidget* GetContent() const;

	void RefleshSplitterFactor();

	const QJsonObject& GetMetaData() const;
private:
	TypeId mTypeID = 0;
	QString mName;
	Getter mGetter;
	Setter mSetter;
	QVariant mInitialValue;
	bool mIsChanged = false;
	QDetailWidgetPropertyItemWidget* mContent = nullptr;
	QJsonObject mMetaData;
};

#endif // QDetailWidgetPropertyItem_h__


