#include "x64dbg.h"
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation of certain functions
#include "OllyDbg.h"
#include "loghacks.h"

int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;

struct OllyPlugin
{
    HINSTANCE hInst;
    char shortname[32];

    p_ODBG_Plugindata ODBG_Plugindata = 0; //mandatory
    p_ODBG_Plugininit ODBG_Plugininit = 0; //mandatory
    p_ODBG_Pluginmainloop ODBG_Pluginmainloop = 0;
    p_ODBG_Pluginsaveudd ODBG_Pluginsaveudd = 0;
    p_ODBG_Pluginuddrecord ODBG_Pluginuddrecord = 0;
    p_ODBG_Pluginmenu ODBG_Pluginmenu = 0;
    p_ODBG_Pluginaction ODBG_Pluginaction = 0;
    p_ODBG_Pluginshortcut ODBG_Pluginshortcut = 0;
    p_ODBG_Pluginreset ODBG_Pluginreset = 0;
    p_ODBG_Pluginclose ODBG_Pluginclose = 0;
    p_ODBG_Plugindestroy ODBG_Plugindestroy = 0;
    p_ODBG_Paused ODBG_Paused = 0;
    p_ODBG_Pausedex ODBG_Pausedex = 0;
    p_ODBG_Plugincmd ODBG_Plugincmd = 0;

    bool Load(const wchar_t* szFileName)
    {
        hInst = LoadLibraryW(szFileName);
        if(!hInst)
            return false;

        ODBG_Plugindata = p_ODBG_Plugindata(GetProcAddress(hInst, "_ODBG_Plugindata"));
        ODBG_Plugininit = p_ODBG_Plugininit(GetProcAddress(hInst, "_ODBG_Plugininit"));
        ODBG_Pluginmainloop = p_ODBG_Pluginmainloop(GetProcAddress(hInst, "_ODBG_Pluginmainloop"));
        ODBG_Pluginsaveudd = p_ODBG_Pluginsaveudd(GetProcAddress(hInst, "_ODBG_Pluginsaveudd"));
        ODBG_Pluginuddrecord = p_ODBG_Pluginuddrecord(GetProcAddress(hInst, "_ODBG_Pluginuddrecord"));
        ODBG_Pluginmenu = p_ODBG_Pluginmenu(GetProcAddress(hInst, "_ODBG_Pluginmenu"));
        ODBG_Pluginaction = p_ODBG_Pluginaction(GetProcAddress(hInst, "_ODBG_Pluginaction"));
        ODBG_Pluginshortcut = p_ODBG_Pluginshortcut(GetProcAddress(hInst, "_ODBG_Pluginshortcut"));
        ODBG_Pluginreset = p_ODBG_Pluginreset(GetProcAddress(hInst, "_ODBG_Pluginreset"));
        ODBG_Pluginclose = p_ODBG_Pluginclose(GetProcAddress(hInst, "_ODBG_Pluginclose"));
        ODBG_Plugindestroy = p_ODBG_Plugindestroy(GetProcAddress(hInst, "_ODBG_Plugindestroy"));
        ODBG_Paused = p_ODBG_Paused(GetProcAddress(hInst, "_ODBG_Paused"));
        ODBG_Pausedex = p_ODBG_Pausedex(GetProcAddress(hInst, "_ODBG_Pausedex"));
        ODBG_Plugincmd = p_ODBG_Plugincmd(GetProcAddress(hInst, "_ODBG_Plugincmd"));

        if(!ODBG_Plugindata || !ODBG_Plugininit)
        {
            dprintf("Exports _ODBG_Plugindata and _ODBG_Plugininit are mandatory!\n");
            FreeLibrary(hInst);
            return false;
        }

        auto version = ODBG_Plugindata(shortname);
        if(version < 106 || version > 110)
        {
            dprintf("_ODBG_Plugindata returned incompatible version: %d\n", version);
            FreeLibrary(hInst);
            return false;
        }

        if(ODBG_Plugininit(ODBG_PLUGIN_VERSION, GuiGetWindowHandle(), nullptr) != 0)
        {
            dprintf("_ODBG_Plugininit failed...\n");
            FreeLibrary(hInst);
            return false;
        }

        return true;
    }
};

static std::vector<OllyPlugin> ollyPlugins;

static void LoadPlugins()
{
    WIN32_FIND_DATAW fd;
    auto hSearch = FindFirstFileW(L"*.dll", &fd);
    if(hSearch == INVALID_HANDLE_VALUE)
        return;
    do
    {
        OllyPlugin plugin;
        if(plugin.Load(fd.cFileName))
        {
            ollyPlugins.push_back(plugin);
            dprintf("%S => %s\n", fd.cFileName, plugin.shortname);
        }
    }
    while(FindNextFileW(hSearch, &fd));
    CloseHandle(hSearch);
}

static void loadConsole()
{
    AllocConsole();
    SetConsoleTitleA(PLUGIN_NAME);

    duint x, y;
    if(BridgeSettingGetUint("OllyDbg", "ConsoleX", &x) && BridgeSettingGetUint("OllyDbg", "ConsoleY", &y))
    {
        auto zhwnd = HWND_TOPMOST;
        duint setting;
        if(BridgeSettingGetUint("OllyDbg", "NoConsoleTopmost", &setting) && setting)
            zhwnd = HWND_TOP;
        SetWindowPos(GetConsoleWindow(), zhwnd, int(x), int(y), 0, 0, SWP_NOSIZE);
        SwitchToThisWindow(GuiGetWindowHandle(), FALSE);
    }
}

static void saveConsole()
{
    RECT pos;
    GetWindowRect(GetConsoleWindow(), &pos);
    BridgeSettingSetUint("OllyDbg", "ConsoleX", pos.left);
    BridgeSettingSetUint("OllyDbg", "ConsoleY", pos.top);
}

PLUG_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    loadConsole();

    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, PLUGIN_NAME, _TRUNCATE);
    pluginHandle = initStruct->pluginHandle;

    LoadPlugins();
    return true;
}

PLUG_EXPORT bool plugstop()
{
    saveConsole();
    return true;
}

PLUG_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;
}