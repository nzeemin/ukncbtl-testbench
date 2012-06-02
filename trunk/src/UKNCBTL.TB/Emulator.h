/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// Emulator.h

#pragma once

#include "emubase\Board.h"

//////////////////////////////////////////////////////////////////////


extern CMotherboard* g_pBoard;

extern BOOL g_okEmulatorRunning;


//////////////////////////////////////////////////////////////////////


BOOL Emulator_Init();
void Emulator_Done();
void Emulator_SetCPUBreakpoint(WORD address);
void Emulator_SetPPUBreakpoint(WORD address);
BOOL Emulator_IsBreakpoint();
void Emulator_Start();
void Emulator_Stop();
void Emulator_Reset();
int  Emulator_SystemFrame();

void Emulator_PrepareScreenRGB32(void* pBits, const DWORD* colors);

BOOL Emulator_LoadROMCartridge(int slot, LPCTSTR sFilePath);
BOOL Emulator_AttachFloppyImage(int slot, LPCTSTR sFilePath);

BOOL Emulator_OpenTape(LPCTSTR sFilePath);
void Emulator_CloseTape();

BOOL Emulator_Run(int frames);
BOOL Emulator_SaveScreenshot(LPCTSTR sFileName);
int  Emulator_CheckScreenshot(LPCTSTR sFileName);
void Emulator_KeyboardPressRelease(BYTE ukncscan, int timeout = 3);
void Emulator_KeyboardPressReleaseChar(char ch, int timeout = 3);
void Emulator_KeyboardSequence(const char * str);
void Emulator_KeyboardPressReleaseShift(BYTE ukncscan);


//////////////////////////////////////////////////////////////////////
