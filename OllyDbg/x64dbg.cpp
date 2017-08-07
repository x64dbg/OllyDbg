#include "x64dbg.h"
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation of certain functions
#include "OllyDbg.h"
#include "loghacks.h"
#include "stringutils.h"
#include <unordered_map>

int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;

static HINSTANCE hInstMain;
static int hEntryPool = 100;
struct OllyPlugin;
static std::unordered_map<int, std::tuple<OllyPlugin*, int, int>> menuActionMap; //x64dbg hMenuEntry -> plugin + action + origin

static void eventReg(CBTYPE cbType)
{
    static bool registeredEvents[CB_LAST];
    if(registeredEvents[cbType])
        return;
    auto exportName = [cbType]()
    {
        switch(cbType)
        {
        case CB_INITDEBUG:
            return "CBINITDEBUG";
        case CB_STOPDEBUG:
            return "CBSTOPDEBUG";
        case CB_CREATEPROCESS:
            return "CBCREATEPROCESS";
        case CB_EXITPROCESS:
            return "CBEXITPROCESS";
        case CB_CREATETHREAD:
            return "CBCREATETHREAD";
        case CB_EXITTHREAD:
            return "CBEXITTHREAD";
        case CB_SYSTEMBREAKPOINT:
            return "CBSYSTEMBREAKPOINT";
        case CB_LOADDLL:
            return "CBLOADDLL";
        case CB_UNLOADDLL:
            return "CBUNLOADDLL";
        case CB_OUTPUTDEBUGSTRING:
            return "CBOUTPUTDEBUGSTRING";
        case CB_EXCEPTION:
            return "CBEXCEPTION";
        case CB_BREAKPOINT:
            return "CBBREAKPOINT";
        case CB_PAUSEDEBUG:
            return "CBPAUSEDEBUG";
        case CB_RESUMEDEBUG:
            return "CBRESUMEDEBUG";
        case CB_STEPPED:
            return "CBSTEPPED";
        case CB_ATTACH:
            return "CBATTACH";
        case CB_DETACH:
            return "CBDETACH";
        case CB_DEBUGEVENT:
            return "CBDEBUGEVENT";
        case CB_MENUENTRY:
            return "CBMENUENTRY";
        case CB_WINEVENT:
            return "CBWINEVENT";
        case CB_WINEVENTGLOBAL:
            return "CBWINEVENTGLOBAL";
        case CB_LOADDB:
            return "CBLOADDB";
        case CB_SAVEDB:
            return "CBSAVEDB";
        case CB_FILTERSYMBOL:
            return "CBFILTERSYMBOL";
        case CB_TRACEEXECUTE:
            return "CBTRACEEXECUTE";
        case CB_ANALYZE:
            return "CBANALYZE";
        case CB_ADDRINFO:
            return "CBADDRINFO";
        case CB_VALFROMSTRING:
            return "CBVALFROMSTRING";
        case CB_VALTOSTRING:
            return "CBVALTOSTRING";
        case CB_MENUPREPARE:
            return "CBMENUPREPARE";
        default:
            __debugbreak();
            return "";
        }
    }();
    auto cbPlugin = CBPLUGIN(GetProcAddress(hInstMain, exportName));
    if(!cbPlugin)
        __debugbreak();
    _plugin_registercallback(pluginHandle, cbType, cbPlugin);
    oprintf("eventReg => %s\n", exportName);
    registeredEvents[cbType] = true;
}

struct OllyPlugin
{
    HINSTANCE hInst = 0;
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

    char shortname[32];

    struct Menu
    {
        int hMenu;
        int origin;
        std::string data;
    };

    Menu menuDisasm;
    Menu menuDump;
    Menu menuStack;

