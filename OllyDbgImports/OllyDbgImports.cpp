#include <windows.h>
#include <stdio.h>
#include <vector>

const char* imports[] =
{
    "_Addsorteddata",
    "_Addtolist",
    "_Analysecode",
    "_Assemble",
    "_Broadcast",
    "_Browsefilename",
    "_Calculatecrc",
    "_Checkcondition",
    "_Compress",
    "_Createdumpwindow",
    "_Createlistwindow",
    "_Createsorteddata",
    "_Decodeaddress",
    "_Decodecharacter",
    "_Decodefullvarname",
    "_Decodeknownargument",
    "_Decodename",
    "_Decoderange",
    "_Decoderelativeoffset",
    "_Decodethreadname",
    "_Decompress",
    "_Defaultbar",
    "_Deletebreakpoints",
    "_Deletehardwarebreakpoint",
    "_Deletenamerange",
    "_Deletenonconfirmedsorteddata",
    "_Deletesorteddata",
    "_Deletesorteddatarange",
    "_Demanglename",
    "_Destroysorteddata",
    "_Disasm",
    "_Disassembleback",
    "_Disassembleforward",
    "_Discardquicknames",
    "_Error",
    "_Expression",
    "_Findallcommands",
    "_Finddecode",
    "_Findfileoffset",
    "_Findfixup",
    "_Findimportbyname",
    "_Findlabel",
    "_Findlabelbyname",
    "_Findmemory",
    "_Findmodule",
    "_Findname",
    "_Findnextname",
    "_Findreferences",
    "_Findsorteddata",
    "_Findsorteddataindex",
    "_Findsorteddatarange",
    "_Findstrings",
    "_Findthread",
    "_Flash",
    "_Get3dnow",
    "_Getaddressfromline",
    "_Getasmfindmodel",
    "_Getbprelname",
    "_Getbreakpointtype",
    "_Getcputhreadid",
    "_Getdisassemblerrange",
    "_Getfloat",
    "_Getfloat10",
    "_Gethexstring",
    "_Getline",
    "_Getlinefromaddress",
    "_Getlong",
    "_Getmmx",
    "_Getnextbreakpoint",
    "_Getresourcestring",
    "_Getsortedbyselection",
    "_Getsourcefilelimits",
    "_Getstatus",
    "_Gettext",
    "_Go",
    "_Guardmemory",
    "_Havecopyofmemory",
    "_Infoline",
    "_Insertname",
    "_Isretaddr",
    "_IstextA",
    "_IstextW",
    "_Manualbreakpoint",
    "_Mergequicknames",
    "_Message",
    "_Newtablewindow",
    "_Painttable",
    "_Plugingetvalue",
    "_Pluginreadintfromini",
    "_Pluginreadstringfromini",
    "_Pluginsaverecord",
    "_Pluginwriteinttoini",
    "_Pluginwritestringtoini",
    "_Print3dnow",
    "_Printfloat10",
    "_Printfloat4",
    "_Printfloat8",
    "_Progress",
    "_Quickinsertname",
    "_Quicktablewindow",
    "_Readmemory",
    "_Redrawdisassembler",
    "_Registerotclass",
    "_Registerpluginclass",
    "_Selectandscroll",
    "_Setbreakpoint",
    "_Setcpu",
    "_Sethardwarebreakpoint",
    "_Setmembreakpoint",
    "_Showsourcefromaddress",
    "_Sortsorteddata",
    "_Suspendprocess",
    "_Tablefunction",
    "_Unregisterpluginclass",
    "_Updatelist",
    "_Walkreference",
    "_Writememory",
    "_Findhittrace",
    "_Findnextruntraceip",
    "_Findprevruntraceip",
    "_Getruntraceprofile",
    "_Getruntraceregisters",
    "_Modifyhittrace",
    "_Runtracesize",
    "_Scrollruntracewindow",
    "_Createprofilewindow",
    "_Decodeascii",
    "_Decodeunicode",
    "_Deleteruntrace",
    "_Deletewatch",
    "_Findallsequences",
    "_Findnextproc",
    "_Findprevproc",
    "_Findprocbegin",
    "_Findprocend",
    "_Findsymbolicname",
    "_Findunknownfunction",
    "_Get3dnowxy",
    "_Getasmfindmodelxy",
    "_Getfloat10xy",
    "_Getfloatxy",
    "_Gethexstringxy",
    "_Getlinexy",
    "_Getlongxy",
    "_Getmmxxy",
    "_Getoriginaldatasize",
    "_Getproclimits",
    "_Gettableselectionxy",
    "_Gettextxy",
    "_Getwatch",
    "_Injectcode",
    "_Insertwatch",
    "_Isfilling",
    "_Issuspicious",
    "_OpenEXEfile",
    "_Printsse",
    "_Readcommand",
    "_Restoreallthreads",
    "_Runsinglethread",
    "_Setdisasm",
    "_Startruntrace",
    "_Stringtotext",
    "_Walkreferenceex",
    "_Animate",
    "_Creatertracewindow",
    "_Createthreadwindow",
    "_Createwatchwindow",
    "_Createwinwindow",
    "_Deletehardwarebreakbyaddr",
    "_Dumpbackup",
    "_Hardbreakpoints",
    "_Sendshortcut",
    "_Setdumptype",
    "_Settracecondition",
    "_Findalldllcalls",
    "_Followcall",
    "_Getregxy",
    "_Isprefix",
    "_Tempbreakpoint",
    "_Attachtoactiveprocess",
    "_Createpatchwindow",
    "_Settracecount",
    "_Settracepauseoncommands",
    "_Getbreakpointtypecount",
    "_Setbreakpointext",
    "_Listmemory",
    "__GetExceptDLLinfo",
    "___CPPdebugHook"
};

