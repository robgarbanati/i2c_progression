// Add types that are used in the Nuvoton code to 
// help make code sharing easier.

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#if defined (__ARMCC_VERSION)
#include <stddef.h>
#endif

#define CONST				const

#define FALSE				0
#define TRUE				1

#ifndef __NRFTYPE_PVOID_DEFINED__
#define __NRFTYPE_PVOID_DEFINED__
typedef void *				PVOID;
#endif	// #ifndef __NRFTYPE_PVOID_DEFINED__

#ifndef __NRFTYPE_BOOL_DEFINED__
#define __NRFTYPE_BOOL_DEFINED__
typedef unsigned char		BOOL;
#endif	// #ifndef __NRFTYPE_BOOL_DEFINED__

#ifndef __NRFTYPE_PBOOL_DEFINED__
#define __NRFTYPE_PBOOL_DEFINED__
typedef unsigned char *		PBOOL;
#endif	// #ifndef __NRFTYPE_PBOOL_DEFINED__

#ifndef	__NRFTYPE_INT8__
#define __NRFTYPE_INT8__
typedef signed char			INT8;
#endif	// #ifndef __NRFTYPE_INT8_DEFINED__

#ifndef	__NRFTYPE_PINT8_DEFINED__
#define __NRFTYPE_PINT8_DEFINED__
typedef signed char *		PINT8;
#endif	// #ifndef __NRFTYPE_PINT8_DEFINED__

#ifndef	__NRFTYPE_UINT8_DEFINED__
#define __NRFTYPE_UINT8_DEFINED__
typedef unsigned char		UINT8;
#endif	// #ifndef __NRFTYPE_UINT8_DEFINED__

#ifndef	__NRFTYPE_PUINT8_DEFINED__
#define __NRFTYPE_PUINT8_DEFINED__
typedef unsigned char *		PUINT8;
#endif	// #ifndef __NRFTYPE_PUINT8_DEFINED__

#ifndef	__NRFTYPE_INT16_DEFINED__
#define __NRFTYPE_INT16_DEFINED__
typedef signed short		INT16;
#endif	// #ifndef __NRFTYPE_INT16_DEFINED__

#ifndef	__NRFTYPE_PINT16_DEFINED__
#define __NRFTYPE_PINT16_DEFINED__
typedef signed short *		PINT16;
#endif	// #ifndef __NRFTYPE_PINT16_DEFINED__

#ifndef	__NRFTYPE_UINT16_DEFINED__
#define __NRFTYPE_UINT16_DEFINED__
typedef unsigned short		UINT16;
#endif	// #ifndef __NRFTYPE_UINT16_DEFINED__

#ifndef	__NRFTYPE_PUINT16_DEFINED__
#define __NRFTYPE_PUINT16_DEFINED__
typedef unsigned short *	PUINT16;
#endif	// #ifndef __NRFTYPE_PUINT16_DEFINED__

#ifndef	__NRFTYPE_INT32_DEFINED__
#define __NRFTYPE_INT32_DEFINED__
typedef signed int			INT32;
#endif	// #ifndef __NRFTYPE_INT32_DEFINED__

#ifndef	__NRFTYPE_PINT32__
#define __NRFTYPE_PINT32__
typedef signed int *		PINT32;
#endif	// #ifndef __NRFTYPE_PINT32__

#ifndef	__NRFTYPE_UINT32_DEFINED__
#define __NRFTYPE_UINT32_DEFINED__
typedef unsigned int		UINT32;
#endif	// #ifndef __NRFTYPE_UINT32_DEFINED__

#ifndef	__NRFTYPE_PUINT32_DEFINED__
#define __NRFTYPE_PUINT32_DEFINED__
typedef unsigned int *		PUINT32;
#endif	// #ifndef __NRFTYPE_PUINT32_DEFINED__

#ifdef __GNUC__

#ifndef	__NRFTYPE_INT64_DEFINED__
#define __NRFTYPE_INT64_DEFINED__
typedef signed long long	INT64;
#endif	// #ifndef __NRFTYPE_INT64_DEFINED__

#ifndef	__NRFTYPE_PINT64_DEFINED__
#define __NRFTYPE_PINT64_DEFINED__
typedef signed long long *	PINT64;
#endif	// #ifndef __NRFTYPE_PINT64_DEFINED__

#ifndef	__NRFTYPE_UINT64_DEFINED__
#define __NRFTYPE_UINT64_DEFINED__
typedef unsigned long long	UINT64;
#endif	// #ifndef __NRFTYPE_UINT64_DEFINED__

#ifndef	__NRFTYPE_PUINT64_DEFINED__
#define __NRFTYPE_PUINT64_DEFINED__
typedef unsigned long long *PUINT64;
#endif	// #ifndef __NRFTYPE_PUINT64_DEFINED__

#elif defined (__ARMCC_VERSION)

#ifndef	__NRFTYPE_INT64_DEFINED__
#define __NRFTYPE_INT64_DEFINED__
typedef signed __int64		INT64;
#endif	// #ifndef __NRFTYPE_INT64_DEFINED__

#ifndef	__NRFTYPE_PINT64_DEFINED__
#define __NRFTYPE_PINT64_DEFINED__
typedef signed __int64 *	PINT64;
#endif	// #ifndef __NRFTYPE_PINT64_DEFINED__

#ifndef	__NRFTYPE_UINT64_DEFINED__
#define __NRFTYPE_UINT64_DEFINED__
typedef unsigned __int64	UINT64;
#endif	// #ifndef __NRFTYPE_UINT64_DEFINED__

#ifndef	__NRFTYPE_PUINT64_DEFINED__
#define __NRFTYPE_PUINT64_DEFINED__
typedef unsigned __int64	*PUINT64;
#endif	// #ifndef __NRFTYPE_PUINT64_DEFINED__

