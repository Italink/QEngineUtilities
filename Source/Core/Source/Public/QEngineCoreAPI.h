#ifndef QENGINECORE_API_H
#define QENGINECORE_API_H

#include <QtCore/qglobal.h>

#if defined(QENGINECORE_LIBRARY)
#  define QENGINECORE_API Q_DECL_EXPORT
#else
#  define QENGINECORE_API Q_DECL_IMPORT
#endif

#endif // QENGINECORE_API_H