    bool Load(const wchar_t* szFileName)
    {
        hInst = LoadLibraryW(szFileName);
        if(!hInst)
            return false;

        ODBG_Plugindata = p_ODBG_Plugindata(GetProcAddress(hInst, "_ODBG_Plugindata"));
        ODBG_Plugininit = p_ODBG_Plugininit(GetProcAddress(hInst, "_ODBG_Plugininit"));
        if(ODBG_Pluginmainloop = p_ODBG_Pluginmainloop(GetProcAddress(hInst, "_ODBG_Pluginmainloop")))
            eventReg(CB_DEBUGEVENT);
        ODBG_Pluginsaveudd = p_ODBG_Pluginsaveudd(GetProcAddress(hInst, "_ODBG_Pluginsaveudd"));
        ODBG_Pluginuddrecord = p_ODBG_Pluginuddrecord(GetProcAddress(hInst, "_ODBG_Pluginuddrecord"));
        if(ODBG_Pluginmenu = p_ODBG_Pluginmenu(GetProcAddress(hInst, "_ODBG_Pluginmenu")))
            eventReg(CB_MENUPREPARE);
        if(ODBG_Pluginaction = p_ODBG_Pluginaction(GetProcAddress(hInst, "_ODBG_Pluginaction")))
            eventReg(CB_MENUENTRY);
        ODBG_Pluginshortcut = p_ODBG_Pluginshortcut(GetProcAddress(hInst, "_ODBG_Pluginshortcut"));
        if(ODBG_Pluginreset = p_ODBG_Pluginreset(GetProcAddress(hInst, "_ODBG_Pluginreset")))
            eventReg(CB_INITDEBUG);
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

        if(ODBG_Plugininit(ODBG_PLUGIN_VERSION, GuiGetWindowHandle(), nullptr) == -1)
        {
            dprintf("_ODBG_Plugininit failed...\n");
            FreeLibrary(hInst);
            return false;
        }

        return true;
    }

    Menu* GetMenuOrigin(int hMenu)
    {
        switch(hMenu)
        {
        case GUI_DISASM_MENU:
            return &menuDisasm;
        case GUI_DUMP_MENU:
            return &menuDump;
        case GUI_STACK_MENU:
            return &menuStack;
        default:
            __debugbreak();
            return nullptr;
        }
    }

    void ParseMenu(const std::string & data, int hMenuRoot, int origin)
    {
        oprintf("ParseMenu (%s) => \"%s\"\n", shortname, data.c_str());

        std::string id;
        std::string name;
        std::vector<int> menuStack;
        menuStack.push_back(hMenuRoot);

        auto hMenu = [&]()
        {
            if(menuStack.empty())
                __debugbreak();
            return menuStack[menuStack.size() - 1];
        };

        enum State
        {
            None,
            Id,
            Name
        } state = None;

        auto addSubmenu = [&]()
        {
            id = StringUtils::Trim(id);
            name = StringUtils::Trim(name);

            if(id.empty() && name.empty())
                __debugbreak();
            id.append(name);

            auto hSubMenu = _plugin_menuadd(hMenu(), id.c_str());
            oprintf("added menu to %d: \"%s\" => %d\n", hMenu(), id.c_str(), hSubMenu);

            menuStack.push_back(hSubMenu);

            id.clear();
            name.clear();
            state = None;
        };

        auto addEntry = [&](char ch)
        {
            id = StringUtils::Trim(id);
            name = StringUtils::Trim(name);

            if(id.empty() || name.empty())
                __debugbreak();

            size_t idNum;
            if(!convertNumber(id.c_str(), idNum, 10))
                __debugbreak();
            if(idNum > 63)
                __debugbreak();

            auto hEntry = hEntryPool++;

            menuActionMap[hEntry] = std::make_tuple(this, int(idNum), origin);

            if(!_plugin_menuaddentry(hMenu(), hEntry, name.c_str()))
                __debugbreak();
            oprintf("added entry to %d: %d => %d, \"%s\"\n", hMenu(), hEntry, idNum, name.c_str());

            if(ch == '|')
            {
                if(!_plugin_menuaddseparator(hMenu()))
                    __debugbreak();
                else
                    oprintf("added separator (Name) to %d\n", ch, hMenu());
            }
            else if(ch == '}')
                menuStack.pop_back();

            id.clear();
            name.clear();
            state = None;
        };

        for(size_t i = 0; i < data.length(); i++)
        {
            auto ch = data[i];
            switch(state)
            {
            case None:
                if(ch == '#' || ch == '|')
                {
                    if(!_plugin_menuaddseparator(hMenu()))
                        __debugbreak();
                    else
                        oprintf("added separator (None) to %d\n", hMenu());
                }
                else if(id.empty() && name.empty() && (ch == ',' || ch == ' '))
                {
                }
                else if(isdigit(ch))
                {
                    id.push_back(ch);
                    state = Id;
                }
                else
                {
                    name.push_back(ch);
                    state = Name;
                }
                break;
            case Id:
                if(isdigit(ch))
                    id.push_back(ch);
                else if(ch == '{')
                    addSubmenu();
                else
                {
                    name.push_back(ch);
                    state = Name;
                }
                break;
            case Name:
                if(ch == ',' || ch == '|' || ch == '}')
                    addEntry(ch);
                else if(ch == '{')
                    addSubmenu();
                else
                    name.push_back(ch);
                break;
            default:
                __debugbreak();
            }
        }
        if(!id.empty() || !name.empty())
            addEntry('\0');

        oputs("ParseMenu finished!");
    }
};

