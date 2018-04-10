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
#include "util\BitmapFile.h"

//////////////////////////////////////////////////////////////////////


extern CMotherboard* g_pBoard;

extern bool g_okEmulatorRunning;


//////////////////////////////////////////////////////////////////////


bool Emulator_Init();
void Emulator_Done();
void Emulator_SetCPUBreakpoint(uint16_t address);
void Emulator_SetPPUBreakpoint(uint16_t address);
bool Emulator_IsBreakpoint();
void Emulator_Start();
void Emulator_Stop();
void Emulator_Reset();
bool Emulator_SystemFrame();
uint32_t Emulator_GetUptime();  // UKNC uptime, in seconds

void Emulator_PrepareScreenRGB32(void* pBits, const uint32_t* colors);

bool Emulator_LoadROMCartridge(int slot, LPCTSTR sFilePath);
bool Emulator_AttachFloppyImage(int slot, LPCTSTR sFilePath);
bool Emulator_AttachHardImage(int slot, LPCTSTR sFilePath);

bool Emulator_OpenTape(LPCTSTR sFilePath);
bool Emulator_CreateTape(LPCTSTR sFilePath);
void Emulator_CloseTape();

bool Emulator_Run(int frames);
bool Emulator_RunUntilMotorOff();
bool Emulator_RunUntilCursorShown();
bool Emulator_RunAndWaitForCursor(int frames);

bool Emulator_SaveScreenshot(LPCTSTR sFileName);
int  Emulator_CheckScreenshot(LPCTSTR sFileName);
void Emulator_KeyboardPressRelease(BYTE ukncscan, int timeout = 3);
void Emulator_KeyboardPressReleaseChar(char ch, int timeout = 3);
void Emulator_KeyboardSequence(const char * str);
void Emulator_KeyboardPressReleaseShift(BYTE ukncscan);
void Emulator_KeyboardPressReleaseAlt(BYTE ukncscan);
void Emulator_KeyboardPressReleaseCtrl(BYTE ukncscan);

bool Emulator_SaveImage(LPCTSTR sFilePath);
bool Emulator_LoadImage(LPCTSTR sFilePath);


//////////////////////////////////////////////////////////////////////
