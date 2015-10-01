#pragma once

#include <stdint.h>

// Standard int typedefs
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;
typedef wchar_t wchar;
typedef uint32_t bool32;

//#if defined(_DEBUG) 
//#ifndef DBG_NEW      
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )  
//#define new DBG_NEW   
//#endif
//#endif  // _DEBUG