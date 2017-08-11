#include "x64dbg.h"
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation of certain functions
#include "OllyDbg.h"
#include "loghacks.h"
#include "stringutils.h"
#include <unordered_map>
#include <map>

HINSTANCE hInstMain;
int pluginHandle;
HWND hwndOlly;

static int hEntryPool = 100;
struct OllyPlugin;
static std::unordered_map<int, std::tuple<OllyPlugin*, int, int>> menuActionMap; //x64dbg hMenuEntry -> plugin + action + origin
static std::unordered_map<HINSTANCE, std::string> hinstPluginMap;
static HANDLE hEventOlly;

static void createOllyWindow()
{
    hEventOlly = CreateEventW(nullptr, FALSE, TRUE, nullptr);
    CloseHandle(CreateThread(nullptr, 0, [](void*) -> DWORD
    {
        // Register the window class.
        const wchar_t CLASS_NAME[] = L"OllyDbg Window Class";
        WNDCLASSW wc = {};
        wc.hInstance = hInstMain;
        wc.lpszClassName = CLASS_NAME;
        wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            switch(uMsg)
            {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            }
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        };

        RegisterClassW(&wc);

        // Create the window.
        HWND hwnd = hwndOlly = CreateWindowExW(
                                   0, // Optional window styles.
                                   CLASS_NAME, // Window class
                                   L"OllyDbg", // Window text
                                   WS_OVERLAPPEDWINDOW, // Window style
                                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
                                   NULL, // Parent window
                                   NULL, // Menu
                                   hInstMain, // Instance handle
                                   NULL // Additional application data
                               );
        SetEvent(hEventOlly);
        if(hwnd == NULL)
        {
            __debugbreak();
            return 0;
        }

        //ShowWindow(hwnd, SW_SHOW);

        // Run the message loop.
        MSG msg = {};
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return 0;
    }, nullptr, 0, nullptr));
    WaitForSingleObject(hEventOlly, INFINITE);
    CloseHandle(hEventOlly);
}

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

struct DumpItem
{
    t_dump item;
    int hWindow;
    bool prepared;

    static ulong specfunc(char*, ulong, ulong, ulong, t_disasm*, int)
    {
        oputs("specfunc = UNIMPLEMENTED");
        return 0;
    }

    static int drawfunc(char*, char*, int*, t_sortheader*, int)
    {
        oputs("drawfunc = UNIMPLEMENTED");
        return 0;
    }

    DumpItem(int hWindow)
        : hWindow(hWindow), prepared(false)
    {
        memset(&item, 0, sizeof(item));
        item.table.drawfunc = drawfunc;
        item.specdump = specfunc;
    }

