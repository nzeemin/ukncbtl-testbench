/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// main.cpp

#include "stdafx.h"
#include "Emulator.h"


void Test1_MenuAndSelfTest()
{
    Test_Init(_T("TEST 1: Menu & Self Test"));

    Emulator_Run(75);  // Boot: 3 seconds

    Test_CheckScreenshot(_T("data\\test01_1.bmp"));  // Boot menu

    Emulator_KeyboardPressRelease(0152);  // "ÓÑÒ"
    Emulator_Run(5);
    Test_CheckScreenshot(_T("data\\test01_2.bmp"));  // Settings menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Test_CheckScreenshot(_T("data\\test01_3.bmp"));  // Settings menu: colors
    Emulator_KeyboardPressRelease(0151);  // "ÈÑÏ"
    Emulator_Run(5);

    Emulator_KeyboardPressRelease(0016);  // "7"
    Emulator_KeyboardPressRelease(0153);  // "Enter"

    Emulator_Run(20);
    Test_CheckScreenshot(_T("data\\test01_4.bmp"));  // Self test pass 1
    Emulator_Run(25 * 200);
    Test_CheckScreenshot(_T("data\\test01_5.bmp"));  // Self test pass 2

    Test_Done();
}

void Test2_Basic()
{
    Test_Init(_T("TEST 2: BASIC"));

    BOOL res = Emulator_LoadROMCartridge(1, _T("romctr_basic.bin"));
    if (!res)
    {
        Test_LogError(_T("Failed to load ROM BASIC cartridge."));
        exit(1);
    }

    Emulator_Run(75);  // Boot: 3 seconds

    Emulator_KeyboardPressRelease(0031);  // "2"
    Emulator_KeyboardPressRelease(0153);  // "Enter"

    Emulator_Run(100);  // Boot BASIC: 5 seconds

    Test_CheckScreenshot(_T("data\\test02_1.bmp"));

    Emulator_KeyboardSequence("PRINT PI\n");

    // 10 FOR I=32 TO 255
    Emulator_KeyboardSequence("10 FOR I");
    Emulator_KeyboardPressReleaseShift(0175);  // "="
    Emulator_KeyboardSequence("32 TO 255\n");
    // 20 PRINT CHR$(I);
    Emulator_KeyboardSequence("20 PRINT CHR");
    Emulator_KeyboardPressReleaseShift(0013);  // "$"
    Emulator_KeyboardPressReleaseShift(0017);  // "("
    Emulator_KeyboardPressRelease(0073);  // "I"
    Emulator_KeyboardPressReleaseShift(0177);  // ")"
    Emulator_KeyboardSequence(";\n");
    // 30 IF I MOD 16 = 15 THEN PRINT
    Emulator_KeyboardSequence("30 IF I MOD 16 ");
    Emulator_KeyboardPressReleaseShift(0175);  // "="
    Emulator_KeyboardSequence(" 15 THEN PRINT\n");
    // 50 NEXT I
    Emulator_KeyboardSequence("50 NEXT I\n");

    Emulator_KeyboardPressRelease(0015);  // "K5" == run
    Emulator_Run(25);  // Wait 1 second

    Test_CheckScreenshot(_T("data\\test02_2.bmp"));

    Emulator_Reset();
    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    Emulator_KeyboardSequence("RU DBAS\n");
    Emulator_Run(190);  // Boot BASIC: 5 seconds
    Test_CheckScreenshot(_T("data\\test02_5.bmp"));

    Test_Done();
}

