#pragma once

#include <stdint.h>
typedef  void        nil;

typedef  int8_t  s8, s08,  S08, *pS08;
typedef  int16_t     s16,  S16, *pS16;
typedef  int32_t     s32,  S32, *pS32;
typedef  int64_t     s64,  S64, *pS64;

typedef uint8_t  u8, u08,  U08, *pU08, b08;
typedef uint16_t     u16,  U16, *pU16;
typedef uint32_t     u32,  U32, *pU32, b32;
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

// Utils
#define lengthOf(a) (sizeof(a)/sizeof(*a))
#define stRETURN(value) {static const auto st = value; return st;}

// Debug
typedef int (__cdecl *print_ft)(const char*, ...);
extern print_ft dprintf, eprintf;
//extern DWORD log_report( enumErrorWarning code, DWORD err, TCHAR* fn );
//extern DWORD log_error ( const char* file, int line, enumErrorWarning code, DWORD err, TCHAR* fn );

#define EVAL_STRING(a) #a
#define MAKE_STRING(a) EVAL_STRING(a)
#define _sLINE__       MAKE_STRING(__LINE__)
#define __TRACE__      "["__FUNCTION__":"_sLINE__"]"
#define _ln "\n"

#define print(...)   dprintf(__VA_ARGS__)
#define trace(...)   dprintf(__TRACE__ __VA_ARGS__)
#define  warn(...)   dprintf("Warning: "__VA_ARGS__)
#define error(...)   eprintf("Error: " __FILE__ "[" _sLINE__ "]\n\t" __VA_ARGS__)
// Search and replace to match '\n' on end of strings
// {DEFAULT\(.*}[^\\][^n]" -> \1\\n"

#define println(str,...)   dprintf(str _ln,__VA_ARGS__)
#define traceln(str,...)   dprintf(__TRACE__ str _ln,__VA_ARGS__)
#define  warnln(str,...)   dprintf("Warning: "str _ln __VA_ARGS__)

#if defined _WIN32 && defined _DEBUG
#define DBREAK __debugbreak()
#else
#define DBREAK
#endif

#ifndef ASSERT
#define ASSERT(a,...) if(!(a)) { DBREAK; error(#a _ln __VA_ARGS__); }
//#define ASSERT(a,...) (!a || (__debugbreak(),error(#a"\n"__VA_ARGS__))
#endif

#ifndef VERIFY
#define VERIFY(a) ASSERT(a)
#endif

#define VERIFY_EQ(v,a)            ASSERT((a)==v)
#define VERIFY_NEQ(v,a)           ASSERT((a)!=v)
#define VERIFY_SUCCESS(a)         ASSERT(SUCCEEDED(a))
#define BREAK                     DBREAK; error("BREAK" _ln);  break
#define RETURN                    DBREAK; error("RETURN" _ln); return
#define GOTO                      DBREAK; error("GOTO" _ln);   goto
#define DEFAULT(...)     default: DBREAK; error("DEFAULT: " __VA_ARGS__);
//#define DEFAULT_(c,r,fn) default: DBREAK; log_error(__FILE__,__LINE__,c,e,fn);
#define CHECK                     DBREAK
#define ONCE(action) { static bool once = true; if( once ) { once = false; action; } }

#ifdef _WIN32

#endif

