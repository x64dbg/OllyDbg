#include "HackyPeParser.h"

void HackyParsePe(const wchar_t* szFileName, PeData & data)
{
    memset(&data, 0, sizeof(data));
    auto hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        // SEC_IMAGE will load the file like the Windows loader would, saving us RVA -> File offset conversion crap.
        // NOTE: this WILL rebase the file so it should be used with caution.
        auto hMappedFile = CreateFileMappingW(hFile, nullptr, PAGE_READONLY | SEC_IMAGE, 0, 0, nullptr);
        if(hMappedFile)
        {
            auto fileMap = MapViewOfFile(hMappedFile, FILE_MAP_READ, 0, 0, 0);
            if(fileMap)
            {
                __try
                {
                    auto pidh = PIMAGE_DOS_HEADER(fileMap);
                    if(pidh->e_magic == IMAGE_DOS_SIGNATURE)
                    {
                        auto pnth = PIMAGE_NT_HEADERS32(ULONG_PTR(fileMap) + pidh->e_lfanew);
                        if(pnth->Signature == IMAGE_NT_SIGNATURE)
                        {
                            auto Machine = pnth->FileHeader.Machine;
                            if(Machine == IMAGE_FILE_MACHINE_I386)
                            {
                                // NOTE: try to access fields in the order they appear in memory (just in case the header is cut off)
                                data.isDll = (pnth->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL;
                                data.imagebase = pnth->OptionalHeader.ImageBase;
                                data.codebase = pnth->OptionalHeader.BaseOfCode;
                                data.codesize = pnth->OptionalHeader.SizeOfCode;
                                data.resbase = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
                                data.ressize = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
                            }
                        }
                    }
                }
                __except(EXCEPTION_ACCESS_VIOLATION)
                {
                }
                UnmapViewOfFile(fileMap);
            }
            CloseHandle(hMappedFile);
        }
        CloseHandle(hFile);
    }
}