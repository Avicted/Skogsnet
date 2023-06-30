#pragma once

// Standard library
#include <iostream>
#include <memory>
#include <ctime>
#include <cstring>
#include <time.h>
#include <string>
#include <cmath>
#include <signal.h>

// Reference additional headers your program requires here.
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// Program specific stuff --------------------------------------
#define local_persist static   // localy scoped persisted variable
#define global_variable static // globaly scoped variable in the same translation unit
#define internal static        // localy scoped function to the translation unit

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;
typedef intmax_t isize;

typedef float f32;
typedef double f64;

// Macros ------------------------
// Write straight to the null pointer to crash the program
#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(i32 *)0 = 0;     \
    }

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)