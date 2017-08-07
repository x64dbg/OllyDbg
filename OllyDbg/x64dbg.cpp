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

static int hEntryPool = 100;
struct OllyPlugin;
static std::unordered_map<int, std::pair<OllyPlugin*, int>> menuActionMap; //x64dbg hMenuEntry -> OllyDbg plugin + action

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

        if(ODBG_Plugininit(ODBG_PLUGIN_VERSION, GuiGetWindowHandle(), nullptr) == -1)
        {
            dprintf("_ODBG_Plugininit failed...\n");
            FreeLibrary(hInst);
            return false;
        }

        return true;
    }

    //ParseMenu => "Map Loader{0Load labels, 1Load comments}, 2Resource analyse, 3Process analyse, 4IDA signature loader, 5Notepad, | 15About.."
    void ParseMenu(std::string data, int hMenuRoot)
    {
        oprintf("ParseMenu => \"%s\"\n", data.c_str());

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

        for(size_t i = 0; i < data.length(); i++)
        {
            auto ch = data[i];
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
                if(ch == ',' || ch == '|' || ch == '}' || i + 1 == data.length())
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

                    menuActionMap[hEntry] = { this, int(idNum) };

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
                }
                else if(ch == '{')
                    addSubmenu();
                else
                    name.push_back(ch);
                break;
            default:
                __debugbreak();
            }
        }

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
            if(plugin.ODBG_Pluginmenu(PM_MAIN, data, 0) == 1) //TODO: implement others than PM_MAIN
                plugin.ParseMenu(data, _plugin_menuadd(hMenu, plugin.shortname));
            else
            {
                menuActionMap[hEntryPool] = { &plugin, 0 };
                _plugin_menuaddentry(hMenu, hEntryPool++, plugin.shortname);
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
    auto & plugin = *found->second.first;
    if(plugin.ODBG_Pluginaction)
        plugin.ODBG_Pluginaction(PM_MAIN, found->second.second, 0);
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