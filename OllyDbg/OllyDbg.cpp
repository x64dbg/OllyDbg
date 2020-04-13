#include <windows.h>
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation
#include "OllyDbg.h"
#include "x64dbg.h"
#include <stdarg.h>
#include "loghacks.h"
#include "stringutils.h"
#include <map>

//NOTE: SEG_XXX are overloaded. The OllyDbg ones are renamed to ODBG_SEG_XXX

#define var_plog(function, ...) \
    { \
        oprintf("%s(\n", function); \
        logArgs(__VA_ARGS__); \
        oprintf("  format: \"%s\"", format); \
        if(strchr(format, '%')) \
        { \
            oprintf(" => \""); \
            oprintf_args(format, args); \
            oprintf("\""); \
        } \
        oprintf("\n"); \
        oprintf(")\n"); \
    }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// INFORMATION FUNCTIONS /////////////////////////////
extc void cdecl Addtolist(long addr, int highlight, char* format, ...)
{
    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__, p(addr), p(highlight));
    auto msg = StringUtils::LocalCpToUtf8(StringUtils::vsprintf(format, args));
    va_end(args);

    if(addr)
        _plugin_logprintf("%p %s\n", addr, msg.c_str());
    else
        _plugin_logputs(msg.c_str());
}

extc void cdecl Updatelist()
{
    ulog(__FUNCTION__);
    GuiFlushLog();
}

extc HWND cdecl Createlistwindow()
{
    ulog(__FUNCTION__);
    //TODO: "Creates or restores log window (window that displays contents of log buffer) on the screen.";
    return 0;
}

extc void cdecl Error(char* format, ...)
{
    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__);
    auto msg = StringUtils::vsprintf(format, args);
    va_end(args);

    MessageBoxW(GuiGetWindowHandle(), StringUtils::LocalCpToUtf16(msg).c_str(), L"OllyDbg error", MB_ICONERROR);
}

extc void cdecl Message(ulong addr, char* format, ...)
{
    if(!format)
    {
        //TODO: "If format is NULL, message will be removed from the bottom line but not added to the log."
        GuiAddStatusBarMessage("\n");
        return;
    }

    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__, p(addr));
    auto msg = StringUtils::LocalCpToUtf8(StringUtils::vsprintf(format, args));
    va_end(args);

    auto dollar = msg.find('$');
    if(dollar == String::npos)
        _plugin_logputs(msg.c_str());
    else
    {
        auto log = msg.substr(0, dollar);
        _plugin_logputs(log.c_str());
        msg[dollar] = '-';
        msg.push_back('\n');
        GuiAddStatusBarMessage(msg.c_str());
    }
}

extc void cdecl Infoline(char* format, ...)
{
    if(!format)
    {
        GuiAddStatusBarMessage("\n");
        return;
    }

    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__);
    auto msg = StringUtils::LocalCpToUtf8(StringUtils::vsprintf(format, args));
    va_end(args);

    msg.push_back('\n');
    GuiAddStatusBarMessage(msg.c_str());
}

extc void cdecl Progress(int promille, char* format, ...)
{
    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__, p(promille));
    auto msg = StringUtils::LocalCpToUtf8(StringUtils::vsprintf(format, args));
    va_end(args);

    if(!promille) //TODO: "If promille is 0, function closes progress bar restores previously displayed message."
    {
        GuiAddStatusBarMessage("\n");
        return;
    }

    auto dollar = msg.find('$');
    if(dollar == String::npos)
        GuiAddStatusBarMessage(StringUtils::sprintf("%.1f%%: %s\n", promille / 10.0, msg.c_str()).c_str());
    else
    {
        auto before = msg.substr(0, dollar);
        auto after = msg.substr(dollar + 1);
        GuiAddStatusBarMessage(StringUtils::sprintf("%s- %.1f%% -%s\n", before.c_str(), promille / 10.0, after.c_str()).c_str());
    }
}