int error(const char* text = "")
{
    printf("error: %s\n", text);
    return 1;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
    if(argc < 2)
    {
        puts("usage: OllyDbgImports plugin.dll");
        return 1;
    }

    //read the file
    auto hFile = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile == INVALID_HANDLE_VALUE)
        return error("CreateFile");

    //map the file
    auto hMappedFile = CreateFileMappingA(hFile, nullptr, PAGE_READONLY | SEC_IMAGE, 0, 0, nullptr); //notice SEC_IMAGE
    if(!hMappedFile)
        return error("CreateFileMappingA");

    //map the sections appropriately
    auto fileMap = MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 0);
    if(!fileMap)
        return error("MapViewOfFile");

    auto pidh = PIMAGE_DOS_HEADER(fileMap);
    if(pidh->e_magic != IMAGE_DOS_SIGNATURE)
        return error("IMAGE_DOS_SIGNATURE");

    auto pnth = PIMAGE_NT_HEADERS(ULONG_PTR(fileMap) + pidh->e_lfanew);
    if(pnth->Signature != IMAGE_NT_SIGNATURE)
        return error("IMAGE_NT_SIGNATURE");

    if(pnth->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return error("IMAGE_FILE_MACHINE_I386");

    if(pnth->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        return error("IMAGE_NT_OPTIONAL_HDR_MAGIC");

    auto importDir = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if(!importDir.VirtualAddress || !importDir.Size)
        return error("No Import directory!");

    std::vector<std::string> ollyImports;

    auto importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(ULONG_PTR(fileMap) + importDir.VirtualAddress);
    if(!IsBadReadPtr((char*)fileMap + importDir.VirtualAddress, 1))
    {
        for(; importDescriptor->FirstThunk; importDescriptor++)
        {
            auto name = (char*)fileMap + importDescriptor->Name;
            auto isOllyDbg = !IsBadReadPtr(name, 1) && !_strnicmp(name, "ollydbg", 7);

            auto thunkData = PIMAGE_THUNK_DATA(ULONG_PTR(fileMap) + importDescriptor->FirstThunk);
            for(; thunkData->u1.AddressOfData; thunkData++)
            {
                auto rva = ULONG_PTR(thunkData) - ULONG_PTR(fileMap);

                auto data = thunkData->u1.AddressOfData;
                if((data & IMAGE_ORDINAL_FLAG) == IMAGE_ORDINAL_FLAG && isOllyDbg)
                {
                    auto ordinal = data & ~IMAGE_ORDINAL_FLAG;
                    ollyImports.push_back(ordinal - 1 < _countof(imports) ? imports[ordinal - 1] : "UNKNOWN");
                }
                else
                {
                    auto importByName = PIMAGE_IMPORT_BY_NAME(ULONG_PTR(fileMap) + data);
                    if(!IsBadReadPtr(importByName, 1) && isOllyDbg)
                        ollyImports.push_back((char*)importByName->Name);
                }
            }
        }
    }
    else
        puts("INVALID IMPORT DESCRIPTOR");

    printf("\n%d OllyDbg import(s) found\n\n", ollyImports.size());
    for(auto & name : ollyImports)
        puts(name.c_str());

    return 0;
}