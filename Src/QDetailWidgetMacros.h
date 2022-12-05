#ifndef QDetailWidgetMacros_h__
#define QDetailWidgetMacros_h__

#include "Core/QMetaDataDefine.h"

#define Q_PROPERTY_VAR(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
		qDebug()<<"Set "<<#Name<<": "<<var; \
    } \
    Type Name

#define Q_PROPERTY_DEC(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
    } 


#endif // QDetailWidgetMacros_h__
