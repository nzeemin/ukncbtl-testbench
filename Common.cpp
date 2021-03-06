/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// Common.cpp

#include "stdafx.h"
#include "Emulator.h"
#include "util\BitmapFile.h"

//////////////////////////////////////////////////////////////////////

DWORD m_dwTotalEmulatorUptime = 0;  // Total UKNC uptime, seconds
int m_nCommon_TestsStarted = 0;
bool m_okCommon_CurrentTestFailed = false;
int m_nCommon_TestsFailed = 0;


//////////////////////////////////////////////////////////////////////

bool AssertFailedLine(LPCSTR lpszFileName, int nLine)
{
    DebugPrintFormat(_T("ASSERT in %s at line %n"), lpszFileName, nLine);

    return FALSE;
}

void AlertWarning(LPCTSTR sMessage)
{
    Test_Log('*', sMessage);
}
void AlertWarningFormat(LPCTSTR format, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, format);
    _vsntprintf_s(buffer, 512, 512 - 1, format, ptr);
    va_end(ptr);

    Test_Log('*', buffer);
}


//////////////////////////////////////////////////////////////////////
// DebugPrint and DebugLog

#if !defined(PRODUCT)

void DebugPrint(LPCTSTR message)
{
    Test_Log('d', message);
}

void DebugPrintFormat(LPCTSTR format, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, format);
    _vsntprintf_s(buffer, 512, 512 - 1, format, ptr);
    va_end(ptr);

    Test_Log('d', buffer);
}

const LPCTSTR TRACELOG_FILE_NAME = _T("trace.log");
const LPCTSTR TRACELOG_NEWLINE = _T("\r\n");

HANDLE Common_LogFile = NULL;

void DebugLogClear()
{
    if (Common_LogFile != NULL)
    {
        CloseHandle(Common_LogFile);
        Common_LogFile = NULL;
    }

    ::DeleteFile(TRACELOG_FILE_NAME);
}

void DebugLog(LPCTSTR message)
{
    if (Common_LogFile == NULL)
    {
        // Create file
        Common_LogFile = CreateFile(TRACELOG_FILE_NAME,
                GENERIC_WRITE, FILE_SHARE_READ, NULL,
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    SetFilePointer(Common_LogFile, 0, NULL, FILE_END);

    DWORD dwLength = lstrlen(message) * sizeof(TCHAR);

    char ascii[256];  *ascii = 0;
    WideCharToMultiByte(CP_ACP, 0, message, dwLength, ascii, 256, NULL, NULL);

    DWORD dwBytesWritten = 0;
    //WriteFile(Common_LogFile, message, dwLength, &dwBytesWritten, NULL);
    WriteFile(Common_LogFile, ascii, (uint32_t)strlen(ascii), &dwBytesWritten, NULL);
}
void DebugLog(const char * message)
{
    if (Common_LogFile == NULL)
    {
        // Create file
        Common_LogFile = CreateFile(TRACELOG_FILE_NAME,
                GENERIC_WRITE, FILE_SHARE_READ, NULL,
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    SetFilePointer(Common_LogFile, 0, NULL, FILE_END);

    DWORD dwBytesWritten = 0;
    //WriteFile(Common_LogFile, message, dwLength, &dwBytesWritten, NULL);
    WriteFile(Common_LogFile, message, (uint32_t)strlen(message), &dwBytesWritten, NULL);
}

void DebugLogFormat(LPCTSTR pszFormat, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, pszFormat);
    _vsntprintf_s(buffer, 512, 512 - 1, pszFormat, ptr);
    va_end(ptr);

    DebugLog(buffer);
}


#endif // !defined(PRODUCT)


//////////////////////////////////////////////////////////////////////


// �������� ��������� ����������
const TCHAR* REGISTER_NAME[] = { _T("R0"), _T("R1"), _T("R2"), _T("R3"), _T("R4"), _T("R5"), _T("SP"), _T("PC") };


// Print octal 16-bit value to buffer
// buffer size at least 7 characters
void PrintOctalValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 6; p++)
    {
        int digit = value & 7;
        buffer[5 - p] = _T('0') + (TCHAR)digit;
        value = (value >> 3);
    }
    buffer[6] = 0;
}
// Print hex 16-bit value to buffer
// buffer size at least 5 characters
void PrintHexValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 4; p++)
    {
        int digit = value & 15;
        buffer[3 - p] = (digit < 10) ? _T('0') + (TCHAR)digit : _T('a') + (TCHAR)digit - 10;
        value = (value >> 4);
    }
    buffer[4] = 0;
}
// Print binary 16-bit value to buffer
// buffer size at least 17 characters
void PrintBinaryValue(TCHAR* buffer, WORD value)
{
    for (int b = 0; b < 16; b++)
    {
        int bit = (value >> b) & 1;
        buffer[15 - b] = bit ? _T('1') : _T('0');
    }
    buffer[16] = 0;
}


