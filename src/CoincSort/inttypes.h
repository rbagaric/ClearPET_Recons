///\file    inttypes.h
///\brief   Some integer type declarations
///\author  Axel Wagner
///
/// This header declares some integer types. Unfortunately, not all compilers
/// support the stdint header. This header works around that problem.
///
/// If the symbol HAS_STDINT is defined, the header just includes <stdint.h>.
/// Otherwise, the header tries to define the types with help of <limits.h>.
///
#ifndef inttypes_h_included
#define inttypes_h_included

#if HAS_STDINT
# include <stdint.h>
#else

#include <limits.h>

#if (UCHAR_MAX != 0xFF)
# error "Invalid size of char type"
#endif
typedef signed char int8_t;
typedef unsigned char uint8_t;

#if (USHRT_MAX != 0xFFFF)
# error "Invalid size of short type"
#endif
typedef short int16_t;
typedef unsigned short uint16_t;

#if (UINT_MAX != 0xFFFFFFFF)
# if (ULONG_MAX != 0xFFFFFFFFUL)
#  error "Invalid size of long type"
# else
typedef long int32_t;
typedef unsigned long uint32_t;
# endif
#else
typedef int int32_t;
typedef unsigned int uint32_t;
#endif

typedef long long int64_t;
typedef unsigned long long uint64_t;

#endif

#endif // inttypes_h_included
