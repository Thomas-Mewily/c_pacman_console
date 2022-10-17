#ifndef _macro_h_
#define _macro_h_
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <conio.h>
#include <float.h>
#include <wchar.h>

// keyword
#define null NULL

#define true  1
#define false 0
#define bool int

#define sbyte int8_t
#define short int16_t
#define int   int32_t
#define long  int64_t
#define nint  intptr_t

#define byte   uint8_t
#define ushort uint16_t
#define uint   uint32_t
#define ulong  uint64_t
#define nuint  uintptr_t

#define int8   int8_t
#define int16  int16_t
#define int32  int32_t
#define int64  int64_t
#define intptr intptr_t

#define uint8   uint8_t
#define uint16  uint16_t
#define uint32  uint32_t
#define uint64  uint64_t
#define uintptr uintptr_t

#define index  nint
//#define size   index

#define file FILE

#define float32 float
#define float64 double

#define str_utf8 char*
#define wchar  wchar_t
// represent any unicode character code point
#define codepoint int32

#define until(cond) while(!(cond))
#define forever while(true)
#define repeat(_varName, _nbTime) for(index _varName = 0; _varName < (_nbTime); _varName++)
#define repeat_reverse(_varName, _nbTime) for(int _varName = (_nbTime)-1; _varName >= 0; _varName--)
#define for_plus_and_minus(_varName) for(index _varName = -1; _varName <= 1; _varName+= 2)

#define _current_file __FILE__
#define _current_line __LINE__
#define _current_func __func__

#define _current_time __TIME__
#define _current_date __DATE__

#define header_struct(name) struct struct_ ## name; typedef struct struct_ ## name name;

#define DEBUG_TRACE_WITH_ARG(file, func, line) printf("function: %s, file: %s:%i", func, file, line)
#define DEBUG_TRACE() DEBUG_TRACE_WITH_ARG(_current_file, _current_line, _current_line)
#define EXIT_ASSERT_FAILED EXIT_FAILURE

#define assert(ifFalseCondition, str_utf8_error) \
if(!(ifFalseCondition))\
{\
    printf("assert was false : %s\n", str_utf8_error);\
    printf("at ");\
    DEBUG_TRACE_WITH_ARG(_current_file, _current_func, _current_line);\
    printf("\n");\
    exit(EXIT_ASSERT_FAILED);\
}

#define smalloc  malloc
#define scalloc  calloc
#define srealloc realloc
#define sfree    free
#define smemcpy memcpy
#define smemset memset

#define new_array(type, quantity) smalloc(quantity*sizeof(type))
#define new(type) smalloc(sizeof(type))

#define ANSI_COLOR_BLACK      "\e[30m"
#define ANSI_COLOR_RED        "\e[31m"
#define ANSI_COLOR_GREEN      "\e[32m"
#define ANSI_COLOR_YELLOW     "\e[33m"
#define ANSI_COLOR_BLUE       "\e[34m"
#define ANSI_COLOR_MAGENTA    "\e[35m"
#define ANSI_COLOR_CYAN       "\e[36m"
#define ANSI_COLOR_WHITE      "\e[37m"

#define ANSI_COLOR_BLACK_BG   "\e[40m"
#define ANSI_COLOR_RED_BG     "\e[41m"
#define ANSI_COLOR_GREEN_BG   "\e[42m"
#define ANSI_COLOR_YELLOW_BG  "\e[43m"
#define ANSI_COLOR_BLUE_BG    "\e[44m"
#define ANSI_COLOR_MAGENTA_BG "\e[45m"
#define ANSI_COLOR_WHITE_BG   "\e[47m"

#define ANSI_COLOR_RESET   "\e[0;37;40m"


#endif