//////////////////////////////////////////////////////////////////////


void Test_Log(char eventtype, LPCTSTR message)
{
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    WORD fgcolor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (eventtype == 'E')
    {
        fgcolor = FOREGROUND_RED | FOREGROUND_INTENSITY;
        m_okCommon_CurrentTestFailed = true;
    }
    else if (eventtype == '!')
        fgcolor = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    else if (eventtype == '*')
        fgcolor = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    //TODO: Show machine uptime
    SYSTEMTIME stm;
    ::GetLocalTime(&stm);
    ::SetConsoleTextAttribute(hStdOut, fgcolor);
    printf("%02d:%02d:%02d.%03d %c %S\n",
           stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds,
           eventtype, message);
    ::SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void Test_LogFormat(char eventtype, LPCTSTR format, ...)
{
    TCHAR buffer[512];

    va_list ptr;
    va_start(ptr, format);
    _vsntprintf_s(buffer, 512, 512 - 1, format, ptr);
    va_end(ptr);

    Test_Log(eventtype, buffer);
}

void Test_Init(LPCTSTR sTestTitle)
{
    Test_Log('!', sTestTitle);

    m_nCommon_TestsStarted++;
    m_okCommon_CurrentTestFailed = false;

    if (! Emulator_Init())
        Test_Log('E', _T("FAILED to initialize the emulator."));
}

void Test_Done()
{
    m_dwTotalEmulatorUptime += Emulator_GetUptime();
    Emulator_Done();

    if (m_okCommon_CurrentTestFailed)
        m_nCommon_TestsFailed++;
}

BOOL Test_LogSummary()
{
    Test_LogFormat('i', _T("Emulator time spent: %u seconds"), m_dwTotalEmulatorUptime);
    char evtype = (m_nCommon_TestsFailed == 0) ? '!' : 'E';
    Test_LogFormat(evtype, _T("TOTAL tests started: %u, failed: %u"), m_nCommon_TestsStarted, m_nCommon_TestsFailed);

    return (m_nCommon_TestsFailed == 0);
}

void Test_LoadROMCartridge(int slot, LPCTSTR sFilePath)
{
    BOOL res = Emulator_LoadROMCartridge(slot, sFilePath);
    if (!res)
        Test_LogError(_T("Failed to load ROM cartridge image."));
}

void Test_AttachFloppyImage(int slot, LPCTSTR sFilePath)
{
    BOOL res = Emulator_AttachFloppyImage(slot, sFilePath);
    if (!res)
        Test_LogFormat('E', _T("FAILED to attach floppy image %s"), sFilePath);
}

void Test_AttachHardImage(int slot, LPCTSTR sFilePath)
{
    BOOL res = Emulator_AttachHardImage(slot, sFilePath);
    if (!res)
        Test_LogFormat('E', _T("FAILED to attach HDD image %s"), sFilePath);
}

void Test_CreateHardImage(BYTE sectors, BYTE heads, int cylinders, LPCTSTR sFileName)
{
    LONG fileSize = (LONG)sectors * (LONG)heads * (LONG)cylinders * (LONG)512;
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Test_LogFormat('E', _T("FAILED to create HDD image %s"), sFileName);
        return;
    }

    // Zero-fill the file
    ::SetFilePointer(hFile, fileSize, NULL, FILE_BEGIN);
    ::SetEndOfFile(hFile);

    // Write sectors and heads values
    ::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    DWORD dwBytesWritten;
    ::WriteFile(hFile, &sectors, 1, &dwBytesWritten, NULL);
    ::WriteFile(hFile, &heads, 1, &dwBytesWritten, NULL);

    ::CloseHandle(hFile);
}

