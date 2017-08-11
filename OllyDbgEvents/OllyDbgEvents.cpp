#include <windows.h>
#include "../OllyDbg/oprintf.h"
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation of certain functions
#include "../OllyDbg/OllyDbg.h"

#pragma comment(lib, "OllyDbg.lib")

#define EXPORT extern "C" __declspec(dllexport)

static bool isRealOlly = false;

EXPORT int _ODBG_Plugindata(char* shortname)
{
    AllocConsole();
    oprintf("%s()\n", __FUNCTION__);

    strcpy_s(shortname, 32, "OllyDbgEvents");
    return 110;
}

EXPORT int _ODBG_Plugininit(int ollydbgversion, HWND hw, ulong* features)
{
    oprintf("%s(%d, %X, %p)\n", __FUNCTION__, ollydbgversion, hw, features);

    //real fix goes here
    __try
    {
        DWORD old;
        auto addr = (void*)0x4311C2;
        if(*(int*)addr == 0x8F831174)
        {
            oputs("OllyWow64 v0.2 by waleedassar");
            VirtualProtect(addr, 0x1, PAGE_EXECUTE_READWRITE, &old);
            *(unsigned char*)addr = 0xEB;
            VirtualProtect(addr, 0x1, old, &old);
            isRealOlly = true;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return 0;
}

EXPORT void _ODBG_Pluginmainloop(DEBUG_EVENT* debugevent)
{
    if(!debugevent)
    {
        static auto previousMillis = 0;
        auto currentMillis = GetTickCount();
        if(currentMillis - previousMillis >= 10000)
            previousMillis = currentMillis;
        else
            return;
    }
    oprintf("%s(%p)\n", __FUNCTION__, debugevent);
}

#define UDD_TAG 1337

EXPORT void _ODBG_Pluginsaveudd(t_module* pmod, int ismainmodule)
{
    oprintf("%s(\"%s\", %d)\n", __FUNCTION__, pmod->path, ismainmodule);
    DWORD ms = GetTickCount();
    oprintf("Pluginsaverecord(%u): %d\n", ms, Pluginsaverecord(UDD_TAG, sizeof(ms), &ms));
}

EXPORT int _ODBG_Pluginuddrecord(t_module* pmod, int ismainmodule, ulong tag, ulong size, void* data)
{
    oprintf("%s(%p, %d, %X, %d, %p)\n", __FUNCTION__, pmod, ismainmodule, tag, size, data);
    if(tag == UDD_TAG)
    {
        DWORD ms;
        if(size == sizeof(ms))
        {
            memcpy(&ms, data, sizeof(ms));
            oprintf("data: %u\n", ms);
        }
        else
            oprintf("INVALID UDD DATA!");
        return 1;
    }
    return 0;
}

EXPORT int _ODBG_Pluginmenu(int origin, char data[4096], void* item)
{
    oprintf("%s(%d, %p, %p)\n", __FUNCTION__, origin, data, item);

    strcpy_s(data, 4096, "#A{0Aaa,B{1Bbb|2Ccc}}");
    return 1;
}

EXPORT void _ODBG_Pluginaction(int origin, int action, void* item)
{
    oprintf("%s(%d, %d, %p)\n", __FUNCTION__, origin, action, item);
}

EXPORT int _ODBG_Pluginshortcut(int origin, int ctrl, int alt, int shift, int key, void* item)
{
    oprintf("%s(%d, %d, %d, %d, %d, %p)\n", __FUNCTION__, origin, ctrl, alt, shift, key, item);
    return 0;
}

EXPORT void _ODBG_Pluginreset()
{
    oprintf("%s()\n", __FUNCTION__);
}

EXPORT int _ODBG_Pluginclose()
{
    oprintf("%s()\n", __FUNCTION__);
    return 0;
}

EXPORT void _ODBG_Plugindestroy()
{
    oprintf("%s()\n", __FUNCTION__);
    if(isRealOlly)
        system("pause");
}

EXPORT int _ODBG_Paused(int reason, t_reg* reg)
{
    oprintf("%s(%d, %p)\n", __FUNCTION__, reason, reg);
    return 0;
}

EXPORT int _ODBG_Pausedex(int reason, int extdata, t_reg* reg, DEBUG_EVENT* debugevent)
{
    oprintf("%s(%d, %d, %p, %p)\n", __FUNCTION__, reason, extdata, reg, debugevent);
    return 0;
}

EXPORT int _ODBG_Plugincmd(int reason, t_reg* reg, char* cmd)
{
    oprintf("%s(%d, %p, \"%s\")\n", __FUNCTION__, reason, reg, cmd);
    return 0;
}