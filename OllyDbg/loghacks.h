#pragma once

#define dprintf _plugin_logprintf
#define dputs _plugin_logputs

void dprintf_args(_In_z_ _Printf_format_string_ const char* Format, va_list Args)
{
    char buffer[16384];
    vsnprintf_s(buffer, _TRUNCATE, Format, Args);
    dprintf("%s", buffer);
}

#define log(x, ...) dprintf("[" PLUGIN_NAME "] " x, __VA_ARGS__)
#define ulog(function, ...) \
    { \
        log("%s(\n", function); /* log the function name */\
        logArgs(__VA_ARGS__); /* log the function arguments */\
        log(") = UNIMPLEMENTED\n"); \
    }

static inline void processArg(ULONG_PTR arg)
{
    log("  0x%p\n", arg);
}

static inline void processArg(char* x)
{

}

static inline void processArg(const char* x)
{

}

static inline void processArg(float x)
{

}

static inline void processArg(double x)
{

}

static inline void processArg(long x)
{

}

static inline void processArg(int x)
{

}

static inline void processArg(uchar* x)
{

}

static inline void processArg(uint x)
{

}

//no arguments base case
static inline void logArgs()
{
}

//template takes one argument (Arg) and zero or more extra arguments (Args)
template<typename Arg, typename... Args>
static inline void logArgs(Arg a1, Args... args)
{
    processArg(a1); //call processArg on the current (first) argument
    logArgs(args...); //expand the additional arguments -> logArgs(a2, a3, a4) where args = { a2, a3, a4 }
}