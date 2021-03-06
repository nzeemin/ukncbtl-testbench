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
#include "emubase\\Emubase.h"


void ListBusDevices(const CBusDevice** pDevices)
{
    while ((*pDevices) != NULL)
    {
        Test_LogFormat('i', _T("  %s"), (*pDevices)->GetName());
        const WORD * pRanges = (*pDevices)->GetAddressRanges();
        while (*pRanges != 0)
        {
            WORD start = *pRanges;  pRanges++;
            WORD length = *pRanges;  pRanges++;
            Test_LogFormat('i', _T("    %06o-%06o"), start, start + length - 1);
        }
        pDevices++;
    }
}

void Test0_ListBusDevices()
{
    Test_Init(_T("TEST 0: Bus devices list"));

    const CBusDevice** device1 = g_pBoard->GetCPUBusDevices();
    Test_LogInfo(_T("CPU bus devices:"));
    ListBusDevices(device1);
    const CBusDevice** device2 = g_pBoard->GetPPUBusDevices();
    Test_LogInfo(_T("PPU bus devices:"));
    ListBusDevices(device2);

    Test_Done();
}

void Test1_MenuAndSelfTest()
{
    Test_Init(_T("TEST 1: Menu & Self Test"));

    Emulator_RunAndWaitForCursor(75);  // Boot: 3 seconds

    Test_CheckScreenshot(_T("data\\test01_1.bmp"));  // Boot menu

    Emulator_KeyboardPressRelease(0152);  // "���"
    Emulator_RunAndWaitForCursor(5);
    Test_CheckScreenshot(_T("data\\test01_2.bmp"));  // Settings menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_RunAndWaitForCursor(3);
    Test_CheckScreenshot(_T("data\\test01_3.bmp"));  // Settings menu: colors
    Emulator_KeyboardPressRelease(0151);  // "���"
    Emulator_Run(5);

    Emulator_KeyboardSequence("7\n");  // Select "7 - ������������", Enter

    Emulator_RunAndWaitForCursor(22);
    Test_CheckScreenshot(_T("data\\test01_4.bmp"));  // Self test pass 1
    Emulator_RunAndWaitForCursor(25 * 200);
    Test_CheckScreenshot(_T("data\\test01_5.bmp"));  // Self test pass 2

    Test_Done();
}

