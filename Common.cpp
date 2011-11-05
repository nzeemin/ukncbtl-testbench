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

//////////////////////////////////////////////////////////////////////


BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine)
{
    //TODO: Implement in this environment

    return FALSE;
}

void AlertWarning(LPCTSTR sMessage)
{
    //TODO: Implement in this environment
}
void AlertWarningFormat(LPCTSTR sFormat, ...)
{
    //TODO: Implement in this environment
}


//////////////////////////////////////////////////////////////////////
// DebugPrint and DebugLog

#if !defined(PRODUCT)

void DebugPrint(LPCTSTR message)
{
    //TODO: Implement in this environment
}

void DebugPrintFormat(LPCTSTR pszFormat, ...)
{
    //TODO: Implement in this environment
}

const LPCTSTR TRACELOG_FILE_NAME = _T("trace.log");
const LPCTSTR TRACELOG_NEWLINE = _T("\r\n");

HANDLE Common_LogFile = NULL;

void DebugLog(LPCTSTR message)
{
    Test_Log('d', message);
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


// Названия регистров процессора
const TCHAR* REGISTER_NAME[] = { _T("R0"), _T("R1"), _T("R2"), _T("R3"), _T("R4"), _T("R5"), _T("SP"), _T("PC") };


// Print octal 16-bit value to buffer
// buffer size at least 7 characters
void PrintOctalValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 6; p++) {
        int digit = value & 7;
        buffer[5 - p] = _T('0') + digit;
        value = (value >> 3);
    }
    buffer[6] = 0;
}
// Print hex 16-bit value to buffer
// buffer size at least 5 characters
void PrintHexValue(TCHAR* buffer, WORD value)
{
    for (int p = 0; p < 4; p++) {
        int digit = value & 15;
        buffer[3 - p] = (digit < 10) ? _T('0') + digit : _T('a') + digit - 10;
        value = (value >> 4);
    }
    buffer[4] = 0;
}
// Print binary 16-bit value to buffer
// buffer size at least 17 characters
void PrintBinaryValue(TCHAR* buffer, WORD value)
{
    for (int b = 0; b < 16; b++) {
        int bit = (value >> b) & 1;
        buffer[15 - b] = bit ? _T('1') : _T('0');
    }
    buffer[16] = 0;
}


//////////////////////////////////////////////////////////////////////


void Test_Log(char eventtype, LPCTSTR message)
{
    //TODO: Show UKNC uptime
    SYSTEMTIME stm;
    ::GetLocalTime(&stm);
    printf("%02d:%02d:%02d.%03d %c %S\n",
        stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds,
        eventtype, message);
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

void Test_SaveScreenshot(LPCTSTR sFileName)
{
    if (Emulator_SaveScreenshot(sFileName))
        Test_LogFormat('i', _T("Saved screenshot %s"), sFileName);
    else
        Test_LogFormat('E', _T("FAILED to save screenshot %s"), sFileName);
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

    Test_LogFormat('E', _T("TEST FAILED checking screenshot %s, difference %d"), sFileName, diff);
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


//////////////////////////////////////////////////////////////////////
