#pragma once

#include <windows.h>

struct PeData
{
    bool appearsToBeValid = false; //got past the IMAGE_FILE_MACHINE_I386 check
    bool isDll = false;
    DWORD imagebase = 0;
    DWORD codebase = 0;
    DWORD codesize = 0;
    DWORD resbase = 0;
    DWORD ressize = 0;
};

/// <summary>
/// This function will do a best-effort to retrieve data from a PE file.
/// </summary>
/// <param name="szFileName">Filename to parse.</param>
/// <param name="data">Output data, it will do a best-effort to retrieve all data.</param>
void HackyParsePe(const wchar_t* szFileName, PeData & data);