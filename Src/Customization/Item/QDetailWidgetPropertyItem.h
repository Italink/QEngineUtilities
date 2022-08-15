#ifndef QDetailWidgetPropertyItem_h__
#define QDetailWidgetPropertyItem_h__

#include "QDetailWidgetItem.h"
#include "QBoxLayout"
#include "QSplitter"
#include "QJsonObject"
#include "Core/QPropertyHandler.h"
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
	void ClearValueAttachWidget();
	void AddValueWidget(QWidget* inWidget);
	QHBoxLayout* GetNameContentLayout() const;
	QHBoxLayout* GetValueContentLayout() const;
	QPushButton* GetResetButton() const { return mResetButton; }
	void ShowRenameEditor();
};

template<typename Aux_type>
void GetIDListFromTypeInternal(QList<QMetaType>& inIDList) {}

template<typename Aux_type, typename Head, typename... Tail>
void GetIDListFromTypeInternal(QList<QMetaType>& inIDList) {
	if (QMetaTypeId2<Head>::Defined) {
		inIDList << QMetaType::fromType<Head>();
	}
	GetIDListFromTypeInternal<Aux_type, Tail...>(inIDList);
}

template<typename... Args>
QList<QMetaType> GetIDListFromType() {
	QList<QMetaType> idList;
	GetIDListFromTypeInternal<void, Args...>(idList);
	return idList;
}

#define Q_DETAIL_SUPPORTED_TYPES(...)	\
	inline static QList<QMetaType> SupportedTypes() {	\
		return GetIDListFromType<__VA_ARGS__>(); \
	}

class QDetailWidgetPropertyItem : public QObject, public QDetailWidgetItem {
	Q_OBJECT
public:
	static QDetailWidgetPropertyItem* Create(QPropertyHandler* inHandler);

	void SetValue(QVariant inValue, QString isPushUndoStackWithDesc = QString());

	virtual void SetHandler(QPropertyHandler* inHandler);

	virtual void ResetValue();

	virtual QString GetKeywords() override;

	virtual void BuildContentAndChildren() override;
	void RebuildAttachWidget();

	virtual void BuildMenu(QMenu& inMenu) override;

	virtual ItemType Type() const override { return ItemType::Property; }

	virtual QWidget* GenerateValueWidget() = 0;

	void AddValueWidget(QWidget* inWigdet);
	void AddValueLayout(QLayout* inLayout);

	QDetailWidgetPropertyItemWidget* GetContent() const;

	void RefleshSplitterFactor();

	const QVariantHash& GetMetaData() const;
	QVariant GetMetaData(const QString& Key);

	template<typename Type>
	bool canConvert() {
		return QMetaType::canConvert(GetHandler()->GetType(), QMetaType::fromType<Type>());
	}

	QVariant GetValue();

	QString GetName();

	QPropertyHandler* GetHandler() const { return mHandler; }
	QObject* GetParentObject() const { return mHandler->parent(); }

	void RequestRename();

	bool CanRename() const { return mRenameCallback ? true : false; }

	void SetRenameCallback(std::function<bool(QString)> val) { mRenameCallback = val; }
	std::function<bool(QString)> GetRenameCallback() const { return mRenameCallback; }

	void SetBuildContentAndChildrenCallback(std::function<void()> val);
protected:
	QDetailWidgetPropertyItem();
	void RefleshResetButtonStatus();
private:
	QPropertyHandler* mHandler = nullptr;
	QWidget* mValueWidget = nullptr;
	QDetailWidgetPropertyItemWidget* mContent = nullptr;
	std::function<bool(QString)> mRenameCallback;
	std::function<void()> mBuildContentAndChildrenCallback;
};

#endif // QDetailWidgetPropertyItem_h__


