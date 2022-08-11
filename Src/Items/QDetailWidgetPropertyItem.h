#ifndef QDetailWidgetPropertyItem_h__
#define QDetailWidgetPropertyItem_h__

#include "QDetailWidgetItem.h"
#include "QBoxLayout"
#include "QSplitter"
#include "QJsonObject"
#include "QPropertyHandler.h"
#include "QMenu"

class QDetailWidgetPropertyItem;
class QPushButton;

class QDetailWidgetPropertyItemWidget : public QSplitter {
	Q_OBJECT
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
	Q_SIGNAL void AsRequsetReset();
	QDetailWidgetPropertyItemWidget(QDetailWidgetPropertyItem* inRow);

	void RefleshSplitterFactor();
	void SetNameWidget(QWidget* inWidget);
	void SetNameWidgetByText(QString inName);
	void SetValueWidget(QWidget* inWidget);
	QHBoxLayout* GetNameContentLayout() const;
	QHBoxLayout* GetValueContentLayout() const;
	QPushButton* GetResetButton() const { return mResetButton; }
	void ShowRenameEditor();
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
	Q_OBJECT
public:
	using TypeId = QPropertyHandler::TypeId;

	static QDetailWidgetPropertyItem* Create(QPropertyHandler* inHandler, QVariantHash inMetaData = QVariantHash());

	void SetValue(QVariant inValue);

	virtual void SetHandler(QPropertyHandler* inHandler);

	virtual void ResetValue();

	virtual QString GetKeywords() override;

	virtual void BuildContentAndChildren() override;

	virtual void BuildMenu(QMenu& inMenu) override;

	virtual QWidget* GenerateValueWidget() = 0;

	QDetailWidgetPropertyItemWidget* GetContent() const;

	void RefleshSplitterFactor();

	const QVariantHash& GetMetaData() const;
	QVariant GetMetaData(const QString& Key);

	template<typename Type>
	bool canConvert() {
		return QMetaType::canConvert(QMetaType(GetHandler()->GetTypeID()), QMetaType::fromType<Type>());
	}

	QVariant GetValue();

	QString GetName();

	QPropertyHandler* GetHandler() const { return mHandler; }

	void RequestRename();

	bool CanRename() const { return mRenameCallback ? true : false; }

	bool CanReorderChildren() const { return mCanReorderChildren; }

	void SetReorderChildrenEnabled(bool val) { mCanReorderChildren = val; }

	void SetRenameCallback(std::function<bool(QString)> val) { mRenameCallback = val; }

	std::function<bool(QString)> GetRenameCallback() const { return mRenameCallback; }
protected:
	QDetailWidgetPropertyItem();
	void RefleshResetButtonStatus();
private:
	QPropertyHandler* mHandler = nullptr;
	QDetailWidgetPropertyItemWidget* mContent = nullptr;
	QVariantHash mMetaData;
	bool mCanReorderChildren = false;
	std::function<bool(QString)> mRenameCallback;
};

#endif // QDetailWidgetPropertyItem_h__


