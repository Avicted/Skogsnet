#pragma once

// Standard C++ library headers
#include <iostream>  // For input/output stream operations
#include <memory>    // For smart pointers like std::unique_ptr and std::shared_ptr
#include <ctime>     // For time manipulation (C-style time handling)
#include <cstring>   // For C-style string manipulation (e.g., strcmp, strcpy)
#include <time.h>    // For time functions and structures (similar to <ctime>)
#include <string>    // For std::string and string manipulation
#include <cmath>     // For common mathematical functions like sqrt, sin, cos
#include <algorithm> // For standard algorithms like std::sort, std::find
#include <fstream>   // For file input/output stream operations
#include <random>    // For random number generation facilities
#include <climits>   // For limits of integral types (e.g., INT_MAX)
#include <sstream>   // For string stream classes (std::stringstream)
#include <chrono>    // For high-precision time utilities

// POSIX headers (Portable Operating System Interface)
#include <errno.h>    // For error handling (defines errno)
#include <termios.h>  // For terminal I/O interfaces
#include <unistd.h>   // For access to the POSIX operating system API (e.g., sleep, fork)
#include <fcntl.h>    // For file control options (e.g., open, O_RDONLY)
#include <math.h>     // For mathematical functions (C-style, similar to <cmath>)
#include <stdio.h>    // For standard input/output functions (C-style, e.g., printf, scanf)
#include <stdlib.h>   // For standard library functions like malloc, free, exit
#include <signal.h>   // For signal handling (e.g., SIGKILL, SIGINT)
#include <sys/time.h> // For time functions and structures (e.g., gettimeofday)

// Third party
#include "code/json.hpp"
using json = nlohmann::json;

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
        __builtin_trap();  \
    }

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)