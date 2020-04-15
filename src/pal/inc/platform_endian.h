/*
 *  endian.h
 *
 *  Copyright 2013 Michael Zillgith
 *
 *  This file is taken from libIEC61850.
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

#include "stack_config.h"

#ifndef PLATFORM_IS_BIGENDIAN
#ifdef __GNUC__
#ifdef __BYTE_ORDER__
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define PLATFORM_IS_BIGENDIAN 1
#endif

#else

/* older versions of GCC have __BYTE_ORDER__ not defined! */
#ifdef __PPC__
#define PLATFORM_IS_BIGENDIAN 1
#endif

#endif /* __BYTE_ORDER__ */
#endif /* __GNUC__ */
#endif

#if (PLATFORM_IS_BIGENDIAN == 1)
#  define ORDER_LITTLE_ENDIAN 0
#else
#  define ORDER_LITTLE_ENDIAN 1
#endif

#endif /* ENDIAN_H_ */
