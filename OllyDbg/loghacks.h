#pragma once

static inline void oprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    static char dprintf_msg[66000];
    *dprintf_msg = 0;
    vsnprintf_s(dprintf_msg, sizeof(dprintf_msg), format, args);
    auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD w;
    WriteFile(hOut, dprintf_msg, strlen(dprintf_msg), &w, nullptr);
}

static inline void oputs(const char* text)
{
    oprintf("%s\n", text);
}

static inline void oprintf_args(_In_z_ _Printf_format_string_ const char* Format, va_list Args)
{
    char buffer[16384];
    vsnprintf_s(buffer, _TRUNCATE, Format, Args);
    oprintf("%s", buffer);
}

#define ulog(function, ...) \
    { \
        oprintf("%s(\n", function); /* log the function name */\
        logArgs(__VA_ARGS__); /* log the function arguments */\
        oprintf(") = UNIMPLEMENTED\n"); \
    }

static inline void processArg(char* x)
{
    oprintf("\"%s\"", x);
}

static inline void processArg(const char* x)
{
    oprintf("\"%s\"", x);
}

static inline void processArg(float x)
{
    oprintf("%f", x);
}

static inline void processArg(double x)
{
    oprintf("%f", x);
}

static inline void processArg(long x)
{
    oprintf("%d", x);
}

static inline void processArg(int x)
{
    oprintf("%d", x);
}

static inline void processArg(uchar* x)
{
    oprintf("%p", x);
}

static inline void processArg(uint x)
{
    oprintf("%p", x);
}

static inline void processArg(ulong x)
{
    oprintf("%p", x);
}

//no arguments base case
static inline void logArgs()
{
}

#define p(x) #x, x

//template takes one argument (Arg) and zero or more extra arguments (Args)
template<typename Arg, typename... Args>
static inline void logArgs(const char* a1_name, Arg a1, Args... args)
{
    oprintf("  %s: ", a1_name);
    processArg(a1); //call processArg on the current (first) argument
    oprintf("\n");
    logArgs(args...); //expand the additional arguments -> logArgs(a2, a3, a4) where args = { a2, a3, a4 }
}