extc void cdecl Flash(char* format, ...)
{
    va_list args;
    va_start(args, format);
    var_plog(__FUNCTION__);
    auto msg = StringUtils::LocalCpToUtf8(StringUtils::vsprintf(format, args));
    va_end(args);
    msg.push_back('\n');
    GuiAddStatusBarMessage(msg.c_str()); //TODO: "This message automatically disappears in 500 milliseconds."
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// DATA FORMATTING FUNCTIONS ///////////////////////////
extc int cdecl Decodeaddress(ulong addr, ulong base, int addrmode, char* symb, int nsymb, char* comment)
{
    plog(__FUNCTION__, p(addr), p(base), p(addrmode), p(symb), p(nsymb), p(comment));

    //TODO: implement properly
    if(nsymb)
    {
        sprintf_s(symb, nsymb, "%p", addr);
        return strlen(symb);
    }
    return 0;
}

extc int cdecl Decoderelativeoffset(ulong addr, int addrmode, char* symb, int nsymb) { ulog(__FUNCTION__, p(addr), p(addrmode), p(symb), p(nsymb)) return 0; }

extc int cdecl Decodecharacter(char* s, uint c) { ulog(__FUNCTION__, p(s), p(c)) return 0; }

//extc int cdecl Printfloat4(char* s, float f) { ulog(__FUNCTION__, p(s), p(f)) return 0; }

//extc int cdecl Printfloat8(char* s, double d) { ulog(__FUNCTION__, p(s), p(d)) return 0; }

//TODO: this will break because sizeof(long double) != 10 with MSVC
//extc int cdecl Printfloat10(char* s, long double ext) { __debugbreak(); return 0; }

//extc int cdecl Print3dnow(char* s, uchar* f) { ulog(__FUNCTION__, p(s), p(f)) return 0; }

extc int cdecl Printsse(char* s, char* f) { ulog(__FUNCTION__, p(s), p(f)) return 0; }

extc ulong cdecl Followcall(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc int cdecl IstextA(char c) { ulog(__FUNCTION__, p(c)) return 0; }

extc int cdecl IstextW(wchar_t w) { ulog(__FUNCTION__, p(w)) return 0; }

extc int cdecl Stringtotext(char* data, int ndata, char* text, int ntext) { ulog(__FUNCTION__, p(data), p(ndata), p(text), p(ntext)) return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// DATA INPUT FUNCTIONS /////////////////////////////
extc int cdecl Getlong(char* title, ulong* data, int datasize, char letter, int mode) { ulog(__FUNCTION__, p(title), p(data), p(datasize), p(letter), p(mode)) return 0; }

extc int cdecl Getlongxy(char* title, ulong* data, int datasize, char letter, int mode, int x, int y) { ulog(__FUNCTION__, p(title), p(data), p(datasize), p(letter), p(mode), p(x), p(y)) return 0; }

extc int cdecl Getregxy(char* title, ulong* data, char letter, int x, int y) { ulog(__FUNCTION__, p(title), p(data), p(letter), p(x), p(y)) return 0; }

extc int cdecl Getline(char* title, ulong* data) { ulog(__FUNCTION__, p(title), p(data)) return 0; }

extc int cdecl Getlinexy(char* title, ulong* data, int x, int y) { ulog(__FUNCTION__, p(title), p(data), p(x), p(y)) return 0; }

extc int cdecl Getfloat10(char* title, long double* fdata, uchar* tag, char letter, int mode) { ulog(__FUNCTION__, p(title), p(fdata), p(tag), p(letter), p(mode)) return 0; }

extc int cdecl Getfloat10xy(char* title, long double* fdata, char* tag, char letter, int mode, int x, int y) { ulog(__FUNCTION__, p(title), p(fdata), p(tag), p(letter), p(mode), p(x), p(y)) return 0; }

extc int cdecl Getfloat(char* title, void* fdata, int size, char letter, int mode) { ulog(__FUNCTION__, p(title), p(fdata), p(size), p(letter), p(mode)) return 0; }

extc int cdecl Getfloatxy(char* title, void* fdata, int size, char letter, int mode, int x, int y) { ulog(__FUNCTION__, p(title), p(fdata), p(size), p(letter), p(mode), p(x), p(y)) return 0; }

extc void cdecl Getasmfindmodel(t_asmmodel model[NMODELS], char letter, int searchall) { ulog(__FUNCTION__, p(model), p(letter), p(searchall)) }

extc void cdecl Getasmfindmodelxy(t_asmmodel model[NMODELS], char letter, int searchall, int x, int y) { ulog(__FUNCTION__, p(model), p(letter), p(searchall), p(x), p(y)) }

extc int cdecl Gettext(char* title, char* text, char letter, int type, int fontindex) { ulog(__FUNCTION__, p(title), p(text), p(letter), p(type), p(fontindex)) return 0; }

extc int cdecl Gettextxy(char* title, char* text, char letter, int type, int fontindex, int x, int y) { ulog(__FUNCTION__, p(title), p(text), p(letter), p(type), p(fontindex), p(x), p(y)) return 0; }

extc int cdecl Gethexstring(char* title, t_hexstr* hs, int mode, int fontindex, char letter) { ulog(__FUNCTION__, p(title), p(hs), p(mode), p(fontindex), p(letter)) return 0; }

extc int cdecl Gethexstringxy(char* title, t_hexstr* hs, int mode, int fontindex, char letter, int x, int y) { ulog(__FUNCTION__, p(title), p(hs), p(mode), p(fontindex), p(letter), p(x), p(y)) return 0; }

extc int cdecl Getmmx(char* title, uchar* data, int mode) { ulog(__FUNCTION__, p(title), p(data), p(mode)) return 0; }

extc int cdecl Getmmxxy(char* title, char* data, int mode, int x, int y) { ulog(__FUNCTION__, p(title), p(data), p(mode), p(x), p(y)) return 0; }

extc int cdecl Get3dnow(char* title, uchar* data, int mode) { ulog(__FUNCTION__, p(title), p(data), p(mode)) return 0; }

extc int cdecl Get3dnowxy(char* title, char* data, int mode, int x, int y) { ulog(__FUNCTION__, p(title), p(data), p(mode), p(x), p(y)) return 0; }

extc int cdecl Browsefilename(char* title, char* name, char* defext, int getarguments)
/* TODO: Add a hook to implement getarguments
getarguments - mode of operation.Modes 3 to 8 are not intended for use in plugins and are not described here :
0 standard dialog without additional elements
1 dialog with combobox "Arguments"
2 dialog with checkbox "Append to existing file"
New in version 1.10: if mode is ORed with 0x80, Browsefilename opens Save File dialog instead of Open File.
*/
{
    OPENFILENAME ofn;

    plog(__FUNCTION__, p(title), p(name), p(defext), p(getarguments));
    if(getarguments > 0)
        oputs("UNIMPLEMENTED: mode value");

    // Zero out name so that GetOpenFileName does
    // not use the contents to initialize itself and hence fail
    ZeroMemory(name, sizeof(MAX_PATH));
    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GuiGetWindowHandle();
    ofn.lpstrFile = name;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = defext;
    ofn.lpstrDefExt = defext;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    return (int)(FALSE != GetOpenFileName(&ofn));
}

extc int cdecl OpenEXEfile(char* path, int dropped) { ulog(__FUNCTION__, p(path), p(dropped)) return 0; }

extc int cdecl Attachtoactiveprocess(int newprocessid) { ulog(__FUNCTION__, p(newprocessid)) return 0; }

extc void cdecl Animate(int animation) { ulog(__FUNCTION__, p(animation)) }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// SORTED DATA FUNCTIONS /////////////////////////////
extc int cdecl Createsorteddata(t_sorted* sd, char* name, int itemsize, int nmax, SORTFUNC* sortfunc, DESTFUNC* destfunc) { ulog(__FUNCTION__, p(sd), p(name), p(itemsize), p(nmax), p(sortfunc), p(destfunc)) return 0; }

extc void cdecl Destroysorteddata(t_sorted* sd) { ulog(__FUNCTION__, p(sd)) }

extc void* cdecl Addsorteddata(t_sorted* sd, void* item) { ulog(__FUNCTION__, p(sd), p(item)) return 0; }

extc void cdecl Deletesorteddata(t_sorted* sd, ulong addr) { ulog(__FUNCTION__, p(sd), p(addr)) }

extc void cdecl Deletesorteddatarange(t_sorted* sd, ulong addr0, ulong addr1) { ulog(__FUNCTION__, p(sd), p(addr0), p(addr1)) }

extc int cdecl Deletenonconfirmedsorteddata(t_sorted* sd) { ulog(__FUNCTION__, p(sd)) return 0; }

extc void* cdecl Findsorteddata(t_sorted* sd, ulong addr) { ulog(__FUNCTION__, p(sd), p(addr)) return 0; }

extc void* cdecl Findsorteddatarange(t_sorted* sd, ulong addr0, ulong addr1) { ulog(__FUNCTION__, p(sd), p(addr0), p(addr1)) return 0; }

extc int cdecl Findsorteddataindex(t_sorted* sd, ulong addr0, ulong addr1) { ulog(__FUNCTION__, p(sd), p(addr0), p(addr1)) return 0; }

extc int cdecl Sortsorteddata(t_sorted* sd, int sort) { ulog(__FUNCTION__, p(sd), p(sort)) return 0; }

extc void* cdecl Getsortedbyselection(t_sorted* sd, int index) { ulog(__FUNCTION__, p(sd), p(index)) return 0; }

extc void cdecl Defaultbar(t_bar* pb) { ulog(__FUNCTION__, p(pb)) }

extc int cdecl Tablefunction(t_table* pt, HWND hw, UINT msg, WPARAM wp, LPARAM lp) { ulog(__FUNCTION__, p(pt), p(hw), p(msg), p(wp), p(lp)) return 0; }

extc void cdecl Painttable(HWND hw, t_table* pt, DRAWFUNC getline) { ulog(__FUNCTION__, p(hw), p(pt), p(getline)) }

extc int cdecl Gettableselectionxy(t_table* pt, int column, int* px, int* py) { ulog(__FUNCTION__, p(pt), p(column), p(px), p(py)) return 0; }

extc void cdecl Selectandscroll(t_table* pt, int index, int mode) { ulog(__FUNCTION__, p(pt), p(index), p(mode)) }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// NAME FUNCTIONS ////////////////////////////////
extc int cdecl Insertname(ulong addr, int type, char* name)
{
    plog(__FUNCTION__, p(addr), p(type), p(name));
    switch(type)
    {
    case NM_NONAME:
        oputs("UNIMPLEMENTED: NM_NONAME");
        break;
    case NM_ANYNAME:
        oputs("UNIMPLEMENTED: NM_ANYNAME");
        break;
    case NM_PLUGCMD:
        oputs("UNIMPLEMENTED: NM_PLUGCMD");
        break;
    case NM_LABEL:
        DbgSetLabelAt(addr, name);
        break;
    case NM_EXPORT:
        oputs("UNIMPLEMENTED: NM_EXPORT");
        break;
    case NM_IMPORT:
        oputs("UNIMPLEMENTED: NM_IMPORT");
        break;
    case NM_LIBRARY:
        oputs("UNIMPLEMENTED: NM_LIBRARY");
        break;
    case NM_CONST:
        oputs("UNIMPLEMENTED: NM_CONST");
        break;
    case NM_COMMENT:
        DbgSetCommentAt(addr, name);
        break;
    case NM_LIBCOMM:
        oputs("UNIMPLEMENTED: NM_LIBCOMM");
        break;
    case NM_BREAK:
        oputs("UNIMPLEMENTED: NM_BREAK");
        break;
    case NM_ARG:
        oputs("UNIMPLEMENTED: NM_ARG");
        break;
    case NM_ANALYSE:
        oputs("UNIMPLEMENTED: NM_ANALYSE");
        break;
    case NM_BREAKEXPR:
        oputs("UNIMPLEMENTED: NM_BREAKEXPR");
        break;
    case NM_BREAKEXPL:
        oputs("UNIMPLEMENTED: NM_BREAKEXPL");
        break;
    case NM_ASSUME:
        oputs("UNIMPLEMENTED: NM_ASSUME");
        break;
    case NM_STRUCT:
        oputs("UNIMPLEMENTED: NM_STRUCT");
        break;
    case NM_CASE:
        oputs("UNIMPLEMENTED: NM_CASE");
        break;
    case NM_INSPECT:
        oputs("UNIMPLEMENTED: NM_INSPECT");
        break;
    case NM_WATCH:
        oputs("UNIMPLEMENTED: NM_WATCH");
        break;
    case NM_ASM:
        oputs("UNIMPLEMENTED: NM_ASM");
        break;
    case NM_FINDASM:
        oputs("UNIMPLEMENTED: NM_FINDASM");
        break;
    case NM_LASTWATCH:
        oputs("UNIMPLEMENTED: NM_LASTWATCH");
        break;
    case NM_SOURCE:
        oputs("UNIMPLEMENTED: NM_SOURCE");
        break;
    case NM_REFTXT:
        oputs("UNIMPLEMENTED: NM_REFTXT");
        break;
    case NM_GOTO:
        oputs("UNIMPLEMENTED: NM_GOTO");
        break;
    case NM_GOTODUMP:
        oputs("UNIMPLEMENTED: NM_GOTODUMP");
        break;
    case NM_TRPAUSE:
        oputs("UNIMPLEMENTED: NM_TRPAUSE");
        break;
    case NM_IMCALL:
        oputs("UNIMPLEMENTED: NM_IMCALL");
        break;
    //case NMHISTORY:
    //oputs("UNIMPLEMENTED: NMHISTORY");
    //break;
    default:
        break;
    }

    GuiUpdateAllViews();
    return 0;
}

// Quick names table
static std::map<std::pair<int, duint>, std::string> quickNames;

extc int cdecl Quickinsertname(ulong addr, int type, char* name)
{
    plog(__FUNCTION__, p(addr), p(type), p(name));

    if(!*name)
        return 0;

    auto key = std::make_pair(type, addr);
    auto item = quickNames.find(key);
    if(item != quickNames.end())  // replacing
    {
        if(item->second != name)  // edit name
            item->second = name;
    }
    else
        quickNames.insert({ key, name }); // add new entry

    return 0;
}

extc void cdecl Mergequicknames()
{
    plog(__FUNCTION__)

    for(auto & item : quickNames)
    {
        auto key = item.first;
        Insertname((duint)key.first, (int)key.second, (char*)item.second.c_str());
    }

    quickNames.clear();
}

extc void cdecl Discardquicknames()
{
    plog(__FUNCTION__);
    quickNames.clear();
}

extc int cdecl Findname(ulong addr, int type, char* name) { ulog(__FUNCTION__, p(addr), p(type), p(name)) return 0; }

extc int cdecl Decodename(ulong addr, int type, char* name) { ulog(__FUNCTION__, p(addr), p(type), p(name)) return 0; }

extc ulong cdecl Findnextname(char* name) { ulog(__FUNCTION__, p(name)) return 0; }

extc int cdecl Findlabel(ulong addr, char* name) { ulog(__FUNCTION__, p(addr), p(name)) return 0; }

extc void cdecl Deletenamerange(ulong addr0, ulong addr1, int type) { ulog(__FUNCTION__, p(addr0), p(addr1), p(type)) }

extc int cdecl Findlabelbyname(char* name, ulong* addr, ulong addr0, ulong addr1) { ulog(__FUNCTION__, p(name), p(addr), p(addr0), p(addr1)) return 0; }

extc ulong cdecl Findimportbyname(char* name, ulong addr0, ulong addr1) { ulog(__FUNCTION__, p(name), p(addr0), p(addr1)) return 0; }

extc int cdecl Demanglename(char* name, int type, char* undecorated) { ulog(__FUNCTION__, p(name), p(type), p(undecorated)) return 0; }

extc int cdecl Findsymbolicname(ulong addr, char* fname) { ulog(__FUNCTION__, p(addr), p(fname)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// DISASSEMBLY FUNCTIONS /////////////////////////////
//extc ulong cdecl Disasm(uchar* src, ulong srcsize, ulong srcip, uchar* srcdec, t_disasm* disasm, int disasmmode, ulong threadid) { ulog(__FUNCTION__, p(src), p(srcsize), p(srcip), p(srcdec), p(disasm), p(disasmmode), p(threadid)) return 0; }

//extc ulong cdecl Disassembleback(uchar* block, ulong base, ulong size, ulong ip, int n, int usedec) { ulog(__FUNCTION__, p(block), p(base), p(size), p(ip), p(n), p(usedec)) return 0; }

//extc ulong cdecl Disassembleforward(uchar* block, ulong base, ulong size, ulong ip, int n, int usedec) { ulog(__FUNCTION__, p(block), p(base), p(size), p(ip), p(n), p(usedec)) return 0; }

extc int cdecl Issuspicious(char* cmd, ulong size, ulong ip, ulong threadid, t_reg* preg, char* s) { ulog(__FUNCTION__, p(cmd), p(size), p(ip), p(threadid), p(preg), p(s)) return 0; }

//extc int cdecl Isfilling(ulong offset, char* data, ulong size, ulong align) { ulog(__FUNCTION__, p(offset), p(data), p(size), p(align)) return 0; }

extc int cdecl Isprefix(int c) { ulog(__FUNCTION__, p(c)) return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// ASSEMBLY FUNCTIONS //////////////////////////////
//extc int cdecl Assemble(char* cmd, ulong ip, t_asmmodel* model, int attempt, int constsize, char* errtext) { ulog(__FUNCTION__, p(cmd), p(ip), p(model), p(attempt), p(constsize), p(errtext)) return 0; }

//extc int cdecl Checkcondition(int code, ulong flags) { ulog(__FUNCTION__, p(code), p(flags)) return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// EXPRESSIONS //////////////////////////////////
extc int cdecl Expression(t_result* result, char* expression, int a, int b, uchar* data, ulong database, ulong datasize, ulong threadid) { ulog(__FUNCTION__, p(result), p(expression), p(a), p(b), p(data), p(database), p(datasize), p(threadid)) return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// THREAD FUNCTIONS ///////////////////////////////
extc HWND cdecl Createthreadwindow() { ulog(__FUNCTION__) return 0; }

extc t_thread* cdecl Findthread(ulong threadid) { ulog(__FUNCTION__, p(threadid)) return 0; }

extc int cdecl Decodethreadname(char* s, ulong threadid, int mode) { ulog(__FUNCTION__, p(s), p(threadid), p(mode)) return 0; }

extc ulong cdecl Getcputhreadid() { ulog(__FUNCTION__) return 0; }

extc ulong cdecl Runsinglethread(ulong threadid) { ulog(__FUNCTION__, p(threadid)) return 0; }

extc void cdecl Restoreallthreads() { ulog(__FUNCTION__) }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MEMORY FUNCTIONS ///////////////////////////////
extc int cdecl Listmemory() { ulog(__FUNCTION__) return 0; }

extc t_memory* cdecl Findmemory(ulong addr)
{
    plog(__FUNCTION__, p(addr));

    //TODO: not all fields are implemented
    static t_memory mem;
    memset(&mem, 0, sizeof(mem));
    duint size;
    duint base = DbgMemFindBaseAddr(addr, &size);
    if(!base)
        return 0;
    mem.owner = mem.base = base;
    mem.size = size;
    mem.type = TY_CODE; //TODO: find something proper
    mem.access = mem.initaccess = Script::Memory::GetProtect(base);
    return &mem;
}

extc int cdecl Guardmemory(ulong base, ulong size, int guard) { ulog(__FUNCTION__, p(base), p(size), p(guard)) return 0; }

extc void cdecl Havecopyofmemory(uchar* copy, ulong base, ulong size) { ulog(__FUNCTION__, p(copy), p(base), p(size)) }

extc ulong cdecl Readmemory(void* buf, ulong addr, ulong size, int mode)
{
    plog(__FUNCTION__, p(buf), p(addr), p(size), p(mode));

    //TODO: implement mode:MM_RESTORE
    duint sizeRead = 0;
    Script::Memory::Read(addr, buf, size, &sizeRead);
    return sizeRead;
}

extc ulong cdecl Writememory(void* buf, ulong addr, ulong size, int mode)
{
    ulog(__FUNCTION__, p(buf), p(addr), p(size), p(mode));

    //TODO: implement mode:MM_RESTORE and MM_DELANAL
    duint sizeWritten = 0;
    Script::Memory::Write(addr, buf, size, &sizeWritten);
    return sizeWritten;
}

extc ulong cdecl Readcommand(ulong ip, char* cmd) { ulog(__FUNCTION__, p(ip), p(cmd)) return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MODULE FUNCTIONS ///////////////////////////////
extc t_module* cdecl Findmodule(ulong addr)
{
    plog(__FUNCTION__, p(addr));

    static t_module mod;
    return ollyModFromAddr(addr, &mod) ? &mod : 0;
}

extc t_fixup* cdecl Findfixup(t_module* pmod, ulong addr) { ulog(__FUNCTION__, p(pmod), p(addr)) return 0; }

extc uchar* cdecl Finddecode(ulong addr, ulong* psize)
{
    ulog(__FUNCTION__, p(addr), p(psize));
    *psize = 0;
    return 0;
}

extc ulong cdecl Findfileoffset(t_module* pmod, ulong addr) { ulog(__FUNCTION__, p(pmod), p(addr)) return 0; }

extc int cdecl Decoderange(ulong addr, ulong size, char* s) { ulog(__FUNCTION__, p(addr), p(size), p(s)) return 0; }

extc int cdecl Analysecode(t_module* pm) { ulog(__FUNCTION__, p(pm)) return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// DUMP /////////////////////////////////////
extc int cdecl Registerotclass(char* classname, char* iconname, WNDPROC classproc) { ulog(__FUNCTION__, p(classname), p(iconname), p(classproc)) return 0; }

extc HWND cdecl Newtablewindow(t_table* pt, int nlines, int maxcolumns, char* winclass, char* wintitle) { ulog(__FUNCTION__, p(pt), p(nlines), p(maxcolumns), p(winclass), p(wintitle)) return 0; }

extc HWND cdecl Quicktablewindow(t_table* pt, int nlines, int maxcolumns, char* winclass, char* wintitle) { ulog(__FUNCTION__, p(pt), p(nlines), p(maxcolumns), p(winclass), p(wintitle)) return 0; }

extc HWND cdecl Createdumpwindow(char* name, ulong base, ulong size, ulong addr, int type, SPECFUNC* specdump) { ulog(__FUNCTION__, p(name), p(base), p(size), p(addr), p(type), p(specdump)) return 0; }

extc void cdecl Setdumptype(t_dump* pd, int dumptype) { ulog(__FUNCTION__, p(pd), p(dumptype)) }

extc void cdecl Dumpbackup(t_dump* pd, int action) { ulog(__FUNCTION__, p(pd), p(action)) }

extc int cdecl Broadcast(UINT msg, WPARAM wp, LPARAM lp) { ulog(__FUNCTION__, p(msg), p(wp), p(lp)) return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// DATA CONVERSION FUNCTIONS ///////////////////////////
extc ulong cdecl Compress(uchar* bufin, ulong nbufin, uchar* bufout, ulong nbufout) { ulog(__FUNCTION__, p(bufin), p(nbufin), p(bufout), p(nbufout)) return 0; }

extc ulong cdecl Getoriginaldatasize(char* bufin, ulong nbufin) { ulog(__FUNCTION__, p(bufin), p(nbufin)) return 0; }

extc ulong cdecl Decompress(uchar* bufin, ulong nbufin, uchar* bufout, ulong nbufout) { ulog(__FUNCTION__, p(bufin), p(nbufin), p(bufout), p(nbufout)) return 0; }

extc ulong cdecl Calculatecrc(uchar* copy, ulong base, ulong size, t_module* pmod, ulong fixupoffset) { ulog(__FUNCTION__, p(copy), p(base), p(size), p(pmod), p(fixupoffset)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// REFERENCES AND SEARCH /////////////////////////////
extc int cdecl Findreferences(ulong base, ulong size, ulong addr0, ulong addr1, ulong origin, int recurseonjump, char* title) { ulog(__FUNCTION__, p(base), p(size), p(addr0), p(addr1), p(origin), p(recurseonjump), p(title)) return 0; }

extc int cdecl Findstrings(ulong base, ulong size, ulong origin, char* title) { ulog(__FUNCTION__, p(base), p(size), p(origin), p(title)) return 0; }

extc int cdecl Findalldllcalls(t_dump* pd, ulong origin, char* title) { ulog(__FUNCTION__, p(pd), p(origin), p(title)) return 0; }

extc int cdecl Findallcommands(t_dump* pd, t_asmmodel* model, ulong origin, char* title) { ulog(__FUNCTION__, p(pd), p(model), p(origin), p(title)) return 0; }

extc int cdecl Findallsequences(t_dump* pd, t_extmodel model[NSEQ][NMODELS], ulong origin, char* title) { ulog(__FUNCTION__, p(pd), p(model), p(origin), p(title)) return 0; }

extc ulong cdecl Walkreference(int dir) { ulog(__FUNCTION__, p(dir)) return 0; }

extc ulong cdecl Walkreferenceex(int dir, ulong* size) { ulog(__FUNCTION__, p(dir), p(size)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////// BREAKPOINT AND TRACE FUNCTIONS ////////////////////////
extc int cdecl Setbreakpoint(ulong addr, ulong type, uchar cmd) // deprecated version of Setbreakpointext
{
    plog(__FUNCTION__, p(addr), p(type), p(cmd));

    return Setbreakpointext(addr, type, cmd, 0); // call newer function
}

extc int cdecl Setbreakpointext(ulong addr, ulong type, char cmd, ulong passcount)
{
    char command[MAX_PATH];

    plog(__FUNCTION__, p(addr), p(type), p(cmd), p(passcount));

    // TODO: not all flags are implemented
    // TY_SETCOUNT : NOT DEFINED IN OllyDbg.h. Force pass count even if breakpoint already exists
    if((type & TY_TEMP) == TY_TEMP)
        oputs("UNIMPLEMENTED: TY_TEMP ");
    if((type & TY_STOPAN) == TY_STOPAN)
        oputs("UNIMPLEMENTED: TY_STOPAN ");
    if((type & TY_KEEPCODE) == TY_KEEPCODE)
        oputs("UNIMPLEMENTED: TY_KEEPCODE ");

    if((type & TY_ACTIVE) == TY_ACTIVE)
        ((type & TY_ONESHOT) == TY_ONESHOT) ? sprintf_s(command, "bp %X,ss", addr) : sprintf_s(command, "bp %X", addr); // using command in order to use the type parameter
    else
        sprintf_s(command, "bpd %X", addr);

    return (int)DbgCmdExecDirect(command);

    return 0;
}

extc int cdecl Manualbreakpoint(ulong addr, int key, int shiftkey, ulong nametype, int font) { ulog(__FUNCTION__, p(addr), p(key), p(shiftkey), p(nametype), p(font)) return 0; }

extc void cdecl Deletebreakpoints(ulong addr0, ulong addr1, int silent) { ulog(__FUNCTION__, p(addr0), p(addr1), p(silent)) } // no doc found

extc ulong cdecl Getbreakpointtype(ulong addr) // no doc found but apparently same as Getbreakpointtypecount
{
    plog(__FUNCTION__, p(addr));
    return Getbreakpointtypecount(addr, NULL);
}

extc ulong cdecl Getbreakpointtypecount(ulong addr, ulong* passcount)
{
    BRIDGEBP bp;
    BRIDGEBP* bp_found = NULL;
    ulong bp_flags = 0;

    memset(&bp, 0, sizeof(BRIDGEBP));

    DbgFunctions()->GetBridgeBp(bp_normal, addr, &bp);
    if(bp.type != bp_none)
        bp_found = &bp;

    if(!bp_found)
    {
        DbgFunctions()->GetBridgeBp(bp_hardware, addr, &bp);
        if(bp.type != bp_none)
            bp_found = &bp;
    }

    if(!bp_found)
    {
        DbgFunctions()->GetBridgeBp(bp_memory, addr, &bp);
        if(bp.type != bp_none)
            bp_found = &bp;
    }

    // Olly doesn't handle bp_dll or bp_exception so these are skipped

    if(bp_found)
    {
        // TODO: Missing flags TY_SET and TY_TEMP implementation
        (bp_found->active) ? bp_flags |= TY_ACTIVE : bp_flags |= TY_DISABLED;
        if(bp_found->singleshoot)
            bp_flags |= TY_ONESHOT;

        if(passcount)
            *passcount = bp_found->hitCount;
    }

    plog(__FUNCTION__, p(addr), p(passcount));

    return bp_flags;
}

extc ulong cdecl Getnextbreakpoint(ulong addr, ulong* type, int* cmd) { ulog(__FUNCTION__, p(addr), p(type), p(cmd)) return 0; } // no doc found

extc void cdecl Tempbreakpoint(ulong addr, int mode)
{
    //char command[MAX_PATH];

    plog(__FUNCTION__, p(addr), p(mode));

    // TODO: not all flags are implemented
    if((mode & (TY_ONESHOT | TY_KEEPCOND | TY_STOPAN)) == (TY_ONESHOT | TY_KEEPCOND | TY_STOPAN))
        oputs("UNIMPLEMENTED: TY_ONESHOT|TY_KEEPCOND|TY_STOPAN ");
    //  sprintf_s(command, "bp %X,ss", addr);
    if((mode & (TY_TEMP | TY_KEEPCOND)) == (TY_TEMP | TY_KEEPCOND))
        oputs("UNIMPLEMENTED: TY_TEMP | TY_KEEPCOND ");
    //  sprintf_s(command, "bpd %X", addr);
    //DbgCmdExecDirect(command);

    Setbreakpointext(addr, TY_ACTIVE | TY_ONESHOT, NULL, 0); // force singleshot bp
}

extc int cdecl Hardbreakpoints(int closeondelete) { ulog(__FUNCTION__, p(closeondelete)) return 0; } // show bps tab

extc int cdecl Sethardwarebreakpoint(ulong addr, int size, int type)
{
    char cmd[MAX_PATH];

    plog(__FUNCTION__, p(addr), p(size), p(type));

    char c_type;
    switch(type)
    {
    case HB_CODE:
        c_type = 'x';
        break;
    case HB_ACCESS:
        c_type = 'r';
        break;
    case HB_WRITE:
        c_type = 'w';
        break;
    default:
        break;
    }

    sprintf_s(cmd, "bph %X,%c,%d", addr, c_type, size); // using command in order to use the size parameter
    DbgCmdExecDirect(cmd);

    return 0;
}

extc int cdecl Deletehardwarebreakpoint(int index)
{
    plog(__FUNCTION__, p(index));

    BPMAP hbp_list;
    DbgGetBpList(bp_hardware, &hbp_list);

    if(hbp_list.count > 0)
        return (int)(Script::Debug::DeleteHardwareBreakpoint(hbp_list.bp[index].addr));

    return -1;
}

extc int cdecl Deletehardwarebreakbyaddr(ulong addr)
{
    plog(__FUNCTION__, p(addr));

    return (int)(Script::Debug::DeleteHardwareBreakpoint(addr));
}

extc int cdecl Setmembreakpoint(int type, ulong addr, ulong size) { ulog(__FUNCTION__, p(type), p(addr), p(size)) return 0; }

extc uchar* cdecl Findhittrace(ulong addr, uchar** ptracecopy, ulong* psize) { ulog(__FUNCTION__, p(addr), p(ptracecopy), p(psize)) return 0; }

extc int cdecl Modifyhittrace(ulong addr0, ulong addr1, int mode) { ulog(__FUNCTION__, p(addr0), p(addr1), p(mode)) return 0; }

extc ulong cdecl Isretaddr(ulong retaddr, ulong* procaddr) { ulog(__FUNCTION__, p(retaddr), p(procaddr)) return 0; }

extc HWND cdecl Creatertracewindow() { ulog(__FUNCTION__) return 0; }

extc void cdecl Settracecondition(char* cond, int onsuspicious, ulong in0, ulong in1, ulong out0, ulong out1) { ulog(__FUNCTION__, p(cond), p(onsuspicious), p(in0), p(in1), p(out0), p(out1)) }

extc void cdecl Settracecount(ulong count) { ulog(__FUNCTION__, p(count)) }

extc void cdecl Settracepauseoncommands(char* cmdset) { ulog(__FUNCTION__, p(cmdset)) }

extc int cdecl Startruntrace(t_reg* preg) { ulog(__FUNCTION__, p(preg)) return 0; }

extc void cdecl Deleteruntrace() { ulog(__FUNCTION__) }

extc int cdecl Runtracesize() { ulog(__FUNCTION__) return 0; }

extc int cdecl Findprevruntraceip(ulong ip, int startback) { ulog(__FUNCTION__, p(ip), p(startback)) return 0; }

extc int cdecl Findnextruntraceip(ulong ip, int startback) { ulog(__FUNCTION__, p(ip), p(startback)) return 0; }

extc int cdecl Getruntraceregisters(int nback, t_reg* preg, t_reg* pold, char* cmd, char* comment) { ulog(__FUNCTION__, p(nback), p(preg), p(pold), p(cmd), p(comment)) return 0; }

extc int cdecl Getruntraceprofile(ulong addr, ulong size, ulong* profile) { ulog(__FUNCTION__, p(addr), p(size), p(profile)) return 0; }

extc void cdecl Scrollruntracewindow(int back) { ulog(__FUNCTION__, p(back)) }

extc HWND cdecl Createprofilewindow(ulong base, ulong size) { ulog(__FUNCTION__, p(base), p(size)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// CODE INJECTION ////////////////////////////////
extc int cdecl Injectcode(ulong threadid, t_inject* inject, char* data, ulong datasize, ulong parm1, ulong parm2, INJECTANSWER* answerfunc) { ulog(__FUNCTION__, p(threadid), p(inject), p(data), p(datasize), p(parm1), p(parm2), p(answerfunc)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// CPU-SPECIFIC FUNCTIONS ////////////////////////////
extc void cdecl Setcpu(ulong threadid, ulong asmaddr, ulong dumpaddr, ulong stackaddr, int mode)
{
    plog(__FUNCTION__, p(threadid), p(asmaddr), p(dumpaddr), p(stackaddr), p(mode));

    if(dumpaddr && DbgMemIsValidReadPtr(dumpaddr))
        DbgCmdExecDirect(StringUtils::sprintf("dump 0x%p", dumpaddr).c_str());

    if(threadid != 0) //switch to thread
        DbgCmdExecDirect(StringUtils::sprintf("switchthread 0x%X", threadid).c_str());
    else
    {
        if(Script::Memory::IsValidPtr(stackaddr))
            DbgCmdExecDirect(StringUtils::sprintf("sdump 0x%p", stackaddr).c_str());
        if(Script::Memory::IsValidPtr(asmaddr))
            DbgCmdExecDirect(StringUtils::sprintf("disasm 0x%p", asmaddr).c_str());
    }

    GuiShowCpu();
    if((mode & CPU_ASMFOCUS) == CPU_ASMFOCUS)
        GuiFocusView(GUI_DISASSEMBLY);
    else if((mode & CPU_ASMFOCUS) == CPU_ASMFOCUS)
        GuiFocusView(GUI_DUMP);
    else if((mode & CPU_ASMFOCUS) == CPU_ASMFOCUS)
        GuiFocusView(GUI_STACK);
}

extc void cdecl Setdisasm(ulong asmaddr, ulong selsize, int mode) { ulog(__FUNCTION__, p(asmaddr), p(selsize), p(mode)) }

extc void cdecl Redrawdisassembler() { ulog(__FUNCTION__) }

extc void cdecl Getdisassemblerrange(ulong* pbase, ulong* psize)
{
    *pbase = DbgMemFindBaseAddr(Eval("cip"), psize);
    plog(__FUNCTION__, p(*pbase), p(*psize));
}

extc ulong cdecl Findprocbegin(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc ulong cdecl Findprocend(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc ulong cdecl Findprevproc(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc ulong cdecl Findnextproc(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc int cdecl Getproclimits(ulong addr, ulong* start, ulong* end) { ulog(__FUNCTION__, p(addr), p(start), p(end)) return 0; }

extc void cdecl Sendshortcut(int where, ulong addr, int msg, int ctrl, int shift, int vkcode) { ulog(__FUNCTION__, p(where), p(addr), p(msg), p(ctrl), p(shift), p(vkcode)) }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// PROCESS CONTROL ////////////////////////////////
extc t_status cdecl Getstatus() { ulog(__FUNCTION__) return STAT_NONE; }

extc int cdecl Go(ulong threadid, ulong tilladdr, int stepmode, int givechance, int backupregs) { ulog(__FUNCTION__, p(threadid), p(tilladdr), p(stepmode), p(givechance), p(backupregs)) return 0; }

extc int cdecl Suspendprocess(int processevents) { ulog(__FUNCTION__, p(processevents)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// DECODING OF ARGUMENTS /////////////////////////////
extc uchar* cdecl Findknownfunction(ulong addr, int direct, int level, char* fname) { ulog(__FUNCTION__, p(addr), p(direct), p(level), p(fname)) return 0; }

extc int cdecl Decodeknownargument(ulong addr, uchar* arg, ulong value, int valid, char* s, char* mask, uchar* pset[]) { ulog(__FUNCTION__, p(addr), p(arg), p(value), p(valid), p(s), p(mask), p(pset)) return 0; }

extc char cdecl* Findunknownfunction(ulong ip, char* code, char* dec, ulong size, char* fname) { ulog(__FUNCTION__, p(ip), p(code), p(dec), p(size), p(fname)) return 0; }

extc int cdecl Decodeascii(ulong value, char* s, int len, int mode) { ulog(__FUNCTION__, p(value), p(s), p(len), p(mode)) return 0; }

extc int cdecl Decodeunicode(ulong value, char* s, int len) { ulog(__FUNCTION__, p(value), p(s), p(len)) return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// SOURCE CODE SUPPORT //////////////////////////////
extc HWND cdecl Showsourcefromaddress(ulong addr, int show) { ulog(__FUNCTION__, p(addr), p(show)) return 0; }

extc int cdecl Getresourcestring(t_module* pm, ulong id, char* s) { ulog(__FUNCTION__, p(pm), p(id), p(s)) return 0; }

extc t_sourceline* cdecl Getlinefromaddress(ulong addr) { ulog(__FUNCTION__, p(addr)) return 0; }

extc ulong cdecl Getaddressfromline(ulong addr0, ulong addr1, char* path, ulong line) { ulog(__FUNCTION__, p(addr0), p(addr1), p(path), p(line)) return 0; }

extc int cdecl Getsourcefilelimits(ulong nameaddr, ulong* addr0, ulong* addr1) { ulog(__FUNCTION__, p(nameaddr), p(addr0), p(addr1)) return 0; }

extc int cdecl Decodefullvarname(t_module* pmod, t_symvar* psym, int offset, char* name) { ulog(__FUNCTION__, p(pmod), p(psym), p(offset), p(name)) return 0; }

extc int cdecl Getbprelname(t_module* pmod, ulong addr, long offset, char* s, int nsymb) { ulog(__FUNCTION__, p(pmod), p(addr), p(offset), p(s), p(nsymb)) return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// WATCH SUPPORT /////////////////////////////////
extc HWND cdecl Createwatchwindow() { ulog(__FUNCTION__) return 0; }

extc int cdecl Deletewatch(int indexone) { ulog(__FUNCTION__, p(indexone)) return 0; }

extc int cdecl Insertwatch(int indexone, char* text) { ulog(__FUNCTION__, p(indexone), p(text)) return 0; }

extc int cdecl Getwatch(int indexone, char* text) { ulog(__FUNCTION__, p(indexone), p(text)) return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// WINDOWS-SPECIFIC FUNCTIONS //////////////////////////
extc HWND cdecl Createwinwindow() { ulog(__FUNCTION__) return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// PATCHES ////////////////////////////////////
extc HWND cdecl Createpatchwindow() { ulog(__FUNCTION__) return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// PLUGIN-SPECIFIC FUNCTIONS ///////////////////////////
extc int cdecl Registerpluginclass(char* classname, char* iconname, HINSTANCE dllinst, WNDPROC classproc)
{
    plog(__FUNCTION__, p(classname), p(iconname), p(dllinst), p(classproc));
    if(classname && classproc)
    {
        static int id = 0;
        sprintf_s(classname, 32, "OT_PLUGIN_%04i", id);
        id++;

        WNDCLASSA WndClass = { 0 };
        WndClass.style = 11;
        WndClass.lpfnWndProc = classproc;
        WndClass.cbClsExtra = 0;
        WndClass.cbWndExtra = 32;
        WndClass.hInstance = hInstMain;
        if(iconname && *iconname)
            WndClass.hIcon = LoadIconA(dllinst, iconname);
        /*else
            WndClass.hIcon = LoadIconA(hInstance, "ICO_PLUGIN");*/
        WndClass.hCursor = 0;
        WndClass.hbrBackground = 0;
        WndClass.lpszMenuName = 0;
        WndClass.lpszClassName = classname;
        if(RegisterClassA(&WndClass))
            return 0;
        else
            *classname = '\0';
    }
    return -1;
}

extc void cdecl Unregisterpluginclass(char* classname)
{
    plog(__FUNCTION__, p(classname));

    if(classname && *classname)
        UnregisterClassA(classname, hInstMain);
}

extc int cdecl Pluginwriteinttoini(HINSTANCE dllinst, char* key, int value)
{
    plog(__FUNCTION__, p(dllinst), p(key), p(value));
    return BridgeSettingSetUint(sectionFromHinst(dllinst).c_str(), key, value) ? 1 : 0;
}

extc int cdecl Pluginwritestringtoini(HINSTANCE dllinst, char* key, char* s)
{
    plog(__FUNCTION__, p(dllinst), p(key), p(s));
    return BridgeSettingSet(sectionFromHinst(dllinst).c_str(), key, s) ? 1 : 0;
    return 0;
}

extc int cdecl Pluginreadintfromini(HINSTANCE dllinst, char* key, int def)
{
    plog(__FUNCTION__, p(dllinst), p(key), p(def));
    duint setting = 0;
    auto section = sectionFromHinst(dllinst);
    if(!BridgeSettingGetUint(section.c_str(), key, &setting))
    {
        setting = def;
        BridgeSettingSetUint(section.c_str(), key, setting);
    }
    return setting;
}

extc int cdecl Pluginreadstringfromini(HINSTANCE dllinst, char* key, char* s, char* def)
{
    plog(__FUNCTION__, p(dllinst), p(key), p(s), p(def));
    auto section = sectionFromHinst(dllinst);
    if(!BridgeSettingGet(section.c_str(), key, s))
    {
        strcpy_s(s, TEXTLEN, def);
        BridgeSettingSet(section.c_str(), key, s);
    }
    return strlen(s);
}

extc int cdecl Plugingetvalue(int type)
{
    static char szProcessName[MAX_PATH];

    plog(__FUNCTION__, p(type));
    switch(type)
    {
    case VAL_HINST: // Current program instance
        return int(hInstMain);
    case VAL_HWMAIN: // Handle of the main window
        return int(hwndOlly);
    case VAL_HWCLIENT: // Handle of the MDI client window
        oputs("UNIMPLEMENTED: VAL_HWCLIENT");
        break;
    case VAL_NCOLORS: // Number of common colors
        oputs("UNIMPLEMENTED: VAL_NCOLORS");
        break;
    case VAL_COLORS: // RGB values of common colors
        oputs("UNIMPLEMENTED: VAL_COLORS");
        break;
    case VAL_BRUSHES: // Handles of common color brushes
        oputs("UNIMPLEMENTED: VAL_BRUSHES");
        break;
    case VAL_PENS: // Handles of common color pens
        oputs("UNIMPLEMENTED: VAL_PENS");
        break;
    case VAL_NFONTS: // Number of common fonts
        oputs("UNIMPLEMENTED: VAL_NFONTS");
        break;
    case VAL_FONTS: // Handles of common fonts
        oputs("UNIMPLEMENTED: VAL_FONTS");
        break;
    case VAL_FONTNAMES: // Internal font names
        oputs("UNIMPLEMENTED: VAL_FONTNAMES");
        break;
    case VAL_FONTWIDTHS: // Average widths of common fonts
        oputs("UNIMPLEMENTED: VAL_FONTWIDTHS");
        break;
    case VAL_FONTHEIGHTS: // Average heigths of common fonts
        oputs("UNIMPLEMENTED: VAL_FONTHEIGHTS");
        break;
    case VAL_NFIXFONTS: // Actual number of fixed-pitch fonts
        oputs("UNIMPLEMENTED: VAL_NFIXFONTS");
        break;
    case VAL_DEFFONT: // Index of default font
        oputs("UNIMPLEMENTED: VAL_DEFFONT");
        break;
    case VAL_NSCHEMES: // Number of color schemes
        oputs("UNIMPLEMENTED: VAL_NSCHEMES");
        break;
    case VAL_SCHEMES: // Color schemes
        oputs("UNIMPLEMENTED: VAL_SCHEMES");
        break;
    case VAL_DEFSCHEME: // Index of default colour scheme
        oputs("UNIMPLEMENTED: VAL_DEFSCHEME");
        break;
    case VAL_DEFHSCROLL: // Default horizontal scroll
        oputs("UNIMPLEMENTED: VAL_DEFHSCROLL");
        break;
    case VAL_RESTOREWINDOWPOS: // Restore window positions from .ini
        oputs("UNIMPLEMENTED: VAL_RESTOREWINDOWPOS");
        break;
    case VAL_HPROCESS: // Handle of Debuggee
        return int(DbgGetProcessHandle());
    case VAL_PROCESSID: // Process ID of Debuggee
        return DbgGetProcessId();
    case VAL_HMAINTHREAD: // Handle of main thread
        return int(DbgGetThreadHandle());
    case VAL_MAINTHREADID: // Thread ID of main thread
        return DbgGetThreadId();
    case VAL_MAINBASE: // Base of main module in the process
        return Script::Module::GetMainModuleBase();
        break;
    case VAL_PROCESSNAME: // Name of the active process
        Script::Module::GetMainModuleName(szProcessName);
        return (int)szProcessName;
        break;
    case VAL_EXEFILENAME: // Name of the main debugged file
        oputs("UNIMPLEMENTED: VAL_EXEFILENAME");
        break;
    case VAL_CURRENTDIR: // Current directory for debugged process
        oputs("UNIMPLEMENTED: VAL_CURRENTDIR");
        break;
    case VAL_SYSTEMDIR: // Windows system directory
        oputs("UNIMPLEMENTED: VAL_SYSTEMDIR");
        break;
    case VAL_DECODEANYIP: // Decode registers dependless on EIP
        oputs("UNIMPLEMENTED: VAL_DECODEANYIP");
        break;
    case VAL_PASCALSTRINGS: // Decode Pascal-style string constants
        oputs("UNIMPLEMENTED: VAL_PASCALSTRINGS");
        break;
    case VAL_ONLYASCII: // Only printable ASCII chars in dump
        oputs("UNIMPLEMENTED: VAL_ONLYASCII");
        break;
    case VAL_DIACRITICALS: // Allow diacritical symbols in strings
        oputs("UNIMPLEMENTED: VAL_DIACRITICALS");
        break;
    case VAL_GLOBALSEARCH: // Search from the beginning of block
        oputs("UNIMPLEMENTED: VAL_GLOBALSEARCH");
        break;
    case VAL_ALIGNEDSEARCH: // Search aligned to item's size
        oputs("UNIMPLEMENTED: VAL_ALIGNEDSEARCH");
        break;
    case VAL_IGNORECASE: // Ignore case in string search
        oputs("UNIMPLEMENTED: VAL_IGNORECASE");
        break;
    case VAL_SEARCHMARGIN: // Floating search allows error margin
        oputs("UNIMPLEMENTED: VAL_SEARCHMARGIN");
        break;
    case VAL_KEEPSELSIZE: // Keep size of hex edit selection
        oputs("UNIMPLEMENTED: VAL_KEEPSELSIZE");
        break;
    case VAL_MMXDISPLAY: // MMX display mode in dialog
        oputs("UNIMPLEMENTED: VAL_MMXDISPLAY");
        break;
    case VAL_WINDOWFONT: // Use calling window's font in dialog
        oputs("UNIMPLEMENTED: VAL_WINDOWFONT");
        break;
    case VAL_TABSTOPS: // Distance between tab stops
        oputs("UNIMPLEMENTED: VAL_TABSTOPS");
        break;
    case VAL_MODULES: // Table of modules (.EXE and .DLL)
        oputs("UNIMPLEMENTED: VAL_MODULES");
        break;
    case VAL_MEMORY: // Table of allocated memory blocks
        oputs("UNIMPLEMENTED: VAL_MEMORY");
        break;
    case VAL_THREADS: // Table of active threads
        oputs("UNIMPLEMENTED: VAL_THREADS");
        break;
    case VAL_BREAKPOINTS: // Table of active breakpoints
        oputs("UNIMPLEMENTED: VAL_BREAKPOINTS");
        break;
    case VAL_REFERENCES: // Table with found references
        oputs("UNIMPLEMENTED: VAL_REFERENCES");
        break;
    case VAL_SOURCELIST: // Table of source files
        oputs("UNIMPLEMENTED: VAL_SOURCELIST");
        break;
    case VAL_WATCHES: // Table of watches
        oputs("UNIMPLEMENTED: VAL_WATCHES");
        break;
    case VAL_CPUFEATURES: // CPU feature bits
        oputs("UNIMPLEMENTED: VAL_CPUFEATURES");
        break;
    case VAL_TRACEFILE: // Handle of run trace log file
        oputs("UNIMPLEMENTED: VAL_TRACEFILE");
        break;
    case VAL_ALIGNDIALOGS: // Whether to align dialogs
        oputs("UNIMPLEMENTED: VAL_ALIGNDIALOGS");
        break;
    case VAL_CPUDASM: // Dump descriptor of CPU Disassembler
        oputs("UNIMPLEMENTED: VAL_CPUDASM");
        break;
    case VAL_CPUDDUMP: // Dump descriptor of CPU Dump
        oputs("UNIMPLEMENTED: VAL_CPUDDUMP");
        break;
    case VAL_CPUDSTACK: // Dump descriptor of CPU Stack
        oputs("UNIMPLEMENTED: VAL_CPUDSTACK");
        break;
    case VAL_APIHELP: // Name of selected API help file
        oputs("UNIMPLEMENTED: VAL_APIHELP");
        break;
    case VAL_HARDBP: // Whether hardware breakpoints enabled
        oputs("UNIMPLEMENTED: VAL_HARDBP");
        break;
    case VAL_PATCHES: // Table of patches
        oputs("UNIMPLEMENTED: VAL_PATCHES");
        break;
    case VAL_HINTS: // Sorted data with analysis hints
        oputs("UNIMPLEMENTED: VAL_HINTS");
        break;
    default:
        __debugbreak();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// UNDOCUMENTED FUNCTIONS /////////////////////////////
void __GetExceptDLLinfo(void*) { }
unsigned int ___CPPdebugHook = 0;