#endif	// __GNUC__

#ifndef	__NRFTYPE_FLOAT_DEFINED__
#define __NRFTYPE_FLOAT_DEFINED__
typedef float				FLOAT;
#endif	// #ifndef __NRFTYPE_FLOAT_DEFINED__

#ifndef	__NRFTYPE_PFLOAT_DEFINED__
#define __NRFTYPE_PFLOAT_DEFINED__
typedef float *				PFLOAT;
#endif	// #ifndef __NRFTYPE_PFLOAT_DEFINED__

#ifndef	__NRFTYPE_DOUBLE_DEFINED__
#define __NRFTYPE_DOUBLE_DEFINED__
typedef double				DOUBLE;
#endif	// #ifndef __NRFTYPE_DOUBLE_DEFINED__

#ifndef	__NRFTYPE_PDOUBLE_DEFINED__
#define __NRFTYPE_PDOUBLE_DEFINED__
typedef double *			PDOUBLE;
#endif	// #ifndef __NRFTYPE_PDOUBLE_DEFINED__

#ifndef	__NRFTYPE_CHAR_DEFINED__
#define __NRFTYPE_CHAR_DEFINED__
typedef signed char			CHAR;
#endif	// #ifndef __NRFTYPE_CHAR_DEFINED__

#ifndef	__NRFTYPE_PCHAR_DEFINED__
#define __NRFTYPE_PCHAR_DEFINED__
typedef signed char *		PCHAR;
#endif	// #ifndef __NRFTYPE_PCHAR_DEFINED__

#ifndef	__NRFTYPE_PSTR_DEFINED__
#define __NRFTYPE_PSTR_DEFINED__
typedef signed char *		PSTR;
#endif	// #ifndef __NRFTYPE_PSTR_DEFINED__

#ifndef	__NRFTYPE_PCSTR_DEFINED__
#define __NRFTYPE_PCSTR_DEFINED__
typedef const signed char *	PCSTR;
#endif	// #ifndef __NRFTYPE_PCSTR_DEFINED__

#ifdef __GNUC__
#ifndef	__NRFTYPE_WCHAR_DEFINED__
#define __NRFTYPE_WCHAR_DEFINED__
typedef	UINT16				WCHAR;
#endif	// #ifndef __NRFTYPE_WCHAR_DEFINED__

#ifndef	__NRFTYPE_PWCHAR_DEFINED__
#define __NRFTYPE_PWCHAR_DEFINED__
typedef	UINT16 *			PWCHAR;
#endif	// #ifndef __NRFTYPE_PWCHAR_DEFINED__

#ifndef	__NRFTYPE_PWSTR_DEFINED__
#define __NRFTYPE_PWSTR_DEFINED__
typedef	UINT16 *			PWSTR;
#endif	// #ifndef __NRFTYPE_PWSTR_DEFINED__

#ifndef	__NRFTYPE_PCWSTR_DEFINED__
#define __NRFTYPE_PCWSTR_DEFINED__
typedef	const UINT16 *		PCWSTR;
#endif	// #ifndef __NRFTYPE_PCWSTR_DEFINED__

#elif defined (__ARMCC_VERSION)
#ifndef	__NRFTYPE_WCHAR_DEFINED__
#define __NRFTYPE_WCHAR_DEFINED__
typedef	wchar_t				WCHAR;
#endif	// #ifndef __NRFTYPE_WCHAR_DEFINED__

#ifndef	__NRFTYPE_PWCHAR_DEFINED__
#define __NRFTYPE_PWCHAR_DEFINED__
typedef	wchar_t *			PWCHAR;
#endif	// #ifndef __NRFTYPE_PWCHAR_DEFINED__

#ifndef	__NRFTYPE_PWSTR_DEFINED__
#define __NRFTYPE_PWSTR_DEFINED__
typedef	wchar_t *			PWSTR;
#endif	// #ifndef __NRFTYPE_PWSTR_DEFINED__

#ifndef	__NRFTYPE_PCWSTR_DEFINED__
#define __NRFTYPE_PCWSTR_DEFINED__
typedef	const wchar_t *		PCWSTR;
#endif	// #ifndef __NRFTYPE_PCWSTR_DEFINED__

#endif	// __GNUC__

#ifndef	__NRFTYPE_SIZE_T_DEFINED__
#define __NRFTYPE_SIZE_T_DEFINED__
typedef UINT32				SIZE_T;
#endif	// #ifndef __NRFTYPE_SIZE_T_DEFINED__

#ifndef	__NRFTYPE_REG8_DEFINED__
#define __NRFTYPE_REG8_DEFINED__
typedef volatile UINT8		REG8;
#endif	// #ifndef __NRFTYPE_REG8_DEFINED__

#ifndef	__NRFTYPE_REG16_DEFINED__
#define __NRFTYPE_REG16_DEFINED__
typedef volatile UINT16		REG16;
#endif	// #ifndef __NRFTYPE_REG16_DEFINED__

#ifndef	__NRFTYPE_REG32_DEFINED__
#define __NRFTYPE_REG32_DEFINED__
typedef volatile UINT32		REG32;
#endif	// #ifndef __NRFTYPE_REG32_DEFINED__

#ifndef	__NRFTYPE_BYTE_DEFINED__
#define __NRFTYPE_BYTE_DEFINED__
typedef	unsigned char		BYTE;
#endif	// #ifndef __NRFTYPE_BYTE_DEFINED__

typedef UINT8				ERRCODE;

#define outp32(port,value)	(*((REG32 *)(port))=(value))
#define inp32(port)			(*((REG32 *)(port)))

#endif /* __NRFTYPES_H__ */

