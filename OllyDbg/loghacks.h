#pragma once

#include "oprintf.h"

#define ulog(function, ...) \
    { \
        oprintf("%s(\n", function); /* log the function name */\
        logArgs(__VA_ARGS__); /* log the function arguments */\
        oprintf(") = UNIMPLEMENTED\n"); \
    }

#define plog(function, ...) \
    { \
        oprintf("%s(\n", function); /* log the function name */\
        logArgs(__VA_ARGS__); /* log the function arguments */\
        oprintf(")\n"); \
    }

static inline void printArg(char* x)
{
    oprintf("\"%s\"", x);
}

static inline void printArg(const char* x)
{
    oprintf("\"%s\"", x);
}

static inline void printArg(float x)
{
    oprintf("%f", x);
}

static inline void printArg(double x)
{
    oprintf("%f", x);
}

static inline void printArg(long x)
{
    oprintf("%d", x);
}

static inline void printArg(int x)
{
    oprintf("%d", x);
}

static inline void printArg(uchar* x)
{
    oprintf("%p", x);
}

static inline void printArg(uint x)
{
    oprintf("%p", x);
}

static inline void printArg(ulong x)
{
    oprintf("%p", x);
}

static inline void printArg(t_asmmodel [])
{
    oprintf("TODO: t_asmmodel");
}

static inline void printArg(const void* x)
{
    oprintf("%p", x);
}

#include <functional>

template<typename Arg>
using CustomPrint = std::function<void(const char* name, const char* format, Arg value)>;

template<typename Arg>
struct ArgHack
{
    const char* name = nullptr;
    const char* format = nullptr;
    Arg value;
    CustomPrint<Arg> customPrint;

    ArgHack() { }

    ArgHack(const char* name, Arg value)
        : name(name), value(value) { }

    ArgHack(const char* name, const char* format, Arg value)
        : name(name), format(format), value(value) { }

    ArgHack(const char* name, Arg value, const CustomPrint<Arg> & customPrint)
        : name(name), value(value), customPrint(customPrint) { }

    void Print() const
    {
        if(customPrint)
        {
            customPrint(name, format, value);
            return;
        }

        if(name)
            oprintf("%s: ", name);
        if(format)
            oprintf(format, value);
        else
            printArg(value);
    }
};

template<typename Arg>
static inline void printArg(const ArgHack<Arg> & hack)
{
    hack.Print();
}

//no arguments base case
static inline void logArgs()
{
}

template<typename Arg>
ArgHack<Arg> argHack(const char* name, Arg value)
{
    return ArgHack<Arg>(name, value);
}

template<typename Arg>
ArgHack<Arg> argHack(const char* name, const char* format, Arg value)
{
    return ArgHack<Arg>(name, format, value);
}

template<typename Arg>
ArgHack<Arg> argHack(const char* name, Arg value, const CustomPrint<Arg> & customPrint)
{
    return ArgHack<Arg>(name, value, customPrint)
}

#define p(x) argHack(#x, x)

//template takes one argument (Arg) and zero or more extra arguments (Args)
template<typename Arg, typename... Args>
static inline void logArgs(Arg a1, Args... args)
{
    oprintf("  ");
    printArg(a1); //call printArg on the current (first) argument
    oprintf("\n");
    logArgs(args...); //expand the additional arguments -> logArgs(a2, a3, a4) where args = { a2, a3, a4 }
}