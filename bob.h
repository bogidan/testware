#pragma once

#include <stdint.h>
typedef  int8_t  s8, s08,  S08, *pS08;
typedef  int16_t     s16,  S16, *pS16;
typedef  int32_t     s32,  S32, *pS32;
typedef  int64_t     s64,  S64, *pS64;

typedef uint8_t  u8, u08,  U08, *pU08;
typedef uint16_t     u16,  U16, *pU16;
typedef uint32_t     u32,  U32, *pU32;
typedef uint64_t     u64,  U64, *pU64;

typedef float        f32,  F32, *pF32;
typedef double       f64,  F64, *pF64;

typedef   signed int       sI32, SI32, *pSI32;
typedef unsigned int       uI32, UI32, *pUI32;
typedef   signed long      sL32, SL32, *pSL32;
typedef unsigned long      uL32, UL32, *pUL32;

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

typedef        char   *str_t, cstr_t[MAX_PATH];
typedef const  char   *str_c;
typedef       wchar_t *wcs_t, wstr_t[MAX_PATH];
typedef const wchar_t *wcs_c;

#include <tchar.h>
typedef       TCHAR   *tcs_t, tstr_t[MAX_PATH];
typedef const TCHAR   *tcs_c;


#define lengthOf(a) (sizeof(a)/sizeof(*a))