void Test2_Basic()
{
    Test_Init(_T("TEST 2: BASIC"));

    Test_LoadROMCartridge(1, _T("romctr_basic.bin"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");  // Select boot from the cartridge
    Emulator_AddCPUBreakpoint(000000);
    Test_Assert(!Emulator_Run(6));
    Test_Assert(g_pBoard->GetCPU()->GetPC() == 000000);
    int nAddrType;
    Test_Assert(g_pBoard->GetCPUMemoryController()->GetWordView(000000, FALSE, TRUE, &nAddrType) == 0240);
    Emulator_RemoveCPUBreakpoint(000000);

    Emulator_RunAndWaitForCursor(95);  // Boot BASIC
    Test_CheckScreenshot(_T("data\\test02_1.bmp"));

    Emulator_KeyboardSequence("PRINT PI\n");

    Emulator_KeyboardSequence("10 FOR I=32 TO 255\n");
    Emulator_KeyboardSequence("20 PRINT CHR$(I);\n");
    Emulator_KeyboardSequence("30 IF I MOD 16 = 15 THEN PRINT\n");
    Emulator_KeyboardSequence("50 NEXT I\n");

    Emulator_KeyboardPressRelease(0015);  // "K5" == run
    Emulator_Run(25);
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
    Emulator_RunUntilMotorOff();  // Boot BASIC
    Emulator_RunAndWaitForCursor(25);
    Test_CheckScreenshot(_T("data\\test02_5.bmp"));

    // BASIC speed test by Sergey Frolov, see http://www.leningrad.su/calc/speed.php
    Emulator_KeyboardSequence("4 FOR I = 1 TO 10\n");
    Emulator_KeyboardSequence("5 A = 1.0000001\n");
    Emulator_KeyboardSequence("10 B = A\n");
    Emulator_KeyboardSequence("15 FOR J = 1 TO 27\n");
    Emulator_KeyboardSequence("20 A = A * A\n");
    Emulator_KeyboardSequence("25 B = B ^ 2.01\n");
    Emulator_KeyboardSequence("30 NEXT J\n");
    Emulator_KeyboardSequence("35 NEXT I\n");
    Emulator_KeyboardSequence("40 PRINT A, B\n");
    Emulator_KeyboardPressRelease(0015);  // "K5" == run
    Emulator_RunAndWaitForCursor(145);
    Test_SaveScreenshot(_T("test02_6.bmp"));

    Emulator_KeyboardSequence("NEW\n");
    Emulator_Run(10);
    Emulator_KeyboardSequence("1  !\"#$%&\'()*+,-./\n");
    Emulator_KeyboardSequence("2 0123456789:;<=>?\n");
    Emulator_KeyboardSequence("3 @[\\]^_ `{|}~\n");
    Emulator_KeyboardSequence("4 ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
    Emulator_KeyboardSequence("5 abcdefghijklmnopqrstuvwxyz\n");
    Emulator_RunUntilCursorShown();
    Test_SaveScreenshot(_T("test02_tt.bmp"));

    Emulator_Reset();
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");  // Select boot from the cartridge
    Emulator_Run(100);  // Boot BASIC: 5 seconds

    // Random number generator check by Leonid Broukhis http://www.mailcom.com/bk0010/
    Emulator_KeyboardSequence("NEW\n");
    Emulator_Run(10);
    Emulator_KeyboardSequence("10 SCREEN 2\n");
    Emulator_KeyboardSequence("20 FOR I=0 TO 1000\n");
    Emulator_KeyboardSequence("30 PSET (RND(1)*640, RND(1)*264)\n");
    Emulator_KeyboardSequence("40 NEXT\n");
    Emulator_KeyboardSequence("50 GOTO 50\n");
    Emulator_KeyboardSequence("RUN\n");
    Emulator_RunAndWaitForCursor(180);
    Test_CheckScreenshot(_T("data\\test02_rnd1.bmp"));
    Emulator_KeyboardPressRelease(0004);  // Press STOP
    Emulator_KeyboardSequence("NEW\n");
    Emulator_Run(10);
    Emulator_KeyboardSequence("10 SCREEN 2\n");
    Emulator_KeyboardSequence("20 FOR I%=0% TO 32766%\n");
    Emulator_KeyboardSequence("30 PSET (RND(1)*640%, RND(1)*264%), RND(1)*9%\n");
    Emulator_KeyboardSequence("40 NEXT\n");
    Emulator_KeyboardSequence("50 GOTO 50\n");
    Emulator_KeyboardSequence("RUN\n");
    Emulator_RunAndWaitForCursor(4000);
    Test_CheckScreenshot(_T("data\\test02_rnd2.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test02_%04u.bmp"), 20, 25);

    Emulator_Reset();
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");  // Select boot from the cartridge
    Emulator_Run(100);  // Boot BASIC: 5 seconds

    // 3D letter 'A' - see https://zx-pk.ru/threads/31557-bejsik-na-uknts.html?p=1060194&viewfull=1#post1060194
    Emulator_KeyboardSequence("NEW\n");
    Emulator_Run(10);
    Emulator_KeyboardSequence("10 SCREEN 2\n");
    Emulator_KeyboardSequence("11 FA=3.333\n");
    Emulator_KeyboardSequence("20 FOR X=-20 TO 130 STEP 5\n");
    Emulator_KeyboardSequence("30 FOR Y=0 TO 200\n");
    Emulator_KeyboardSequence("40 GOSUB 120\n");
    Emulator_KeyboardSequence("50 NY=Y-X*.5+80\n");
    Emulator_KeyboardSequence("51 NZ=Z+X*0.6+80\n");
    Emulator_KeyboardSequence("60 LINE(NY,NZ)-(NY,199),0\n");
    Emulator_KeyboardSequence("70 IF Y=0 THEN PSET(NY,NZ) ELSE LINE(PY,PZ)-(NY,NZ)\n");
    Emulator_KeyboardSequence("80 PY=NY\n");
    Emulator_KeyboardSequence("81 PZ=NZ\n");
    Emulator_KeyboardSequence("90 NEXT Y\n");
    Emulator_KeyboardSequence("100 NEXT X\n");
    Emulator_KeyboardSequence("110 GOTO 110\n");
    Emulator_KeyboardSequence("120 REM FUNCTION\n");
    Emulator_KeyboardSequence("130 Z=Y*.1\n");
    Emulator_KeyboardSequence("131 XT=X*.1\n");
    Emulator_KeyboardSequence("132 YT=(Y+120)*.06\n");
    Emulator_KeyboardSequence("140 IF XT<0 OR XT>10 THEN RETURN\n");
    Emulator_KeyboardSequence("150 IF XT<-FA*(YT-10)+10 OR XT<FA*(YT-10)-16.66666 THEN RETURN\n");
    Emulator_KeyboardSequence("160 IF XT<-FA*(YT-10)+16.66666 OR XT<FA*(YT-10)-10 OR (XT>6 AND XT<8) THEN Z=-20\n");
    Emulator_KeyboardSequence("170 RETURN\n");
    Emulator_KeyboardSequence("RUN\n");
    Emulator_Run(4700);
    Test_CheckScreenshot(_T("data\\test02_letterA.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test02_%04u.bmp"), 15, 100);

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
    Emulator_RunAndWaitForCursor(1750);
    Test_CheckScreenshot(_T("data\\test03_1.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_RunAndWaitForCursor(9725);  // 6:29
    Test_CheckScreenshot(_T("data\\test03_2.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_RunAndWaitForCursor(3150);  // 2:06
    Test_CheckScreenshot(_T("data\\test03_3.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_RunAndWaitForCursor(1100);  // 0:44
    Test_CheckScreenshot(_T("data\\test03_4.bmp"));

    // Turn off the timer
    Emulator_KeyboardPressRelease(0152);  // "���"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('2');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "���"
    Emulator_Run(10);

    // Run the FTMON tests
    Emulator_KeyboardSequence("R FTMON\n");
    Emulator_Run(75);  // Loading FTMON
    Emulator_KeyboardSequence("D\n");  // Description
    Emulator_RunAndWaitForCursor(125);
    Test_CheckScreenshot(_T("data\\test03_5.bmp"));
    Emulator_KeyboardSequence("O 791401\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("O 791402\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("O 691404\n");
    Emulator_RunAndWaitForCursor(66);
    Test_CheckScreenshot(_T("data\\test03_6.bmp"));

    Emulator_KeyboardSequence("O SPEED\n");
    Emulator_Run(66);

    // Turn on the timer
    Emulator_KeyboardPressRelease(0152);  // "���"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('1');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "���"

    Emulator_RunAndWaitForCursor(250);
    Test_SaveScreenshot(_T("test03_speed.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot from the disk: 8 seconds

    Emulator_KeyboardSequence("SPEED\n");
    Emulator_Run(550);

    // Turn off the timer
    Emulator_KeyboardPressRelease(0152);  // "���"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('2');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "���"
    Emulator_Run(10);
    Test_SaveScreenshot(_T("test03_speed2.bmp"));

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
    Emulator_RunAndWaitForCursor(200);  // 8 seconds
    Test_CheckScreenshot(_T("data\\test04_1.bmp"));  // Title screen
    Emulator_Run(300);  // 12 seconds
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(25);
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
    Emulator_RunAndWaitForCursor(120);
    Test_CheckScreenshot(_T("data\\test04_3.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:CASTLE\n");
    Emulator_RunAndWaitForCursor(225);  // Wait for title on blue background
    Test_CheckScreenshot(_T("data\\test04_4.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_Run(50);
    Emulator_KeyboardSequence("1");  // Game rank
    Emulator_Run(75);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_RunAndWaitForCursor(5);
    Test_CheckScreenshot(_T("data\\test04_5.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:SOUCOB\n");
    Emulator_RunAndWaitForCursor(275);
    Test_CheckScreenshot(_T("data\\test04_6.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test04_7.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:SPION\n");
    Emulator_Run(275);
    Test_CheckScreenshot(_T("data\\test04_8.bmp"));
    Emulator_KeyboardSequence("1");  // Game rank
    Emulator_Run(51);
    Test_CheckScreenshot(_T("data\\test04_9.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:GARDEN\n");
    Emulator_RunAndWaitForCursor(200);
    Test_CheckScreenshot(_T("data\\test04_10.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_RunAndWaitForCursor(65);
    Test_CheckScreenshot(_T("data\\test04_11.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:CAT\n");
    Emulator_Run(250);
    //NOTE: ��������� � ��� ����, �� ��� ������ (��������������� ������ �� ��������)
    Test_SaveScreenshot(_T("test04_12.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:LAND\n");
    Emulator_RunAndWaitForCursor(200);
    Test_CheckScreenshot(_T("data\\test04_14.bmp"));
    Emulator_Run(10);
    Emulator_KeyboardPressReleaseChar(' ', 15);
    Emulator_RunAndWaitForCursor(95);
    Test_CheckScreenshot(_T("data\\test04_15.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:CHESS\n");
    Emulator_Run(100);
    //Test_SaveScreenshot(_T("test04_16.bmp"));  // Description page 1
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(20);
    //Test_SaveScreenshot(_T("test04_17.bmp"));  // Description page 2
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(1000);
    Emulator_KeyboardSequence("P/E2-E4\n");
    Emulator_Run(20);
    Test_CheckScreenshot(_T("data\\test04_19.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:TETRIS\n");
    Emulator_Run(350);
    Test_CheckScreenshot(_T("data\\test04_20.bmp"));
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(250);
    Emulator_KeyboardSequence("Y");  // Have you color TV?
    Emulator_Run(10);
    Emulator_KeyboardSequence("1\n");  // Level (1..10)?
    Emulator_Run(20);
    Test_CheckScreenshot(_T("data\\test04_21.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:SAPER\n");
    Emulator_RunAndWaitForCursor(33 * 25);
    Test_CheckScreenshot(_T("data\\test04_22.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_RunAndWaitForCursor(6 * 25);
    Test_CheckScreenshot(_T("data\\test04_23.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 25);
    Test_CheckScreenshot(_T("data\\test04_24.bmp"));

    //Test_SaveScreenshotSeria(_T("video\\test4_%04u.bmp"), 60, 25);

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

    Emulator_KeyboardSequence("SHOW CONF\n");
    Emulator_RunUntilMotorOff();
    Emulator_RunAndWaitForCursor(15);
    Test_CheckScreenshot(_T("data\\test05_0.bmp"));

    // Initialize MZ1: disk
    Emulator_KeyboardSequence("INIT MZ1:\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("Y\n");  // Are you sure?
    Emulator_Run(95);
    Emulator_KeyboardSequence("DIR MZ1:\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(15);
    Test_CheckScreenshot(_T("data\\test05_1.bmp"));

    Emulator_KeyboardSequence("COPY MZ0:PIP.SAV MZ1:\n");
    Emulator_Run(1800);
    Emulator_KeyboardSequence("DIR MZ1:\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(15);
    Test_CheckScreenshot(_T("data\\test05_2.bmp"));

    Emulator_KeyboardSequence("COPY /DEVICE MZ0: MZ1:\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("Y\n");  // Are you sure?
    Emulator_RunUntilMotorOff();
    Emulator_KeyboardSequence("DIR /SUM MZ1:\n");
    Emulator_Run(150);
    Emulator_KeyboardSequence("BOOT MZ1:\n");
    Emulator_Run(425);
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test05_3.bmp"));

    Emulator_KeyboardPressReleaseCtrl(0056);  // Ctrl+L clear screen
    Emulator_KeyboardPressReleaseChar('\n');
    Emulator_Run(25);
    Emulator_KeyboardSequence("DIR/FIL/BAD\n");
    Emulator_RunAndWaitForCursor(850);  // ~34 �������
    Test_CheckScreenshot(_T("data\\test05_4.bmp"));

    //Test_SaveScreenshotSeria(_T("video\\test05_%04u.bmp"), 40, 10);

    Test_Done();
}

void Test52_Disk_TESTMZ()
{
    Test_Init(_T("TEST 5.2: TESTMZ"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_CreateDiskImage(_T("temp\\temp052.dsk"), 80);
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\temp052.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    Emulator_KeyboardSequence("TESTMZ\n");
    Emulator_RunAndWaitForCursor(150);
    Test_CheckScreenshot(_T("data\\test052_0.bmp"));

    Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    Emulator_KeyboardSequence("\n");   // Select "��������������"
    Emulator_KeyboardSequence("1\n");  // Select MZ1:
    Emulator_KeyboardSequence("N\n");  // "�������� �����������?" ���
    Emulator_KeyboardSequence("\n");   // "������� �� 0"
    Emulator_KeyboardSequence("79\n"); // "�� 39"
    Emulator_KeyboardSequence("\n");   // "�������"
    Emulator_KeyboardSequence("\n");   // "������"
    Emulator_KeyboardSequence("\n");   // "���-����"
    Test_CheckScreenshot(_T("data\\test052_1.bmp"));
    Emulator_RunAndWaitForCursor(43 * 75 + 8 * 25);  // Formatting
    Test_CheckScreenshot(_T("data\\test052_2.bmp"));
    Emulator_KeyboardSequence("K\n");  // Exit to main menu

    //NOTE: �������������� ���� -- ����� ��������������� �� ��������� 1-�� �������
    //Emulator_Run(75);
    //Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    //Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    //Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    //Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    //Test_SaveScreenshot(_T("test052_3.bmp"));
    //Emulator_KeyboardSequence("\n");  // Select "�������������� ����"

    //Test_SaveScreenshot(_T("test052_4.bmp"));
    //Emulator_RunUntilMotorOff();
    //Test_SaveScreenshot(_T("test052_5.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test052_%04u.bmp"), 60, 300);
}

void Test6_TurboBasic()
{
    Test_Init(_T("TEST 6: Turbo Basic"));

    Test_CopyFile(_T("data\\turbo.dsk"), _T("temp\\turbo.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\turbo.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(175);  // Boot: 8 seconds
    Emulator_KeyboardSequence("\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    Emulator_KeyboardSequence("TURBO\n");
    Emulator_Run(590);
    Test_CheckScreenshot(_T("data\\test06_01.bmp"));

    // Load TESTGR.BAS and run, see http://zx.pk.ru/showpost.php?p=420453&postcount=238
    Emulator_KeyboardPressRelease(0012);  // "K3"
    Emulator_Run(275);
    // ����� ���� ��������: "������ �� ����������� ��� ����, ������� �� ���; � ��������� �������� ���������"; ����������� � r397
    Test_CheckScreenshot(_T("data\\test06_02.bmp"));
    Emulator_KeyboardSequence("TESTGR\n");
    Emulator_Run(170);
    Test_CheckScreenshot(_T("data\\test06_03.bmp"));
    Emulator_KeyboardPressReleaseAlt(0171);  // Alt+F9 -- Compile
    Emulator_Run(25 * 9);
    Test_CheckScreenshot(_T("data\\test06_04.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(50);
    Emulator_KeyboardPressReleaseCtrl(0171);  // Ctrl+F9 -- Run
    Emulator_Run(27 * 22);
    Test_CheckScreenshot(_T("data\\test06_05.bmp"));  // Title screen
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(62);
    Test_CheckScreenshot(_T("data\\test06_06.bmp"));  // Menu screen
    Emulator_KeyboardPressRelease(0154, 40);  // "Up arrow"
    Emulator_KeyboardPressReleaseChar('\n');  // Select "All demonstration"
    Emulator_Run(28 * 15);
    Test_CheckScreenshot(_T("data\\test06_07.bmp"));  // Circles
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 32);
    Test_CheckScreenshot(_T("data\\test06_08.bmp"));
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(5 * 23 + 2);
    Test_CheckScreenshot(_T("data\\test06_09.bmp"));  // Circles
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(126);
    Test_CheckScreenshot(_T("data\\test06_10.bmp"));  // Blocks
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(260);
    Test_CheckScreenshot(_T("data\\test06_11.bmp"));  // Blocks
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(150);
    Test_CheckScreenshot(_T("data\\test06_12.bmp"));  // Line
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(75);
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 15);
    Test_CheckScreenshot(_T("data\\test06_14.bmp"));  // Lines
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 9 + 6);
    Test_CheckScreenshot(_T("data\\test06_15.bmp"));  // Pages
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(5 * 44 - 2);
    Test_CheckScreenshot(_T("data\\test06_16.bmp"));  // Space frogs
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(92);
    Test_CheckScreenshot(_T("data\\test06_17.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test06_%04u.bmp"), 50, 2);

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

    Emulator_Reset();
    Emulator_Run(75);
    Emulator_KeyboardSequence("5\n");
    Emulator_Run(25);
    Test_OpenTape(_T("data\\UKNC_ANT.wav"));
    Emulator_Run(247 * 25);
    Test_CloseTape();
    Emulator_Run(125);
    Test_CheckScreenshot(_T("data\\test07_3.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(125);
    Test_CheckScreenshot(_T("data\\test07_4.bmp"));

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

    Emulator_RunAndWaitForCursor(109 * 10);
    Test_CheckScreenshot(_T("data\\test08_01.bmp"));
    Emulator_RunAndWaitForCursor(920);
    Test_CheckScreenshot(_T("data\\test08_02.bmp"));
    Emulator_RunAndWaitForCursor(910);
    Test_CheckScreenshot(_T("data\\test08_03.bmp"));
    Emulator_RunAndWaitForCursor(420);
    Test_CheckScreenshot(_T("data\\test08_04.bmp"));
    Emulator_Run(865);
    Test_CheckScreenshot(_T("data\\test08_05.bmp"));
    Emulator_RunAndWaitForCursor(230);
    Test_CheckScreenshot(_T("data\\test08_06.bmp"));
    Emulator_RunAndWaitForCursor(480);
    Test_CheckScreenshot(_T("data\\test08_07.bmp"));
    Emulator_Run(70);

    Emulator_KeyboardSequence("EXIT\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("ASS MZ1 DK\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("MOUNT LD1 TST2\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("ASS LD1 DK\n");
    Emulator_Run(100);
    Emulator_KeyboardSequence("RU TST2\n");
    Emulator_RunAndWaitForCursor(215);
    Test_CheckScreenshot(_T("data\\test08_10.bmp"));
    Emulator_KeyboardPressRelease(077);  // @
    Emulator_KeyboardSequence("DEM2\n");
    Emulator_Run(50);

    Emulator_RunAndWaitForCursor(208 * 10);
    Test_CheckScreenshot(_T("data\\test08_11.bmp"));
    Emulator_RunAndWaitForCursor(75 * 10);
    Test_CheckScreenshot(_T("data\\test08_12.bmp"));
    Emulator_RunAndWaitForCursor(223 * 10);
    Test_CheckScreenshot(_T("data\\test08_13.bmp"));
    Emulator_RunAndWaitForCursor(118 * 10);
    Test_CheckScreenshot(_T("data\\test08_14.bmp"));
    Emulator_RunAndWaitForCursor(359 * 10);
    Test_CheckScreenshot(_T("data\\test08_15.bmp"));
    Emulator_RunAndWaitForCursor(1139 * 10);
    Test_CheckScreenshot(_T("data\\test08_16.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test08_%04u.bmp"), 10, 10);

    Test_Done();
}

void Test9_HDD()
{
    Test_Init(_T("TEST 9: HDD"));

    Test_LoadROMCartridge(1, _T("data\\ide_wdromv0110.bin"));

    Test_CopyFile(_T("data\\sys1002wdx.dsk"), _T("temp\\sys1002wdx.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\sys1002wdx.dsk"));

    // Create empty HDD image
    // (63 sectors/track) * (4 heads) * (80 tracks) * (512 bytes/sector) = 10321920 bytes = 9.84375 MB
    Test_CreateHardImage(63, 4, 80, _T("temp\\hdd.img"));
    Test_AttachHardImage(1, _T("temp\\hdd.img"));

    // Boot from the sys1002wdx.dsk
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(350);
    Emulator_KeyboardSequence("\n");
    Emulator_Run(50);

    // Run WDX and answer all the questions
    Emulator_KeyboardSequence("RU WDX\n");
    Emulator_Run(125);
    Emulator_KeyboardSequence("1\n");  // Which slot should I use [1:12] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("N\n");  // May read default block from file. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("Y\n");  // Should clean data. Do it [N] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // May execute autodetect. Do it [Y] ?
    Emulator_Run(100);
    Test_CheckScreenshot(_T("data\\test09_02.bmp"));  // Autodetected params
    Emulator_KeyboardSequence("\n");  // Disk parameters're correctly set; continue. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("4\n");  // Enter number of partitions [1:124] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("6047\n");  // Partition #00 size, blocks
    Emulator_Run(25);
    Emulator_KeyboardSequence("6048\n");  // Partition #01 size
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // Partition #02 size
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // Partition #03 size
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test09_03.bmp"));  // Partitions table
    Emulator_KeyboardSequence("Y\n");  // Test a partition. Do it [N] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("1\n");  // Partition to test
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // First block
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // Last block
    Emulator_Run(75);
    //Test_SaveScreenshot(_T("test09_04.bmp"));  // Partition test complete
    Emulator_KeyboardSequence("N\n");  // Test a partition. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("300\n");  // Enter time for awiting in ticks [0:0450] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("N\n");  // There'll be made a hidden partition. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("\n");  // Size of PP memory shift, bytes [0:032767] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("N\n");  // Should look on data. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("N\n");  // Should save data in file. Do it [Y] ?
    Emulator_Run(25);
    Emulator_KeyboardSequence("Y\n");  // Is ready to load out master block onto disk. Do it [N] ?
    Emulator_Run(25);
    Test_CheckScreenshot(_T("data\\test09_05.bmp"));  // WDX completed

    Emulator_KeyboardSequence("SET WD SYSGEN\n");
    Emulator_Run(50);

    // Reboot from the same floppy
    Emulator_Reset();
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(350);
    Emulator_KeyboardSequence("\n");
    Emulator_Run(50);

    // Run WDR
    Emulator_KeyboardSequence("RU WDR\n");
    Emulator_Run(50);
    //Test_SaveScreenshot(_T("test09_06.bmp"));  // WDR installed

    // Load WD driver
    // The following statement fails with the message "Invalid device WD:"
    Emulator_KeyboardSequence("LOAD WD\n");
    Emulator_Run(50);
    //Test_SaveScreenshot(_T("test09_07.bmp"));

    // Initialize the first HDD partition
    Emulator_KeyboardSequence("DIR WD0:\n");
    Emulator_Run(50);
    //Test_SaveScreenshot(_T("test09_08.bmp"));  // "Invalid directory"
    Emulator_KeyboardSequence("INIT WD0:\n");
    Emulator_Run(50);
    Emulator_KeyboardSequence("Y\n");  // Are you sure?
    Emulator_Run(50);
    //Test_SaveScreenshot(_T("test09_09.bmp"));
    Emulator_KeyboardSequence("DIR WD0:\n");
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test09_10.bmp"));  // 0 Files, 0 Blocks, 6009 Free blocks

    // Copy all files from the floppy to the HDD
    Emulator_KeyboardSequence("COPY/SYS MZ0: WD0:\n");
    Emulator_Run(1600);
    //Test_SaveScreenshot(_T("test09_11.bmp"));

    // Copy the bootloader
    Emulator_KeyboardSequence("COPY/BOOT:WD WD0:RT11SJ WD0:\n");
    Emulator_Run(100);
    Test_CheckScreenshot(_T("data\\test09_12.bmp"));

    // Boot from the HDD image
    Emulator_Reset();
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test09_15.bmp"));
    Emulator_Run(350);
    Emulator_KeyboardSequence("\n");
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test09_16.bmp"));

    Test_Done();
}

void Test10_ITO()
{
    Test_Init(_T("TEST 10: ITO disks"));

    Test_CopyFile(_T("data\\ito90.dsk"), _T("temp\\ito90.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\ito90.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Conan" selected
    Emulator_Run(900);
    // ����� ���� ��������: "�� �����������, ������� �� ���, �� ������������� ��������� ��������"; ����������� � r397
    Test_CheckScreenshot(_T("data\\test10_05.bmp"));
    Emulator_KeyboardPressRelease(0153, 6);  // "Enter" on the title screen
    Emulator_Run(1000);
    // ���������� ������� "Conan"
    Test_CheckScreenshot(_T("data\\test10_06.bmp"));
    //TODO: ����� � �������

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134, 10);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153, 6);  // "Enter" -- "Sammy" selected
    Emulator_Run(975);
    // ����� ���� ��������: "�� �����������, ������� �� ���, �� ������������� ��������� ��������"; ����������� � r397
    Test_CheckScreenshot(_T("data\\test10_08.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');  // "Space" on the title screen -- turns on "explosions"
    Emulator_Run(200);
    Emulator_KeyboardPressReleaseChar(' ');  // "Space" on the title screen with explosions
    Emulator_Run(375);
    Test_CheckScreenshot(_T("data\\test10_09.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Knight" selected
    Emulator_Run(700);
    Test_CheckScreenshot(_T("data\\test10_10.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter" on the title screen
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test10_11.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Lode Runner" selected
    Emulator_Run(750);
    Test_CheckScreenshot(_T("data\\test10_12.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');  // "Space" on the title screen
    Emulator_Run(450);
    Emulator_KeyboardPressReleaseChar(' ');  // Start the round
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test10_13.bmp"));

    Emulator_Reset();

    Test_CopyFile(_T("data\\ito91.dsk"), _T("temp\\ito91.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\ito91.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Puckman" selected
    Emulator_Run(390);
    Test_CheckScreenshot(_T("data\\test10_14.bmp"));
    Emulator_Run(225);
    Test_CheckScreenshot(_T("data\\test10_15.bmp"));
    Emulator_Run(225);
    Test_CheckScreenshot(_T("data\\test10_16.bmp"));  // Ready!
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- start the game
    Emulator_Run(99);
    Test_CheckScreenshot(_T("data\\test10_17.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Arkanoid" selected
    Emulator_Run(350);
    Test_CheckScreenshot(_T("data\\test10_18.bmp"));
    Emulator_Run(350);
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test10_19.bmp"));  // In-game screen

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Road Fighter" selected
    Emulator_Run(1000);
    Test_CheckScreenshot(_T("data\\test10_20.bmp"));
    Emulator_Run(350);
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(375);
    Test_CheckScreenshot(_T("data\\test10_21.bmp"));

    //Test_SaveScreenshotSeria(_T("video\\test10_%04u.bmp"), 30, 25);

    Test_Done();
}

void Test11_SteelRat()
{
    Test_Init(_T("TEST 11: Steel Rat"));

    Test_CopyFile(_T("data\\steel_rat.dsk"), _T("temp\\steel_rat.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\steel_rat.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(16 * 25);
    Test_CheckScreenshot(_T("data\\test11_01.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_Run(25);
    Emulator_KeyboardSequence("0\n");
    Emulator_Run(6 * 25);
    Test_CheckScreenshot(_T("data\\test11_02.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_Run(4 * 25);
    Test_CheckScreenshot(_T("data\\test11_03.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_Run(5 * 25);
    Test_CheckScreenshot(_T("data\\test11_04.bmp"));

    Test_Done();
}

void Test12_JEK()
{
    Test_Init(_T("TEST 12: JEK"));

    Test_CopyFile(_T("data\\jek.dsk"), _T("temp\\jek.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\jek.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(30 * 25);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- date
    Emulator_Run(6 * 25);
    Emulator_KeyboardSequence("DEA DK\n");
    Emulator_Run(3 * 25);
    Emulator_KeyboardSequence("JEK\n");
    Emulator_Run(5 * 25);
    Emulator_KeyboardSequence("LE\n");
    Emulator_Run(15 * 25);
    // ����� ���� ��������: "�� �����������, ������� �� ���, �� ������������� ��������� ��������"; ����������� � r397
    Emulator_Run(250);
    // ���������� ����������� ���������� �� ����������� �� �������� ������,
    // ����������� -- ��������� �� r482 � �������� GRB
    Test_CheckScreenshot(_T("data\\test12_01.bmp"));

    //Test_SaveScreenshotSeria(_T("video\\test12_%04u.bmp"), 10, 25);

    Test_Done();
}

void Test13_PAFCommander()
{
    Test_Init(_T("TEST 13: PAF Commander"));

    Test_CopyFile(_T("data\\rt11a5.dsk"), _T("temp\\rt11a5.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\rt11a5.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(20 * 25);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- date
    Emulator_Run(8 * 25);
    Emulator_KeyboardSequence("PC\n");
    Emulator_Run(12 * 25 - 3);
    Test_CheckScreenshot(_T("data\\test13_01.bmp"));
    // ����� ������ ��������� ������ ����

    Test_Done();
}

void Test14_TapeReadWrite()
{
    Test_Init(_T("TEST 14: Tape read/write"));

    Test_LoadROMCartridge(1, _T("romctr_basic.bin"));
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");  // Select boot from the cartridge
    Emulator_Run(100);  // Boot BASIC: 5 seconds

    Emulator_KeyboardSequence("10 PRINT PI\n");

    Emulator_KeyboardSequence("CSAVE \"PI\"\n");
    Test_CreateTape(_T("temp\\test14_01.wav"));
    Emulator_Run(25 * 12);
    Test_CloseTape();
    Test_CheckScreenshot(_T("data\\test14_01.bmp"));

    Emulator_KeyboardSequence("NEW\n");
    Emulator_KeyboardSequence("CLOAD \"PI\"\n");
    Test_OpenTape(_T("temp\\test14_01.wav"));
    Emulator_Run(25 * 12);
    Test_CloseTape();
    Emulator_KeyboardSequence("LIST\n");
    Emulator_Run(5);
    Test_CheckScreenshot(_T("data\\test14_02.bmp"));

    Test_Done();
}

void Test15_VariousTS()
{
    Test_Init(_T("TEST 15: Various TS"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_CopyFile(_T("data\\various.dsk"), _T("temp\\various.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\various.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    // TEST SYSTEM V01.01 ������ �. ���� "��������", ����������, 1992 �.
    Emulator_KeyboardSequence("RU MZ1:TS\n");
    Emulator_Run(125);
    Test_CheckScreenshot(_T("data\\test15_00.bmp"));
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "CPU RAM Test" selected
    Emulator_Run(280);
    Test_CheckScreenshot(_T("data\\test15_01.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "PPU RAM Test" selected
    Emulator_Run(200);
    Test_CheckScreenshot(_T("data\\test15_02.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153, 20);  // "Enter" -- "ROM Test" selected
    Emulator_Run(110);
    Test_CheckScreenshot(_T("data\\test15_03.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "PPU Test" selected
    Emulator_Run(180);
    Test_CheckScreenshot(_T("data\\test15_04.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- "CPU Test" selected
    Emulator_Run(130);
    Test_CheckScreenshot(_T("data\\test15_05.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- "VideoRAM Test" selected
    Emulator_Run(1240);
    Test_CheckScreenshot(_T("data\\test15_06.bmp"));
    Emulator_Run(50);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- "Monitor Test" selected
    Emulator_Run(240);
    Test_CheckScreenshot(_T("data\\test15_07a.bmp"));
    Emulator_Run(80);
    Test_CheckScreenshot(_T("data\\test15_07b.bmp"));
    Emulator_Run(70);
    Test_CheckScreenshot(_T("data\\test15_07c.bmp"));
    Emulator_Run(150);
    Test_CheckScreenshot(_T("data\\test15_07d.bmp"));
    Emulator_Run(80);
    Test_CheckScreenshot(_T("data\\test15_07e.bmp"));
    Emulator_Run(180);
    Test_CheckScreenshot(_T("data\\test15_07f.bmp"));
    Emulator_Run(190);
    Test_CheckScreenshot(_T("data\\test15_07g.bmp"));
    Emulator_Run(25);
    // � ���� ����� ��������, ���������� � r482. ������� ��������� Titus:
    // ��������������� ������ � ������� ������ ���������� (177702) �� ���� ��.
    // �������� ������������ �� ����, ������ � ������� ������ ���������� �� �������������,
    // ������� ������ �������, ����� ������ ���� ��� ������.

    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- "Floppy Drive Test" selected
    Emulator_Run(25);
    Test_CheckScreenshot(_T("data\\test15_08a.bmp"));
    Emulator_Run(25);
    Test_CreateDiskImage(_T("temp\\tempdisk.dsk"), 80);
    Test_AttachFloppyImage(0, _T("temp\\tempdisk.dsk"));
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- run the test
    Emulator_Run(3735);  //NOTE: ��� ����� ������ ������� �� ���������� ���������, � ����� ����� ��������� ����� � ������� � ����
    Test_CheckScreenshot(_T("data\\test15_08b.bmp"));
    Emulator_Run(25);

    //Test_SaveScreenshotSeria(_T("video\\test15_%04u.bmp"), 30, 5);

    //TODO: ��������� �����

    Test_Done();
}

void Test16_Palette128Colors()
{
    Test_Init(_T("TEST 16: Palette 128 Colors"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_CopyFile(_T("data\\various.dsk"), _T("temp\\various.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\various.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds

    // TSPAL by Titus
    Emulator_KeyboardSequence("RU MZ1:TSPAL\n");
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test16_01.bmp"));
    Emulator_KeyboardPressRelease(0153, 10);  // "Enter" -- exit TSPAL
    Emulator_Run(25);

    Emulator_KeyboardSequence("RU MZ1:DLTST\n");
    Emulator_Run(75);
    Test_SaveScreenshot(_T("test16_dltst.bmp"));

    Test_Done();
}

void Test17_VariousOther()
{
    Test_Init(_T("TEST 17: Various Other"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_CopyFile(_T("data\\various.dsk"), _T("temp\\various.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\various.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(120);  // Boot: 3 seconds

    const char * etalonIOSCAN = "\r\n"
            "176560-176576\r\n"
            "176640-176646\r\n"
            "176660-176676\r\n"
            "177560-177566\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:IOSCAN\n");  // Run I/O port scanner
    Emulator_AttachTerminalBuffer();
    Emulator_Run(55);
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonIOSCAN);
    Emulator_DetachTerminalBuffer();

    const char * etalonIOSCPP = "\r\n"
            "177010-177026\r\n"
            "177054\r\n"
            "177060-177102\r\n"
            "177130-177132\r\n"
            "177700-177704\r\n"
            "177710\r\n"
            "177714-177716\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:IOSCPP\n");  // Run I/O port scanner
    Emulator_AttachTerminalBuffer();
    Emulator_Run(33);  Emulator_RunUntilCursorShown();
    Test_CheckScreenshot(_T("data\\test17_ioscan.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonIOSCPP);
    Emulator_DetachTerminalBuffer();

    Emulator_KeyboardSequence("RU MZ1:TSSPD\n");
    Emulator_RunUntilMotorOff();
    Emulator_RunAndWaitForCursor(30);
    Test_SaveScreenshot(_T("test17_tsspdg_1.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_2.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_3.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_4.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_5.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_6.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_7.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdg_8.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"

    Emulator_KeyboardSequence("RU MZ1:IRQ\n");
    Emulator_RunUntilMotorOff();
    Emulator_RunAndWaitForCursor(100);
    Test_SaveScreenshot(_T("test17_irq12.bmp"));

    const char * etalonMOV =
        "MOV - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "Mov     R1     12   35    35      48      35      48    48    47      64 \r\n"
        "Mov    (R1)    33   50    50      66      50      70    66    66      82 \r\n"
        "Mov    (R2)+   33   50    50      66      50      70    66    66      82 \r\n"
        "Mov    (PC)+   33   50    50      66      50      66    66    66      82 \r\n"
        "Mov   @(R2)+   47   70    70      80      70      83    82    82      96 \r\n"
        "Mov   -(R1)    33   50    50      66      50      70    66    66      82 \r\n"
        "Mov  @-(R1)    47   70    70      80      70      83    82    82      96 \r\n"
        "Mov    Addr    47   66    66      82      66      82    80    80      96 \r\n"
        "Mov  @Tab(R1)  60   80    80      96      80      96    96    96      110 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:MOV\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_mov01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonMOV);
    Emulator_DetachTerminalBuffer();

    const char * etalonMOVB =
        "MovB - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "MovB    R1     13   40    40      59      40      59    59    57      71 \r\n"
        "MovB   (R1)    33   59    59      76      64      80    75    76      93 \r\n"
        "MovB   (R2)+   33   59    59      76      64      80    76    76      93 \r\n"
        "MovB   (PC)+   47   76    76      93      76      93    85    85     103 \r\n"
        "MovB  @(R2)+   47   71    71      85      80      94    93    93     107 \r\n"
        "MovB  -(R1)    33   59    59      76      64      80    76    76      93 \r\n"
        "MovB @-(R1)    47   71    71      85      80      94    93    93     106 \r\n"
        "MovB   Addr    47   80    80      93      80      93    85    85     106 \r\n"
        "MovB @Tab(R1)  60   89    89     106      89     106   106   101     120 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:MOVB\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(4100);
    Test_SaveScreenshot(_T("test17_movb01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonMOVB);
    Emulator_DetachTerminalBuffer();

    const char * etalonCMP =
        "Cmp - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "Cmp     R1     13   33    33      47      33      47    47    33      60 \r\n"
        "Cmp    (R1)    33   47    47      66      50      66    66    50      82 \r\n"
        "Cmp    (R2)+   33   47    47      66      50      66    66    50      82 \r\n"
        "Cmp    (PC)+   33   47    47      60      47      60    66    50      80 \r\n"
        "Cmp   @(R2)+   47   60    60      80      66      82    82    70      96 \r\n"
        "Cmp   -(R1)    33   47    47      66      50      66    66    50      82 \r\n"
        "Cmp  @-(R1)    47   60    60      80      66      82    82    70      96 \r\n"
        "Cmp    Addr    47   66    66      80      66      80    80    66      94 \r\n"
        "Cmp  @Tab(R1)  60   80    80      94      80      94    94    82     108 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:CMP\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_cmp01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonCMP);
    Emulator_DetachTerminalBuffer();

    const char * etalonCMPB =
        "CmpB - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "CmpB    R1     13   33    33      47      33      47    47    47      60 \r\n"
        "CmpB   (R1)    33   47    47      66      50      66    66    66      82 \r\n"
        "CmpB   (R2)+   33   47    47      66      50      66    66    66      82 \r\n"
        "CmpB   (PC)+   47   66    66      80      66      80    80    80      94 \r\n"
        "CmpB  @(R2)+   47   60    60      80      66      82    82    82      96 \r\n"
        "CmpB  -(R1)    33   47    47      66      50      66    66    66      82 \r\n"
        "CmpB @-(R1)    47   60    60      80      66      82    82    82      94 \r\n"
        "CmpB   Addr    47   66    66      80      66      80    80    80      94 \r\n"
        "CmpB @Tab(R1)  60   80    80      94      80      94    94    94     108 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:CMPB\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_cmpb01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonCMPB);
    Emulator_DetachTerminalBuffer();

    const char * etalonADD =
        "Add - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "Add     R1     13   40    40      59      40      59    59    57      72 \r\n"
        "Add    (R1)    33   59    59      76      64      80    75    76      93 \r\n"
        "Add    (R2)+   33   59    59      75      64      80    75    76      93 \r\n"
        "Add    (PC)+   33   60    60      80      60      80    75    75      93 \r\n"
        "Add   @(R2)+   47   72    72      85      80      94    93    93     107 \r\n"
        "Add   -(R1)    33   59    59      74      64      80    74    74      93 \r\n"
        "Add  @-(R1)    47   71    71      85      80      94    93    93     107 \r\n"
        "Add    Addr    47   79    79      93      79      93    85    85     106 \r\n"
        "Add  @Tab(R1)  60   88    88     107      88     107   106   101     120 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:ADD\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(4000);
    Test_SaveScreenshot(_T("test17_add01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonADD);
    Emulator_DetachTerminalBuffer();

    const char * etalonBIS =
        "BiS - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "BiS     R1     13   40    40      59      40      59    59    57      72 \r\n"
        "BiS    (R1)    33   59    59      77      65      80    78    79      93 \r\n"
        "BiS    (R2)+   33   59    59      76      65      80    76    76      93 \r\n"
        "BiS    (PC)+   33   60    60      80      60      80    75    75      93 \r\n"
        "BiS   @(R2)+   47   71    71      85      80      94    93    93     106 \r\n"
        "BiS   -(R1)    33   59    59      76      64      80    76    77      93 \r\n"
        "BiS  @-(R1)    47   72    72      85      80      94    93    93     107 \r\n"
        "BiS    Addr    47   80    80      93      80      93    85    85     106 \r\n"
        "BiS  @Tab(R1)  60   89    89     106      89     106   106   101     120 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:BIS\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3800);
    Test_SaveScreenshot(_T("test17_bis01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonBIS);
    Emulator_DetachTerminalBuffer();

    const char * etalonBISB =
        "BiSB - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "BiSB    R1     13   40    40      59      40      59    59    57      72 \r\n"
        "BiSB   (R1)    33   59    59      77      65      80    76    76      93 \r\n"
        "BiSB   (R2)+   33   59    59      76      64      80    75    77      93 \r\n"
        "BiSB   (PC)+   47   76    76      93      76      93    85    85     103 \r\n"
        "BiSB  @(R2)+   47   71    71      85      80      94    93    93     107 \r\n"
        "BiSB  -(R1)    33   59    59      77      65      80    77    79      93 \r\n"
        "BiSB @-(R1)    47   71    71      85      80      94    93    93     106 \r\n"
        "BiSB   Addr    47   80    80      93      80      93    85    85     106 \r\n"
        "BiSB @Tab(R1)  60   89    89     106      89     106   106   101     120 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:BISB\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(4200);
    Test_SaveScreenshot(_T("test17_bisb01.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonBISB);
    Emulator_DetachTerminalBuffer();

    const char * etalonOP1 =
        "Op1 - v1.0\r\nCPU KHz:  5300 > 8013\r\nCPU KHz:  8013\r\n\r\n"
        "SOB : 46 ! Last SOB : 26 ! Br  : 38 ! BCS : 17 ! BCC : 38 ! SeC : 14\r\n\r\n"
        "               R0  (R0)  (R2)+  @(R2)+  -(R1)  @-(R1)  Addr  (PC)+  @Tab(R0)\n\r\n"
        "Tst            13   33    33      47      33      47    47    35      60 \r\n"
        "TstB           13   33    33      47      33      47    47    47      60 \r\n"
        "Inc            12   40    40      59      40      59    59    56      71 \r\n"
        "IncB           13   40    40      59      40      59    59    56      71 \r\n"
        "Clr            13   35    35      48      35      48    48    47      64 \r\n"
        "ClrB           13   40    40      59      40      59    59    56      71 \r\n"
        "MTPS           16   40    40      54      40      54    54    50      70 \r\n"
        "MFPS           13   40    40      59      40      59    59    56      71 \r\n"
        "XOr            13   40    40      59      40      59    59    56      71 \r\n"
        "SwaB           13   40    40      59      40      59    59    56      71 \r\n"
        "SXt            13   35    35      48      35      48    48    47      64 \r\n"
        "\r\nProgram completed.\r\n\r\n.";
    Emulator_KeyboardSequence("RU MZ1:OP1\n");
    Emulator_AttachTerminalBuffer();
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_op1.bmp"));
    DebugLog((const char *)Emulator_GetTerminalBuffer());
    //Test_CompareText((const char *)Emulator_GetTerminalBuffer(), etalonOP1);
    Emulator_DetachTerminalBuffer();

    Emulator_KeyboardSequence("RU MZ1:MOVPC2\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_movpc2.bmp"));

    Emulator_KeyboardSequence("RU MZ1:TSSPDI\n");
    Emulator_RunUntilMotorOff();
    Emulator_RunAndWaitForCursor(30);
    Test_SaveScreenshot(_T("test17_tsspdi_1.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_2.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_3.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_4.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_5.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_6.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_7.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_8.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_9.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_a.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_tsspdi_b.bmp"));
    Emulator_KeyboardPressRelease(0153);  // "Enter"

    Emulator_KeyboardSequence("RU MZ1:PDPCLK\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardPressRelease(0153);  // "Enter"
    Emulator_RunAndWaitForCursor(200);
    Test_SaveScreenshot(_T("test17_pdpclk.bmp"));

    Emulator_KeyboardSequence("RU MZ1:ASH\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_ash.bmp"));

    Emulator_KeyboardSequence("RU MZ1:ASHC\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_ashc.bmp"));

    Emulator_KeyboardSequence("RU MZ1:DIV\n");
    Emulator_RunUntilMotorOff();
    Emulator_Run(100);
    Emulator_KeyboardSequence("8013\n");
    Emulator_RunAndWaitForCursor(3600);
    Test_SaveScreenshot(_T("test17_div.bmp"));

    //Test_SaveScreenshotSeria(_T("video\\test17_%04u.bmp"), 100, 100);

    Test_Done();
}

int _tmain(int /*argc*/, _TCHAR* /*argv*/[])
{
    SYSTEMTIME timeFrom;  ::GetLocalTime(&timeFrom);
    Test_LogInfo(_T("Initialization..."));
    DebugLogClear();

    Test0_ListBusDevices();
    Test1_MenuAndSelfTest();
    Test2_Basic();
    Test3_FODOSTM1();
    Test4_Games();
    Test5_Disks();
    Test52_Disk_TESTMZ();
    Test6_TurboBasic();
    Test7_TapeRead();
    Test8_GD();
    Test9_HDD();
    Test10_ITO();
    Test11_SteelRat();
    Test12_JEK();
    Test13_PAFCommander();
    Test14_TapeReadWrite();
    Test15_VariousTS();
    Test16_Palette128Colors();
    Test17_VariousOther();

    Test_LogInfo(_T("Finalization..."));

    SYSTEMTIME timeTo;  ::GetLocalTime(&timeTo);

    FILETIME fileTimeTo;
    SystemTimeToFileTime(&timeTo, &fileTimeTo);
    ULARGE_INTEGER ulTimeTo;
    ulTimeTo.LowPart = fileTimeTo.dwLowDateTime;
    ulTimeTo.HighPart = fileTimeTo.dwHighDateTime;

    FILETIME fileTimeFrom;
    SystemTimeToFileTime(&timeFrom, &fileTimeFrom);
    ULARGE_INTEGER ulTimeFrom;
    ulTimeFrom.LowPart = fileTimeFrom.dwLowDateTime;
    ulTimeFrom.HighPart = fileTimeFrom.dwHighDateTime;

    ULONGLONG ulDiff = ulTimeTo.QuadPart - ulTimeFrom.QuadPart;

    float diff = (float)ulDiff;  // number of 100-nanosecond intervals
    Test_LogFormat('i', _T("Time spent: %.3f seconds"), diff / 10000000.0);

    BOOL result = Test_LogSummary();

    return result ? 0 : 10;
}