void Test_OpenTape(LPCTSTR sFilePath)
{
    BOOL res = Emulator_OpenTape(sFilePath);
    if (!res)
        Test_LogFormat('E', _T("FAILED to open tape image %s"), sFilePath);
}
void Test_CreateTape(LPCTSTR sFilePath)
{
    BOOL res = Emulator_CreateTape(sFilePath);
    if (!res)
        Test_LogFormat('E', _T("FAILED to create tape image %s"), sFilePath);
}
void Test_CloseTape()
{
    Emulator_CloseTape();
}

void Test_SaveScreenshot(LPCTSTR sFileName)
{
    if (Emulator_SaveScreenshot(sFileName))
        Test_LogFormat('*', _T("Saved screenshot %s"), sFileName);
    else
        Test_LogFormat('E', _T("FAILED to save screenshot %s"), sFileName);
}

void Test_SaveScreenshotSeria(LPCTSTR sFileNameTemplate, int count, int frameStep)
{
    TCHAR buffer[255];
    for (int i = 0; i < count; i++)
    {
        swprintf(buffer, 255, sFileNameTemplate, i);
        Test_SaveScreenshot(buffer);
        Emulator_Run(frameStep);
    }
}

void Test_CheckScreenshot(LPCTSTR sFileName)
{
    int diff = Emulator_CheckScreenshot(sFileName);
    if (diff == 0)
    {
        Test_LogFormat('i', _T("Checked screenshot %s"), sFileName);
        return;
    }
    if (diff < 0)
    {
        Test_LogFormat('E', _T("ERROR checking screenshot %s"), sFileName);
        exit(1);
    }

    Test_LogFormat('E', _T("TEST FAILED checking screenshot %s, diff %d"), sFileName, diff);
}

void Test_CopyFile(LPCTSTR sFileNameFrom, LPCTSTR sFileNameTo)
{
    if (!CopyFile(sFileNameFrom, sFileNameTo, FALSE))
    {
        Test_LogFormat('E', _T("ERROR copying file %s to %s"), sFileNameFrom, sFileNameTo);
        exit(1);
    }

    Test_LogFormat('i', _T("Copyed file %s to %s"), sFileNameFrom, sFileNameTo);
}

void Test_CreateDiskImage(LPCTSTR sFileName, int tracks)
{
    LONG fileSize = tracks * 10240;
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Test_LogFormat('E', _T("FAILED to create disk image %s (%ld)"), sFileName, GetLastError());
        exit(1);
    }

    // Zero-fill the file
    ::SetFilePointer(hFile, fileSize, NULL, FILE_BEGIN);
    ::SetEndOfFile(hFile);
    ::CloseHandle(hFile);
}

void Test_SaveStateImage(LPCTSTR sFileName)
{
    if (Emulator_SaveImage(sFileName))
        Test_LogFormat('i', _T("Saved state image %s"), sFileName);
    else
        Test_LogFormat('E', _T("FAILED to save state image %s"), sFileName);
}
void Test_LoadStateImage(LPCTSTR sFileName)
{
    if (Emulator_LoadImage(sFileName))
        Test_LogFormat('i', _T("Loaded state image %s"), sFileName);
    else
        Test_LogFormat('E', _T("FAILED to load state image %s"), sFileName);
}

void Test_AssertFailed(LPCSTR sFileName, int nLine)
{
    Test_LogFormat('E', _T("ASSERT FAILED in %S at line %d"), sFileName, nLine);
}

void Test_CompareText(const char * text, const char * etalon)
{
    bool equal = true;
    int pos;
    for (pos = 0;; pos++)
    {
        if (text[pos] == 0 && etalon[pos] == 0)
            break;
        if (text[pos] == 0 || etalon[pos] == 0 || text[pos] != etalon[pos])
        {
            equal = false;
            break;
        }
    }
    if (equal)
        return;

    Test_LogFormat('E', _T("TEST FAILED checking text, diff at pos %d"), pos);

    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    equal = true;
    for (pos = 0;; pos++)
    {
        char ch = text[pos];
        if (ch == 0 && etalon[pos] == 0)
            break;
        if (ch == 0 || etalon[pos] == 0)
            break;  //TODO: show other symbol
        if (ch == etalon[pos])
        {
            if (!equal)
                ::SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            equal = true;
        }
        else
        {
            if (equal)
                ::SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
            equal = false;
            if (ch == ' ') ch = -6; //0xfa;
        }

        printf("%c", ch);
    }
    printf("\n");
    ::SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}


//////////////////////////////////////////////////////////////////////