void Test3_FODOSTM1()
{
    Test_Init(_T("TEST 3: FODOSTM1"));

    Test_CopyFile(_T("data\\fodostm1.dsk"), _T("temp\\fodostm1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\fodostm1.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot from the disk: 8 seconds

    Emulator_KeyboardSequence("UTST01\n");
    Emulator_Run(1750);
    Test_CheckScreenshot(_T("data\\test03_1.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(9550);  // 6:22
    Test_CheckScreenshot(_T("data\\test03_2.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(2950);  // 1:58
    Test_CheckScreenshot(_T("data\\test03_3.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(1000);  // 0:40
    Test_CheckScreenshot(_T("data\\test03_4.bmp"));

    // Turn off the timer
    Emulator_KeyboardPressRelease(0152);  // "ÓÑÒ"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('2');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "ÈÑÏ"
    Emulator_Run(10);

    // Run the FTMON tests
    Emulator_KeyboardSequence("R FTMON\n");
    Emulator_Run(75);  // Loading FTMON
    Emulator_KeyboardSequence("D\n");  // Description
    Emulator_Run(125);
    Test_CheckScreenshot(_T("data\\test03_5.bmp"));
    Emulator_KeyboardSequence("O 791401\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("O 791402\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("O 691404\n");
    Emulator_Run(66);
    Test_CheckScreenshot(_T("data\\test03_6.bmp"));

    Emulator_KeyboardSequence("O SPEED\n");
    Emulator_Run(66);

    // Turn on the timer
    Emulator_KeyboardPressRelease(0152);  // "ÓÑÒ"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('1');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "ÈÑÏ"

    Emulator_Run(250);
    Test_SaveScreenshot(_T("test03_speed.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot from the disk: 8 seconds
    Emulator_KeyboardSequence("IOSCAN\n");  // Run I/O port scanner
    Emulator_Run(50);
    Test_SaveScreenshot(_T("test03_ioscan.bmp"));

    Test_Done();
}

void Test4_Games()
{
    Test_Init(_T("TEST 4: Games"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_CopyFile(_T("data\\game.dsk"), _T("temp\\game.dsk"));

    Test_AttachFloppyImage(0, _T("temp\\disk1.DSK"));
    Test_AttachFloppyImage(1, _T("temp\\game.DSK"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:GOBLIN\n");
    Emulator_Run(200);  // 8 seconds
    Test_CheckScreenshot(_T("data\\test04_1.bmp"));  // Title screen
    Emulator_Run(300);  // 12 seconds
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_Run(25);
    Test_CheckScreenshot(_T("data\\test04_2.bmp"));  // Game start screen

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:DIGGER\n");
    Emulator_Run(400);  // Skip titles
    Emulator_KeyboardSequence("1");  // Game rank
    Emulator_Run(120);
    Test_CheckScreenshot(_T("data\\test04_3.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:CASTLE\n");
    Emulator_Run(225);  // Wait for title on blue background
    Test_CheckScreenshot(_T("data\\test04_4.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_Run(50);
    Emulator_KeyboardSequence("1");  // Game rank
    Emulator_Run(75);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Test_CheckScreenshot(_T("data\\test04_5.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:SOUCOB\n");
    Emulator_Run(275);
    Test_CheckScreenshot(_T("data\\test04_6.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test04_7.bmp"));

    Test_Done();
}

void Test5_Disks()
{
    Test_Init(_T("TEST 5: Disks"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_CreateDiskImage(_T("temp\\tempdisk.dsk"), 40);
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\tempdisk.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    // Initialize MZ1: disk
    Emulator_KeyboardSequence("INIT MZ1:\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("Y\n");  // Are you sure?
    Emulator_Run(50);
    Emulator_KeyboardSequence("DIR MZ1:\n");
    Emulator_Run(250);
    Test_CheckScreenshot(_T("data\\test05_1.bmp"));

    //DebugLogClear();
    //DebugLog(_T("Command COPY MZ0:PIP.SAV MZ1:\n"));
    Emulator_KeyboardSequence("COPY MZ0:PIP.SAV MZ1:\n");
    Emulator_Run(1800);
    //DebugLog(_T("Command DIR MZ1:\n"));
    Emulator_KeyboardSequence("DIR MZ1:\n");
    Emulator_Run(750);
    Test_CheckScreenshot(_T("data\\test05_2.bmp"));

    Emulator_KeyboardSequence("COPY /DEVICE MZ0: MZ1:\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("Y\n");  // Are you sure?
    Emulator_Run(1750);
    Emulator_KeyboardSequence("DIR /SUM MZ1:\n");
    Emulator_Run(150);
    Emulator_KeyboardSequence("BOOT MZ1:\n");
    Emulator_Run(250);
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test05_3.bmp"));

    Test_Done();
}

void Test6_TurboBasic()
{
    Test_Init(_T("TEST 6: Turbo Basic"));

    Test_CopyFile(_T("data\\turbo.dsk"), _T("temp\\turbo.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\turbo.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(125);  // Boot: 8 seconds
    Emulator_KeyboardSequence("\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    Emulator_KeyboardSequence("TURBO\n");
    Emulator_Run(500);
    Test_CheckScreenshot(_T("data\\test06_1.bmp"));

    Test_Done();
}

void Test7_TapeRead()
{
    Test_Init(_T("TEST 7: Read tape"));

    Emulator_Run(75);
    Emulator_KeyboardSequence("5\n");
    Emulator_Run(25);
    Test_OpenTape(_T("data\\UKNC_VERT.wav"));
    Emulator_Run(92 * 25);
    Test_CloseTape();
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test07_1.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(25);
    Test_CheckScreenshot(_T("data\\test07_2.bmp"));

    Test_Done();
}

void Test8_GD()
{
    Test_Init(_T("TEST 8: GD"));

    Test_CopyFile(_T("data\\GD.dsk"), _T("temp\\GD.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\GD.dsk"));

    Emulator_Run(75);
    Emulator_KeyboardPressReleaseChar('1');
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(750);  // Boot from MZ1
    Emulator_KeyboardSequence("SET GD ON\n");
    Emulator_Run(150);
    Emulator_KeyboardSequence("MOUNT LD0 TST1\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("ASS LD0 DK\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("RU TST1\n");
    Emulator_Run(200);
    Emulator_KeyboardPressRelease(077);  // @
    Emulator_KeyboardSequence("DEM1\n");
    Emulator_Run(100);
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(25);

    //TCHAR buffer[100];
    //for (int i = 0; i < 507; i++)
    //{
    //    swprintf(buffer, 100, _T("video\\tst1_%04u.bmp"), i);
    //    Test_SaveScreenshot(buffer);
    //    Emulator_Run(10);
    //}
    Emulator_Run(109 * 10);
    Test_CheckScreenshot(_T("data\\test08_01.bmp"));
    Emulator_Run((203 - 109) * 10);
    Test_CheckScreenshot(_T("data\\test08_02.bmp"));
    Emulator_Run((295 - 203) * 10);
    Test_CheckScreenshot(_T("data\\test08_03.bmp"));
    Emulator_Run((338 - 295) * 10);
    Test_CheckScreenshot(_T("data\\test08_04.bmp"));
    Emulator_Run((422 - 338) * 10);
    Test_CheckScreenshot(_T("data\\test08_05.bmp"));
    Emulator_Run((448 - 422) * 10);
    Test_CheckScreenshot(_T("data\\test08_06.bmp"));
    Emulator_Run((495 - 448) * 10);
    Test_CheckScreenshot(_T("data\\test08_07.bmp"));
    Emulator_Run((507 - 495) * 10);

    Emulator_KeyboardSequence("EXIT\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("ASS MZ1 DK\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("MOUNT LD1 TST2\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("ASS LD1 DK\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("RU TST2\n");
    Emulator_Run(200);
    Test_CheckScreenshot(_T("data\\test08_10.bmp"));
    Emulator_KeyboardPressRelease(077);  // @
    Emulator_KeyboardSequence("DEM2\n");
    Emulator_Run(50);

    //TCHAR buffer[100];
    //for (int i = 0; i < 2125; i++)
    //{
    //    swprintf(buffer, 100, _T("video\\tst2_%04u.bmp"), i);
    //    Test_SaveScreenshot(buffer);
    //    Emulator_Run(10);
    //}
    Emulator_Run(206 * 10);
    Test_CheckScreenshot(_T("data\\test08_11.bmp"));
    Emulator_Run((283 - 206) * 10);
    Test_CheckScreenshot(_T("data\\test08_12.bmp"));
    Emulator_Run((508 - 283) * 10);
    Test_CheckScreenshot(_T("data\\test08_13.bmp"));
    Emulator_Run((624 - 508) * 10);
    Test_CheckScreenshot(_T("data\\test08_14.bmp"));
    Emulator_Run((979 - 624) * 10);
    Test_CheckScreenshot(_T("data\\test08_15.bmp"));
    Emulator_Run((2125 - 979) * 10);
    Test_CheckScreenshot(_T("data\\test08_16.bmp"));

    Test_Done();
}

int _tmain(int argc, _TCHAR* argv[])
{
    Test_LogInfo(_T("Initialization..."));
    BOOL init = Emulator_Init();
    if (!init)
    {
        Test_LogError(_T("Initialization failed."));
        return 1;
    }

    Test1_MenuAndSelfTest();
    Test2_Basic();
    Test3_FODOSTM1();
    Test4_Games();
    Test5_Disks();
    Test6_TurboBasic();
    Test7_TapeRead();
    Test8_GD();

    Test_LogInfo(_T("Finalization..."));
    Emulator_Done();

	return 0;
}