static std::vector<OllyPlugin> ollyPlugins;

static void loadPlugins()
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

    loadPlugins();
    return true;
}

PLUG_EXPORT bool plugstop()
{
    for(auto & plugin : ollyPlugins)
        if(plugin.ODBG_Pluginclose)
            plugin.ODBG_Pluginclose(); //TODO: "Function must return 0 if it is safe to terminate OllyDbg. Any non-zero return will stop closing sequence."
    for(auto & plugin : ollyPlugins)
        if(plugin.ODBG_Plugindestroy)
            plugin.ODBG_Plugindestroy();
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

    char data[4096];
    for(auto & plugin : ollyPlugins)
    {
        *data = '\0';
        if(plugin.ODBG_Pluginmenu)
        {
            plugin.menuDisasm.hMenu = _plugin_menuadd(hMenuDisasm, plugin.shortname);
            plugin.menuDisasm.origin = PM_DISASM;
            plugin.menuDump.hMenu = _plugin_menuadd(hMenuDump, plugin.shortname);
            plugin.menuDump.origin = PM_CPUDUMP;
            plugin.menuStack.hMenu = _plugin_menuadd(hMenuStack, plugin.shortname);
            plugin.menuStack.origin = PM_CPUSTACK;

            if(plugin.ODBG_Pluginmenu(PM_MAIN, data, 0) == 1)
                plugin.ParseMenu(data, _plugin_menuadd(hMenu, plugin.shortname), PM_MAIN);
            else
            {
                menuActionMap[hEntryPool] = std::make_tuple(&plugin, 0, PM_MAIN);
                _plugin_menuaddentry(hMenu, hEntryPool++, plugin.shortname);
            }
        }
    }
}

PLUG_EXPORT void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
    if(info->hEntry < 100)
        __debugbreak(); //TODO: possible plugin management entries
    auto found = menuActionMap.find(info->hEntry);
    if(found == menuActionMap.end())
        __debugbreak();
    auto & plugin = *std::get<0>(found->second);
    if(plugin.ODBG_Pluginaction)
        plugin.ODBG_Pluginaction(std::get<2>(found->second), std::get<1>(found->second), 0);
}

PLUG_EXPORT void CBDEBUGEVENT(CBTYPE, PLUG_CB_DEBUGEVENT* info)
{
    for(auto & plugin : ollyPlugins)
        if(plugin.ODBG_Pluginmainloop)
            plugin.ODBG_Pluginmainloop(info->DebugEvent); //TODO: "If present, OllyDbg will call it on each pass of main loop."
}

PLUG_EXPORT void CBINITDEBUG(CBTYPE, PLUG_CB_INITDEBUG* info)
{
    for(auto & plugin : ollyPlugins)
        if(plugin.ODBG_Pluginreset)
            plugin.ODBG_Pluginreset();
}

PLUG_EXPORT void CBMENUPREPARE(CBTYPE, PLUG_CB_MENUPREPARE* info)
{
    char data[4096];
    for(auto & plugin : ollyPlugins)
    {
        *data = '\0';
        if(plugin.ODBG_Pluginmenu)
        {
            auto & menu = *plugin.GetMenuOrigin(info->hMenu);
            if(plugin.ODBG_Pluginmenu(menu.origin, data, 0)) //TODO: item
            {
                if(menu.data != data)
                {
                    menu.data = data;
                    if(!_plugin_menuclear(menu.hMenu))
                        __debugbreak();
                    plugin.ParseMenu(menu.data, menu.hMenu, menu.origin);
                }
                else
                    oprintf("using cached %d menu for \"%s\"\n", info->hMenu, plugin.shortname);
            }
        }
    }
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        hInstMain = hinstDLL;
    }
    return TRUE;
}