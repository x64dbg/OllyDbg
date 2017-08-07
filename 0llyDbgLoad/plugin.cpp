#include "plugin.h"
#include <string>

static std::wstring Utf8ToUtf16(const char* str)
{
    std::wstring convertedString;
    if(!str || !*str)
        return convertedString;
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if(requiredSize > 0)
    {
        convertedString.resize(requiredSize - 1);
        if(!MultiByteToWideChar(CP_UTF8, 0, str, -1, (wchar_t*)convertedString.c_str(), requiredSize))
            convertedString.clear();
    }
    return convertedString;
}

static bool FileExists(const wchar_t* file)
{
    DWORD attrib = GetFileAttributesW(file);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

static char buildPath[MAX_SETTING_SIZE];

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    _plugin_registercommand(pluginHandle, "0llyDbgLoad", [](int argc, char* argv[])
    {
        if(argc < 2)
        {
            _plugin_logputs("[" PLUGIN_NAME "] Usage: 0llyDbgLoad \"[build path]\"");
            return false;
        }
        if(!FileExists(Utf8ToUtf16(argv[1]).c_str()))
        {
            _plugin_logprintf("[" PLUGIN_NAME "] File does not exist: \"%s\"\n", argv[1]);
            return false;
        }
        _plugin_logprintf("[" PLUGIN_NAME "] Path: \"%s\"\n", argv[1]);
        BridgeSettingSet("OllyDbg", "0llyDbgLoad", argv[1]);
        return true;
    }, false);
    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here (clearing menus optional).
bool pluginStop()
{
    return true;
}

//Do GUI/Menu related things here.
void pluginSetup()
{
    if(!BridgeSettingGet("OllyDbg", "0llyDbgLoad", buildPath))
    {
        DbgCmdExecDirect("0llyDbgLoad");
        return;
    }
    _plugin_logprintf("[" PLUGIN_NAME "] Path: \"%s\"\n", buildPath);
    if(CopyFileW(Utf8ToUtf16(buildPath).c_str(), szOllyDbgPath, FALSE))
        _plugin_logprintf("[" PLUGIN_NAME "] Latest OllyDbg.exe installed!\n", buildPath);
    else
        _plugin_logprintf("[" PLUGIN_NAME "] CopyFileW failed (GetLastError() = %d)...\n", GetLastError());
}