    void* PrepareItem()
    {
        oprintf("PrepareItem() = ");
        if(!prepared)
        {
            //TODO: many fields are unsupported
            SELECTIONDATA selection;
            if(!GuiSelectionGet(hWindow, &selection))
                __debugbreak();
            item.startsel = item.sel0 = selection.start;
            item.sel1 = selection.end + 1;

            oputs("prepared");
            prepared = true;
        }
        else
            oputs("cached");
        return &item;
    }
} disasmItem(GUI_DISASSEMBLY), dumpItem(GUI_DUMP), stackItem(GUI_STACK);

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
        DumpItem* item;
        std::vector<int> topMenus;
        std::vector<int> topEntries;

        Menu() { }

        Menu(int hMenu, int origin, DumpItem* item)
            : hMenu(hMenu), origin(origin), item(item)
        {
        }
    };

    Menu menuDisasm;
    Menu menuDump;
    Menu menuStack;

    OllyPlugin()
    {
        memset(shortname, 0, sizeof(shortname));
    }

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
        if(ODBG_Pluginsaveudd || ODBG_Pluginuddrecord)
        {
            eventReg(CB_CREATEPROCESS);
            eventReg(CB_EXITPROCESS);
            eventReg(CB_LOADDLL);
            eventReg(CB_UNLOADDLL);
            eventReg(CB_LOADDB);
            eventReg(CB_SAVEDB);
        }
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

        oprintf("ODBG_Plugindata =>");
        auto version = ODBG_Plugindata(shortname);
        if(version < 106 || version > 110)
        {
            dprintf("_ODBG_Plugindata returned incompatible version: %d\n", version);
            hinstPluginMap.erase(hInst);
            FreeLibrary(hInst);
            return false;
        }
        else if(!*shortname)
        {
            dputs("_ODBG_Plugindata returned empty shortname");
            hinstPluginMap.erase(hInst);
            FreeLibrary(hInst);
            return false;
        }
        oprintf(" \"%s\"\n", shortname);

        hinstPluginMap[hInst] = shortname;

        oprintf("ODBG_Plugininit");
        if(ODBG_Plugininit(ODBG_PLUGIN_VERSION, hwndOlly, nullptr) == -1)
        {
            dprintf("_ODBG_Plugininit failed...\n");
            hinstPluginMap.erase(hInst);
            FreeLibrary(hInst);
            return false;
        }
        oputs(" DONE!");

        return true;
    }

    Menu* GetMenuOrigin(int hMenu)
    {
        switch(hMenu)
        {
        case PM_DISASM:
        case GUI_DISASM_MENU:
            return &menuDisasm;
        case PM_CPUDUMP:
        case GUI_DUMP_MENU:
            return &menuDump;
        case PM_CPUSTACK:
        case GUI_STACK_MENU:
            return &menuStack;
        case PM_MAIN:
            return nullptr;
        default:
            __debugbreak();
            return nullptr;
        }
    }

    void ParseMenu(Menu* menu)
    {
        oprintf("ParseMenu (%s) => \"%s\"\n", shortname, menu->data.c_str());

        std::string id;
        std::string name;
        std::vector<int> menuStack;
        menuStack.push_back(menu->hMenu);

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
            id = StringUtils::TrimLeft(id, "0123456789 \n\r\t");
            name = StringUtils::Trim(name);

            if(id.empty() && name.empty())
                __debugbreak();
            id.append(name);

            auto hSubMenu = _plugin_menuadd(hMenu(), id.c_str());
            oprintf("added menu to %d: \"%s\" => %d\n", hMenu(), id.c_str(), hSubMenu);

            if(menuStack.size() == 1)
                menu->topMenus.push_back(hSubMenu);

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

            if(!_plugin_menuaddentry(hMenu(), hEntry, name.c_str()))
                __debugbreak();
            oprintf("added entry to %d: %d => %d, \"%s\"\n", hMenu(), hEntry, idNum, name.c_str());

            menuActionMap[hEntry] = std::make_tuple(this, int(idNum), menu->origin);

            if(menuStack.size() == 1)
                menu->topMenus.push_back(hEntry);

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

        for(size_t i = 0; i < menu->data.length(); i++)
        {
            auto ch = menu->data[i];
            switch(state)
            {
            case None:
                if(ch == '}')
                    menuStack.pop_back();
                else if(ch == '#' || ch == '|')
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

std::string sectionFromHinst(HINSTANCE dllinst)
{
    auto found = hinstPluginMap.find(dllinst);
    if(found == hinstPluginMap.end())
    {
        __debugbreak();
        return "OllyDbgPlugins";
    }
    if(found->second.empty())
        __debugbreak();
    return StringUtils::sprintf("OllyDbg %s", found->second.c_str());
}

bool ollyModFromAddr(duint addr, t_module* mod)
{
    Script::Module::ModuleInfo info;
    if(!Script::Module::InfoFromAddr(addr, &info))
        return false;

    //TODO: not all fields are populated
    memset(mod, 0, sizeof(mod));
    mod->base = info.base;
    mod->size = info.size;
    mod->entry = info.entry;
    strcpy_s(mod->path, info.path);
    mod->nsect = info.sectionCount;
    mod->issystemdll = DbgFunctions()->ModGetParty(info.base) == 1;
    return true;
}

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
    FindClose(hSearch);
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
    createOllyWindow();

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
    CloseWindow(hwndOlly);
    return true;
}

PLUG_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    char data[4096];
    for(auto & plugin : ollyPlugins)
    {
        *data = '\0';
        if(plugin.ODBG_Pluginmenu)
        {
            oprintf("ODBG_Pluginmenu => \"%s\"\n", plugin.shortname);

            plugin.menuDisasm = OllyPlugin::Menu(setupStruct->hMenuDisasm, PM_DISASM, &disasmItem);
            plugin.menuDump = OllyPlugin::Menu(setupStruct->hMenuDump, PM_CPUDUMP, &dumpItem);
            plugin.menuStack = OllyPlugin::Menu(setupStruct->hMenuStack, PM_CPUSTACK, &stackItem);

            if(plugin.ODBG_Pluginmenu(PM_MAIN, data, 0) != 0)
            {
                OllyPlugin::Menu menu(_plugin_menuadd(setupStruct->hMenu, plugin.shortname), PM_MAIN, nullptr);
                menu.data = data;
                plugin.ParseMenu(&menu);
            }
            else
            {
                menuActionMap[hEntryPool] = std::make_tuple(&plugin, 0, PM_MAIN);
                _plugin_menuaddentry(setupStruct->hMenu, hEntryPool++, plugin.shortname);
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
    {
        int origin = std::get<2>(found->second);
        int action = std::get<1>(found->second);
        auto menu = plugin.GetMenuOrigin(origin);
        plugin.ODBG_Pluginaction(origin, action, menu ? &menu->item->item : 0);
    }
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
    disasmItem.prepared = dumpItem.prepared = stackItem.prepared = false;
    for(auto & plugin : ollyPlugins)
    {
        *data = '\0';
        if(plugin.ODBG_Pluginmenu)
        {
            auto & menu = *plugin.GetMenuOrigin(info->hMenu);
            if(plugin.ODBG_Pluginmenu(menu.origin, data, menu.item->PrepareItem()))
            {
                if(menu.data != data)
                {
                    menu.data = data;
                    for(auto & hMenu : menu.topMenus)
                        if(!_plugin_menuremove(hMenu))
                            __debugbreak();
                    menu.topMenus.clear();
                    for(auto & hEntry : menu.topEntries)
                        if(!_plugin_menuentryremove(pluginHandle, hEntry))
                            __debugbreak();
                    menu.topEntries.clear();
                    plugin.ParseMenu(&menu);
                }
                else
                    oprintf("using cached %d menu for \"%s\"\n", info->hMenu, plugin.shortname);
            }
        }
    }
}

struct UddEntry
{
    ulong tag;
    std::vector<uchar> data;
};

static std::string currentUddModule;
static std::map<std::string, std::vector<UddEntry>> uddEntryMap;
static std::unordered_set<duint> modulesLoaded;

extc int cdecl Pluginsaverecord(ulong tag, ulong size, void* data)
{
    plog(__FUNCTION__, p(tag), p(size), p(data));

    if(currentUddModule.empty() || size > USERLEN)
        return 0;
    UddEntry entry;
    entry.tag = tag;
    entry.data.resize(size);
    memcpy(entry.data.data(), data, size);
    uddEntryMap[currentUddModule].push_back(std::move(entry));
    return 1;
}

static void modLoad(duint base, bool ismainmodule)
{
    static t_module uddMod;
    if(!ollyModFromAddr(base, &uddMod))
        return;
    char modname[MAX_MODULE_SIZE];
    if(!Script::Module::NameFromAddr(base, modname))
        __debugbreak();
    if(!modulesLoaded.insert(base).second)
        __debugbreak();
    auto found = uddEntryMap.find(modname);
    if(found != uddEntryMap.end())
        for(auto & entry : found->second)
            for(auto & plugin : ollyPlugins)
                if(plugin.ODBG_Pluginuddrecord)
                    if(plugin.ODBG_Pluginuddrecord(&uddMod, ismainmodule, entry.tag, entry.data.size(), entry.data.data()) != 0)
                        break;
}

static void modUnload(duint base, bool ismainmodule)
{
    static t_module uddMod;
    if(!ollyModFromAddr(base, &uddMod))
        return;
    char modname[MAX_MODULE_SIZE];
    if(!Script::Module::NameFromAddr(base, modname))
        __debugbreak();
    currentUddModule = modname;
    for(auto & plugin : ollyPlugins)
        if(plugin.ODBG_Pluginsaveudd)
            plugin.ODBG_Pluginsaveudd(&uddMod, ismainmodule);
    currentUddModule.clear();
    if(!modulesLoaded.erase(base))
        __debugbreak();
}

PLUG_EXPORT void CBCREATEPROCESS(CBTYPE, PLUG_CB_CREATEPROCESS* info)
{
    modLoad(duint(info->CreateProcessInfo->lpBaseOfImage), true);
}

PLUG_EXPORT void CBEXITPROCESS(CBTYPE, PLUG_CB_EXITPROCESS* info)
{
    auto mainbase = Script::Module::GetMainModuleBase();
    for(auto & modbase : modulesLoaded)
        modUnload(modbase, modbase == mainbase);
    modulesLoaded.clear();
}

PLUG_EXPORT void CBLOADDLL(CBTYPE, PLUG_CB_LOADDLL* info)
{
    modLoad(duint(info->LoadDll->lpBaseOfDll), false);
}

PLUG_EXPORT void CBUNLOADDLL(CBTYPE, PLUG_CB_UNLOADDLL* info)
{
    modUnload(duint(info->UnloadDll->lpBaseOfDll), false);
}

PLUG_EXPORT void CBLOADDB(CBTYPE, PLUG_CB_LOADSAVEDB* info)
{
    auto tagModulesArray = json_object_get(info->root, PLUGIN_NAME "Tags");
    size_t i;
    json_t* tagModuleJson;
    uddEntryMap.clear();
    json_array_foreach(tagModulesArray, i, tagModuleJson)
    {
        auto modJson = json_object_get(tagModuleJson, "mod");
        auto tagsArray = json_object_get(tagModuleJson, "tags");
        size_t j;
        json_t* tagJson;
        if(modJson)
        {
            auto mod = json_string_value(modJson);
            if(mod)
            {
                json_array_foreach(tagsArray, j, tagJson)
                {
                    auto valJson = json_object_get(tagJson, "tag");
                    auto dataJson = json_object_get(tagJson, "data");
                    if(valJson && dataJson)
                    {
                        auto val = json_integer_value(valJson);
                        auto data = json_string_value(dataJson);
                        if(data)
                        {
                            UddEntry entry;
                            entry.tag = ulong(val);
                            if(StringUtils::FromCompressedHex(data, entry.data))
                                uddEntryMap[mod].push_back(std::move(entry));
                            else
                                __debugbreak();
                        }
                        else
                            __debugbreak();
                    }
                    else
                        __debugbreak();
                }
            }
            else
                __debugbreak();
        }
        else
            __debugbreak();
    }
}

PLUG_EXPORT void CBSAVEDB(CBTYPE, PLUG_CB_LOADSAVEDB* info)
{
    auto tagModulesArray = json_array();
    for(const auto & it : uddEntryMap)
    {
        if(it.second.empty())
            continue;
        auto tagModuleJson = json_object();
        json_object_set_new(tagModuleJson, "mod", json_string(it.first.c_str()));
        auto tagsArray = json_array();
        for(const auto & entry : it.second)
        {
            auto tagJson = json_object();
            json_object_set_new(tagJson, "tag", json_integer(entry.tag));
            json_object_set_new(tagJson, "data", json_string(StringUtils::ToCompressedHex(entry.data.data(), entry.data.size()).c_str()));
            json_array_append_new(tagsArray, tagJson);
        }
        json_object_set_new(tagModuleJson, "tags", tagsArray);
        json_array_append_new(tagModulesArray, tagModuleJson);
    }
    json_object_set_new(info->root, PLUGIN_NAME "Tags", tagModulesArray);
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