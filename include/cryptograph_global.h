#ifndef CRYPTOGRAPH_GLOBAL_H
#define CRYPTOGRAPH_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CRYPTOGRAPH_LIBRARY)
#  define CRYPTOGRAPHSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CRYPTOGRAPHSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <stdint.h>

typedef unsigned char byte;
typedef unsigned int word;
typedef uint64_t dword;

#define BYTE1(w) (((w) >> 24) & 0xff)
#define BYTE2(w) (((w) >> 16) & 0xff)
#define BYTE3(w) (((w) >> 8) & 0xff)
#define BYTE4(w) ((w) & 0xff)

#endif // CRYPTOGRAPH_GLOBAL_H
