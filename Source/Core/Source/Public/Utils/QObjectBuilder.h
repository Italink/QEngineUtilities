#ifndef QObjectBuilder_h__
#define QObjectBuilder_h__

#include "Utils/QMacroExpand.h"
#include <QVariant>

#define Q_BUILDER_BEGIN(QtClass) \
	struct __Builder{ \
		friend class QtClass; \
		using ClassType = QtClass; 

#define Q_BUILDER_ATTRIBUTE(Type,PropertyName) \
		__Builder& set##PropertyName(Type inVar){ \
			mObject->setProperty(#PropertyName,QVariant::fromValue<Type>(inVar)); \
			return *this; \
		}

#define Q_BUILDER_FUNCTION_BEGIN(FunName,...)\
		__Builder& FunName(__VA_ARGS__){ 
		
#define Q_BUILDER_OBJECT_PTR mObject

#define Q_BUILDER_FUNCTION_END() \
		return *this; \
	}
		
#define Q_BUILDER_END() \
		operator ClassType*(){ return mObject; } \
		private: \
			__Builder(const QString& inObjectName) { \
				mObject = new ClassType; \
				mObject->setObjectName(inObjectName); \
			} \
			ClassType* mObject = nullptr; \
	}; \
	public: \
		static __Builder Create(const QString& inName){ return __Builder(inName); } 

#endif // QObjectBuilder_h__
