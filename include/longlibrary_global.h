#ifndef LONGLIBRARY_GLOBAL_H
#define LONGLIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LONGLIBRARY_LIBRARY)
#  define LONGLIBRARYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LONGLIBRARYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LONGLIBRARY_GLOBAL_H