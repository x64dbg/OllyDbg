#pragma once

#include "../OllyDbg/pluginsdk/bridgemain.h"
#include "../OllyDbg/pluginsdk/_plugins.h"

#include "../OllyDbg/pluginsdk/_scriptapi_argument.h"
#include "../OllyDbg/pluginsdk/_scriptapi_assembler.h"
#include "../OllyDbg/pluginsdk/_scriptapi_bookmark.h"
#include "../OllyDbg/pluginsdk/_scriptapi_comment.h"
#include "../OllyDbg/pluginsdk/_scriptapi_debug.h"
#include "../OllyDbg/pluginsdk/_scriptapi_flag.h"
#include "../OllyDbg/pluginsdk/_scriptapi_function.h"
#include "../OllyDbg/pluginsdk/_scriptapi_gui.h"
#include "../OllyDbg/pluginsdk/_scriptapi_label.h"
#include "../OllyDbg/pluginsdk/_scriptapi_memory.h"
#include "../OllyDbg/pluginsdk/_scriptapi_misc.h"
#include "../OllyDbg/pluginsdk/_scriptapi_module.h"
#include "../OllyDbg/pluginsdk/_scriptapi_pattern.h"
#include "../OllyDbg/pluginsdk/_scriptapi_register.h"
#include "../OllyDbg/pluginsdk/_scriptapi_stack.h"
#include "../OllyDbg/pluginsdk/_scriptapi_symbol.h"

#include "../OllyDbg/pluginsdk/capstone/capstone.h"
#include "../OllyDbg/pluginsdk/DeviceNameResolver/DeviceNameResolver.h"
#include "../OllyDbg/pluginsdk/jansson/jansson.h"
#include "../OllyDbg/pluginsdk/lz4/lz4file.h"
#include "../OllyDbg/pluginsdk/TitanEngine/TitanEngine.h"
#include "../OllyDbg/pluginsdk/XEDParse/XEDParse.h"
#include "../OllyDbg/pluginsdk/yara/yara.h"

#ifdef _WIN64
#pragma comment(lib, "../OllyDbg/pluginsdk/x64dbg.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/x64bridge.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/capstone/capstone_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/DeviceNameResolver/DeviceNameResolver_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/jansson/jansson_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/lz4/lz4_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/TitanEngine/TitanEngine_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/XEDParse/XEDParse_x64.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/yara/yara_x64.lib")
#else
#pragma comment(lib, "../OllyDbg/pluginsdk/x32dbg.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/x32bridge.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/capstone/capstone_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/DeviceNameResolver/DeviceNameResolver_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/jansson/jansson_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/lz4/lz4_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/TitanEngine/TitanEngine_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/XEDParse/XEDParse_x86.lib")
#pragma comment(lib, "../OllyDbg/pluginsdk/yara/yara_x86.lib")
#endif //_WIN64

#define Cmd(x) DbgCmdExecDirect(x)
#define Eval(x) DbgValFromString(x)
#define PLUG_EXPORT extern "C" __declspec(dllexport)

//superglobal variables
extern int pluginHandle;
extern HWND hwndDlg;
extern int hMenu;
extern int hMenuDisasm;
extern int hMenuDump;
extern int hMenuStack;
extern wchar_t szOllyDbgPath[MAX_PATH];