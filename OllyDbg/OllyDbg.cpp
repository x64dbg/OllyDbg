#include <windows.h>
#define _CHAR_UNSIGNED //TODO: this is probably harmful for the implementation
#include "OllyDbg.h"
#include "x64dbg.h"

//NOTE: SEG_XXX are overloaded. The OllyDbg ones are renamed to ODBG_SEG_XXX

#define dprintf _plugin_logprintf
#define dputs _plugin_logputs

#define log(x, ...) dprintf("[" PLUGIN_NAME "] " x, __VA_ARGS__)
#define fwdlog(function, ...) \
    { \
        log("%s(\n", function); /* log the function name */\
        logArgs(__VA_ARGS__); /* log the function arguments */\
        log(") = UNIMPLEMENTED\n"); \
    }

static inline void processArg(ULONG_PTR arg)
{
    log("  0x%p\n", arg);
}

//no arguments base case
static inline void logArgs()
{
}

//template takes one argument (Arg) and zero or more extra arguments (Args)
template<typename Arg, typename... Args>
static inline void logArgs(Arg a1, Args... args)
{
    processArg(ULONG_PTR(a1)); //call processArg on the current (first) argument
    logArgs(args...); //expand the additional arguments -> logArgs(a2, a3, a4) where args = { a2, a3, a4 }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// INFORMATION FUNCTIONS /////////////////////////////
extc void cdecl Addtolist(long addr, int highlight, char* format, ...) { }

extc void cdecl Updatelist() { }

extc HWND cdecl Createlistwindow() { return 0; }

extc void cdecl Error(char* format, ...) { }

extc void cdecl Message(ulong addr, char* format, ...) { }

extc void cdecl Infoline(char* format, ...) { }

extc void cdecl Progress(int promille, char* format, ...) { }

extc void cdecl Flash(char* format, ...) { }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// DATA FORMATTING FUNCTIONS ///////////////////////////
extc int cdecl Decodeaddress(ulong addr, ulong base, int addrmode, char* symb, int nsymb, char* comment) { return 0; }

extc int cdecl Decoderelativeoffset(ulong addr, int addrmode, char* symb, int nsymb) { return 0; }

extc int cdecl Decodecharacter(char* s, uint c) { return 0; }

extc int cdecl Printfloat4(char* s, float f) { return 0; }

extc int cdecl Printfloat8(char* s, double d) { return 0; }

extc int cdecl Printfloat10(char* s, long double ext) { return 0; }

extc int cdecl Print3dnow(char* s, uchar* f) { return 0; }

extc int cdecl Printsse(char* s, char* f) { return 0; }

extc ulong cdecl Followcall(ulong addr) { return 0; }

extc int cdecl IstextA(char c) { return 0; }

extc int cdecl IstextW(wchar_t w) { return 0; }

extc int cdecl Stringtotext(char* data, int ndata, char* text, int ntext) { return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// DATA INPUT FUNCTIONS /////////////////////////////
extc int cdecl Getlong(char* title, ulong* data, int datasize, char letter, int mode) { return 0; }

extc int cdecl Getlongxy(char* title, ulong* data, int datasize, char letter, int mode, int x, int y) { return 0; }

extc int cdecl Getregxy(char* title, ulong* data, char letter, int x, int y) { return 0; }

extc int cdecl Getline(char* title, ulong* data) { return 0; }

extc int cdecl Getlinexy(char* title, ulong* data, int x, int y) { return 0; }

extc int cdecl Getfloat10(char* title, long double* fdata, uchar* tag, char letter, int mode) { return 0; }

extc int cdecl Getfloat10xy(char* title, long double* fdata, char* tag, char letter, int mode, int x, int y) { return 0; }

extc int cdecl Getfloat(char* title, void* fdata, int size, char letter, int mode) { return 0; }

extc int cdecl Getfloatxy(char* title, void* fdata, int size, char letter, int mode, int x, int y) { return 0; }

extc void cdecl Getasmfindmodel(t_asmmodel model[NMODELS], char letter, int searchall) { }

extc void cdecl Getasmfindmodelxy(t_asmmodel model[NMODELS], char letter, int searchall, int x, int y) { }

extc int cdecl Gettext(char* title, char* text, char letter, int type, int fontindex) { return 0; }

extc int cdecl Gettextxy(char* title, char* text, char letter, int type, int fontindex, int x, int y) { return 0; }

extc int cdecl Gethexstring(char* title, t_hexstr* hs, int mode, int fontindex, char letter) { return 0; }

extc int cdecl Gethexstringxy(char* title, t_hexstr* hs, int mode, int fontindex, char letter, int x, int y) { return 0; }

extc int cdecl Getmmx(char* title, uchar* data, int mode) { return 0; }

extc int cdecl Getmmxxy(char* title, char* data, int mode, int x, int y) { return 0; }

extc int cdecl Get3dnow(char* title, uchar* data, int mode) { return 0; }

extc int cdecl Get3dnowxy(char* title, char* data, int mode, int x, int y) { return 0; }

extc int cdecl Browsefilename(char* title, char* name, char* defext, int getarguments) { return 0; }

extc int cdecl OpenEXEfile(char* path, int dropped) { return 0; }

extc int cdecl Attachtoactiveprocess(int newprocessid) { return 0; }

extc void cdecl Animate(int animation) { }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// SORTED DATA FUNCTIONS /////////////////////////////
extc int cdecl Createsorteddata(t_sorted* sd, char* name, int itemsize, int nmax, SORTFUNC* sortfunc, DESTFUNC* destfunc) { return 0; }

extc void cdecl Destroysorteddata(t_sorted* sd) { }

extc void* cdecl Addsorteddata(t_sorted* sd, void* item) { return 0; }

extc void cdecl Deletesorteddata(t_sorted* sd, ulong addr) { }

extc void cdecl Deletesorteddatarange(t_sorted* sd, ulong addr0, ulong addr1) { }

extc int cdecl Deletenonconfirmedsorteddata(t_sorted* sd) { return 0; }

extc void* cdecl Findsorteddata(t_sorted* sd, ulong addr) { return 0; }

extc void* cdecl Findsorteddatarange(t_sorted* sd, ulong addr0, ulong addr1) { return 0; }

extc int cdecl Findsorteddataindex(t_sorted* sd, ulong addr0, ulong addr1) { return 0; }

extc int cdecl Sortsorteddata(t_sorted* sd, int sort) { return 0; }

extc void* cdecl Getsortedbyselection(t_sorted* sd, int index) { return 0; }

extc void cdecl Defaultbar(t_bar* pb) { }

extc int cdecl Tablefunction(t_table* pt, HWND hw, UINT msg, WPARAM wp, LPARAM lp) { return 0; }

extc void cdecl Painttable(HWND hw, t_table* pt, DRAWFUNC getline) { }

extc int cdecl Gettableselectionxy(t_table* pt, int column, int* px, int* py) { return 0; }

extc void cdecl Selectandscroll(t_table* pt, int index, int mode) { }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// NAME FUNCTIONS ////////////////////////////////
extc int cdecl Insertname(ulong addr, int type, char* name) { return 0; }

extc int cdecl Quickinsertname(ulong addr, int type, char* name) { return 0; }

extc void cdecl Mergequicknames() { }

extc void cdecl Discardquicknames() { }

extc int cdecl Findname(ulong addr, int type, char* name) { return 0; }

extc int cdecl Decodename(ulong addr, int type, char* name) { return 0; }

extc ulong cdecl Findnextname(char* name) { return 0; }

extc int cdecl Findlabel(ulong addr, char* name) { return 0; }

extc void cdecl Deletenamerange(ulong addr0, ulong addr1, int type) { }

extc int cdecl Findlabelbyname(char* name, ulong* addr, ulong addr0, ulong addr1) { return 0; }

extc ulong cdecl Findimportbyname(char* name, ulong addr0, ulong addr1) { return 0; }

extc int cdecl Demanglename(char* name, int type, char* undecorated) { return 0; }

extc int cdecl Findsymbolicname(ulong addr, char* fname) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// DISASSEMBLY FUNCTIONS /////////////////////////////
extc ulong cdecl Disasm(uchar* src, ulong srcsize, ulong srcip, uchar* srcdec, t_disasm* disasm, int disasmmode, ulong threadid) { return 0; }

extc ulong cdecl Disassembleback(uchar* block, ulong base, ulong size, ulong ip, int n, int usedec) { return 0; }

extc ulong cdecl Disassembleforward(uchar* block, ulong base, ulong size, ulong ip, int n, int usedec) { return 0; }

extc int cdecl Issuspicious(char* cmd, ulong size, ulong ip, ulong threadid, t_reg* preg, char* s) { return 0; }

extc int cdecl Isfilling(ulong offset, char* data, ulong size, ulong align) { return 0; }

extc int cdecl Isprefix(int c) { return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// ASSEMBLY FUNCTIONS //////////////////////////////
extc int cdecl Assemble(char* cmd, ulong ip, t_asmmodel* model, int attempt, int constsize, char* errtext) { return 0; }

extc int cdecl Checkcondition(int code, ulong flags) { return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// EXPRESSIONS //////////////////////////////////
extc int cdecl Expression(t_result* result, char* expression, int a, int b, uchar* data, ulong database, ulong datasize, ulong threadid) { return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// THREAD FUNCTIONS ///////////////////////////////
extc HWND cdecl Createthreadwindow() { return 0; }

extc t_thread* cdecl Findthread(ulong threadid) { return 0; }

extc int cdecl Decodethreadname(char* s, ulong threadid, int mode) { return 0; }

extc ulong cdecl Getcputhreadid() { return 0; }

extc ulong cdecl Runsinglethread(ulong threadid) { return 0; }

extc void cdecl Restoreallthreads() { }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MEMORY FUNCTIONS ///////////////////////////////
extc int cdecl Listmemory() { return 0; }

extc t_memory* cdecl Findmemory(ulong addr) { return 0; }

extc int cdecl Guardmemory(ulong base, ulong size, int guard) { return 0; }

extc void cdecl Havecopyofmemory(uchar* copy, ulong base, ulong size) { }

extc ulong cdecl Readmemory(void* buf, ulong addr, ulong size, int mode) { return 0; }

extc ulong cdecl Writememory(void* buf, ulong addr, ulong size, int mode) { return 0; }

extc ulong cdecl Readcommand(ulong ip, char* cmd) { return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MODULE FUNCTIONS ///////////////////////////////
extc t_module* cdecl Findmodule(ulong addr) { return 0; }

extc t_fixup* cdecl Findfixup(t_module* pmod, ulong addr) { return 0; }

extc uchar* cdecl Finddecode(ulong addr, ulong* psize) { return 0; }

extc ulong cdecl Findfileoffset(t_module* pmod, ulong addr) { return 0; }

extc int cdecl Decoderange(ulong addr, ulong size, char* s) { return 0; }

extc int cdecl Analysecode(t_module* pm) { return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// DUMP /////////////////////////////////////
extc int cdecl Registerotclass(char* classname, char* iconname, WNDPROC classproc) { return 0; }

extc HWND cdecl Newtablewindow(t_table* pt, int nlines, int maxcolumns, char* winclass, char* wintitle) { return 0; }

extc HWND cdecl Quicktablewindow(t_table* pt, int nlines, int maxcolumns, char* winclass, char* wintitle) { return 0; }

extc HWND cdecl Createdumpwindow(char* name, ulong base, ulong size, ulong addr, int type, SPECFUNC* specdump) { return 0; }

extc void cdecl Setdumptype(t_dump* pd, int dumptype) { }

extc void cdecl Dumpbackup(t_dump* pd, int action) { }

extc int cdecl Broadcast(UINT msg, WPARAM wp, LPARAM lp) { return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// DATA CONVERSION FUNCTIONS ///////////////////////////
extc ulong cdecl Compress(uchar* bufin, ulong nbufin, uchar* bufout, ulong nbufout) { return 0; }

extc ulong cdecl Getoriginaldatasize(char* bufin, ulong nbufin) { return 0; }

extc ulong cdecl Decompress(uchar* bufin, ulong nbufin, uchar* bufout, ulong nbufout) { return 0; }

extc ulong cdecl Calculatecrc(uchar* copy, ulong base, ulong size, t_module* pmod, ulong fixupoffset) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// REFERENCES AND SEARCH /////////////////////////////
extc int cdecl Findreferences(ulong base, ulong size, ulong addr0, ulong addr1, ulong origin, int recurseonjump, char* title) { return 0; }

extc int cdecl Findstrings(ulong base, ulong size, ulong origin, char* title) { return 0; }

extc int cdecl Findalldllcalls(t_dump* pd, ulong origin, char* title) { return 0; }

extc int cdecl Findallcommands(t_dump* pd, t_asmmodel* model, ulong origin, char* title) { return 0; }

extc int cdecl Findallsequences(t_dump* pd, t_extmodel model[NSEQ][NMODELS], ulong origin, char* title) { return 0; }

extc ulong cdecl Walkreference(int dir) { return 0; }

extc ulong cdecl Walkreferenceex(int dir, ulong* size) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////// BREAKPOINT AND TRACE FUNCTIONS ////////////////////////
extc int cdecl Setbreakpoint(ulong addr, ulong type, uchar cmd) { return 0; }

extc int cdecl Setbreakpointext(ulong addr, ulong type, char cmd, ulong passcount) { return 0; }

extc int cdecl Manualbreakpoint(ulong addr, int key, int shiftkey, ulong nametype, int font) { return 0; }

extc void cdecl Deletebreakpoints(ulong addr0, ulong addr1, int silent) { }

extc ulong cdecl Getbreakpointtype(ulong addr) { return 0; }

extc ulong cdecl Getbreakpointtypecount(ulong addr, ulong* passcount) { return 0; }

extc ulong cdecl Getnextbreakpoint(ulong addr, ulong* type, int* cmd) { return 0; }

extc void cdecl Tempbreakpoint(ulong addr, int mode) { }

extc int cdecl Hardbreakpoints(int closeondelete) { return 0; }

extc int cdecl Sethardwarebreakpoint(ulong addr, int size, int type) { return 0; }

extc int cdecl Deletehardwarebreakpoint(int index) { return 0; }

extc int cdecl Deletehardwarebreakbyaddr(ulong addr) { return 0; }

extc int cdecl Setmembreakpoint(int type, ulong addr, ulong size) { return 0; }

extc uchar* cdecl Findhittrace(ulong addr, uchar** ptracecopy, ulong* psize) { return 0; }

extc int cdecl Modifyhittrace(ulong addr0, ulong addr1, int mode) { return 0; }

extc ulong cdecl Isretaddr(ulong retaddr, ulong* procaddr) { return 0; }

extc HWND cdecl Creatertracewindow() { return 0; }

extc void cdecl Settracecondition(char* cond, int onsuspicious, ulong in0, ulong in1, ulong out0, ulong out1) { }

extc void cdecl Settracecount(ulong count) { }

extc void cdecl Settracepauseoncommands(char* cmdset) { }

extc int cdecl Startruntrace(t_reg* preg) { return 0; }

extc void cdecl Deleteruntrace() { }

extc int cdecl Runtracesize() { return 0; }

extc int cdecl Findprevruntraceip(ulong ip, int startback) { return 0; }

extc int cdecl Findnextruntraceip(ulong ip, int startback) { return 0; }

extc int cdecl Getruntraceregisters(int nback, t_reg* preg, t_reg* pold, char* cmd, char* comment) { return 0; }

extc int cdecl Getruntraceprofile(ulong addr, ulong size, ulong* profile) { return 0; }

extc void cdecl Scrollruntracewindow(int back) { }

extc HWND cdecl Createprofilewindow(ulong base, ulong size) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// CODE INJECTION ////////////////////////////////
extc int cdecl Injectcode(ulong threadid, t_inject* inject, char* data, ulong datasize, ulong parm1, ulong parm2, INJECTANSWER* answerfunc) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// CPU-SPECIFIC FUNCTIONS ////////////////////////////
extc void cdecl Setcpu(ulong threadid, ulong asmaddr, ulong dumpaddr, ulong stackaddr, int mode) { }

extc void cdecl Setdisasm(ulong asmaddr, ulong selsize, int mode) { }

extc void cdecl Redrawdisassembler() { }

extc void cdecl Getdisassemblerrange(ulong* pbase, ulong* psize) { }

extc ulong cdecl Findprocbegin(ulong addr) { return 0; }

extc ulong cdecl Findprocend(ulong addr) { return 0; }

extc ulong cdecl Findprevproc(ulong addr) { return 0; }

extc ulong cdecl Findnextproc(ulong addr) { return 0; }

extc int cdecl Getproclimits(ulong addr, ulong* start, ulong* end) { return 0; }

extc void cdecl Sendshortcut(int where, ulong addr, int msg, int ctrl, int shift, int vkcode) { }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// PROCESS CONTROL ////////////////////////////////
extc t_status cdecl Getstatus() { return STAT_NONE; }

extc int cdecl Go(ulong threadid, ulong tilladdr, int stepmode, int givechance, int backupregs) { return 0; }

extc int cdecl Suspendprocess(int processevents) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// DECODING OF ARGUMENTS /////////////////////////////
extc uchar* cdecl Findknownfunction(ulong addr, int direct, int level, char* fname) { return 0; }

extc int cdecl Decodeknownargument(ulong addr, uchar* arg, ulong value, int valid, char* s, char* mask, uchar* pset[]) { return 0; }

extc char cdecl* Findunknownfunction(ulong ip, char* code, char* dec, ulong size, char* fname) { return 0; }

extc int cdecl Decodeascii(ulong value, char* s, int len, int mode) { return 0; }

extc int cdecl Decodeunicode(ulong value, char* s, int len) { return 0; }

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// SOURCE CODE SUPPORT //////////////////////////////
extc HWND cdecl Showsourcefromaddress(ulong addr, int show) { return 0; }

extc int cdecl Getresourcestring(t_module* pm, ulong id, char* s) { return 0; }

extc t_sourceline* cdecl Getlinefromaddress(ulong addr) { return 0; }

extc ulong cdecl Getaddressfromline(ulong addr0, ulong addr1, char* path, ulong line) { return 0; }

extc int cdecl Getsourcefilelimits(ulong nameaddr, ulong* addr0, ulong* addr1) { return 0; }

extc int cdecl Decodefullvarname(t_module* pmod, t_symvar* psym, int offset, char* name) { return 0; }

extc int cdecl Getbprelname(t_module* pmod, ulong addr, long offset, char* s, int nsymb) { return 0; }

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// WATCH SUPPORT /////////////////////////////////
extc HWND cdecl Createwatchwindow() { return 0; }

extc int cdecl Deletewatch(int indexone) { return 0; }

extc int cdecl Insertwatch(int indexone, char* text) { return 0; }

extc int cdecl Getwatch(int indexone, char* text) { return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// WINDOWS-SPECIFIC FUNCTIONS //////////////////////////
extc HWND cdecl Createwinwindow() { return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// PATCHES ////////////////////////////////////
extc HWND cdecl Createpatchwindow() { return 0; }

////////////////////////////////////////////////////////////////////////////////
////////////////////////// PLUGIN-SPECIFIC FUNCTIONS ///////////////////////////
extc int cdecl Registerpluginclass(char* classname, char* iconname, HINSTANCE dllinst, WNDPROC classproc) { return 0; }

extc void cdecl Unregisterpluginclass(char* classname) { }

extc int cdecl Pluginwriteinttoini(HINSTANCE dllinst, char* key, int value) { return 0; }

extc int cdecl Pluginwritestringtoini(HINSTANCE dllinst, char* key, char* s) { return 0; }

extc int cdecl Pluginreadintfromini(HINSTANCE dllinst, char* key, int def) { return 0; }

extc int cdecl Pluginreadstringfromini(HINSTANCE dllinst, char* key, char* s, char* def) { return 0; }

extc int cdecl Pluginsaverecord(ulong tag, ulong size, void* data) { return 0; }

extc int cdecl Plugingetvalue(int type) { return 0; }

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// UNDOCUMENTED FUNCTIONS /////////////////////////////
void __GetExceptDLLinfo(void*) { }
unsigned int ___CPPdebugHook = 0;