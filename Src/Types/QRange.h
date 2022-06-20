#ifndef QRange_h__
#define QRange_h__

#include <QMetaType>
#include "QDebug"

class QRange {
public:
	QRange() {};
	QRange(int lower, int upper, int min, int max);

	void setLimite(int min, int max);
	int min() const;
	void setMin(int min);
	int max() const;
	void setMax(int max);

	int getLower() const;
	void setLower(int val);
	int getUpper() const;
	void setUpper(int val);
	void setRange(int lower, int upper);

	bool operator==(const QRange& other);

	friend QDataStream& operator<<(QDataStream& out, const QRange& var);
	friend QDataStream& operator>>(QDataStream& in, QRange& var);
	friend QDebug operator<<(QDebug debug, const QRange& range);
private:
	int lower_ = 0;
	int upper_ = 100;
	int mMin = 0, mMax = 100;
};

Q_DECLARE_METATYPE(QRange)

#endif // QRange_h__
