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
            Test_LogFormat('i', _T("    %06o-%06o"), start, start+length-1);
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

    Emulator_Run(75);  // Boot: 3 seconds

    Test_CheckScreenshot(_T("data\\test01_1.bmp"));  // Boot menu

    Emulator_KeyboardPressRelease(0152);  // "УСТ"
    Emulator_Run(5);
    Test_CheckScreenshot(_T("data\\test01_2.bmp"));  // Settings menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Test_CheckScreenshot(_T("data\\test01_3.bmp"));  // Settings menu: colors
    Emulator_KeyboardPressRelease(0151);  // "ИСП"
    Emulator_Run(5);

    Emulator_KeyboardSequence("7\n");  // Select "7 - тестирование", Enter

    Emulator_Run(20);
    Test_CheckScreenshot(_T("data\\test01_4.bmp"));  // Self test pass 1
    Emulator_Run(25 * 200);
    Test_CheckScreenshot(_T("data\\test01_5.bmp"));  // Self test pass 2

    Test_Done();
}

void Test2_Basic()
{
    Test_Init(_T("TEST 2: BASIC"));

    Test_LoadROMCartridge(1, _T("romctr_basic.bin"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("2\n");  // Select boot from the cartridge
    Emulator_Run(100);  // Boot BASIC: 5 seconds
    Test_CheckScreenshot(_T("data\\test02_1.bmp"));

    Emulator_KeyboardSequence("PRINT PI\n");

    Emulator_KeyboardSequence("10 FOR I=32 TO 255\n");
    Emulator_KeyboardSequence("20 PRINT CHR$(I);\n");
    Emulator_KeyboardSequence("30 IF I MOD 16 = 15 THEN PRINT\n");
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
    Emulator_Run(143);

    Test_SaveScreenshot(_T("test02_6.bmp"));

    Emulator_KeyboardSequence("NEW\n");
    Emulator_Run(10);
    Emulator_KeyboardSequence("1  !\"#$%&\'()*+,-./\n");
    Emulator_KeyboardSequence("2 0123456789:;<=>?\n");
    Emulator_KeyboardSequence("3 @[\\]^_ `{|}~\n");
    Emulator_KeyboardSequence("4 ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
    Emulator_KeyboardSequence("5 abcdefghijklmnopqrstuvwxyz\n");
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
    Emulator_Run(175);
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
    Emulator_Run(3950);
    Test_CheckScreenshot(_T("data\\test02_rnd2.bmp"));
    //Test_SaveScreenshotSeria(_T("video\\test02_%04u.bmp"), 20, 25);

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
    Emulator_KeyboardPressRelease(0152);  // "УСТ"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('2');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "ИСП"
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
    Emulator_KeyboardPressRelease(0152);  // "УСТ"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('1');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "ИСП"

    Emulator_Run(250);
    Test_SaveScreenshot(_T("test03_speed.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot from the disk: 8 seconds
    Emulator_KeyboardSequence("IOSCAN\n");  // Run I/O port scanner
    Emulator_Run(50);
    Test_SaveScreenshot(_T("test03_ioscan.bmp"));

    Emulator_KeyboardSequence("SPEED\n");
    Emulator_Run(550);

    // Turn off the timer
    Emulator_KeyboardPressRelease(0152);  // "УСТ"
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('8');  // Timer
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0133);  // Right arrow
    Emulator_Run(5);
    Emulator_KeyboardPressReleaseChar('2');  // Off
    Emulator_Run(5);
    Emulator_KeyboardPressRelease(0151);  // "ИСП"
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
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test04_9.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:GARDEN\n");
    Emulator_Run(210);
    Test_CheckScreenshot(_T("data\\test04_10.bmp"));
    Emulator_KeyboardSequence(" ");
    Emulator_Run(75);
    Test_CheckScreenshot(_T("data\\test04_11.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:CAT\n");
    Emulator_Run(250);
    //NOTE: Непонятно в чём дело, не идёт дальше (переподключение дисков НЕ помогает)
    Test_SaveScreenshot(_T("test04_12.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");
    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("RU MZ1:LAND\n");
    Emulator_Run(200);
    Test_CheckScreenshot(_T("data\\test04_14.bmp"));
    Emulator_Run(10);
    Emulator_KeyboardPressReleaseChar(' ', 15);
    Emulator_Run(100);
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
    Emulator_Run(33*25);
    Test_CheckScreenshot(_T("data\\test04_22.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(6*25);
    Test_CheckScreenshot(_T("data\\test04_23.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5*25);
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
    Emulator_Run(150);
    Test_CheckScreenshot(_T("data\\test05_0.bmp"));

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

    // Load TESTGR.BAS and run, see http://zx.pk.ru/showpost.php?p=420453&postcount=238
    Emulator_KeyboardPressRelease(0012);  // "K3"
    Emulator_Run(250);
    // Здесь была проблема: "Диалог не заполняется как надо, сколько ни жди; в эмуляторе работает нормально"; исправилось в r397
    Test_CheckScreenshot(_T("data\\test06_02.bmp"));
    Emulator_KeyboardSequence("TESTGR\n");
    Emulator_Run(150);
    Test_CheckScreenshot(_T("data\\test06_03.bmp"));
    Emulator_KeyboardPressReleaseAlt(0171);  // Alt+F9 -- Compile
    Emulator_Run(25 * 9);
    Test_CheckScreenshot(_T("data\\test06_04.bmp"));
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(50);
    Emulator_KeyboardPressReleaseCtrl(0171);  // Ctrl+F9 -- Run
    Emulator_Run(25 * 22);
    Test_CheckScreenshot(_T("data\\test06_05.bmp"));  // Title screen
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(50);
    Test_CheckScreenshot(_T("data\\test06_06.bmp"));  // Menu screen
    Emulator_KeyboardPressRelease(0154, 40);  // "Up arrow"
    Emulator_KeyboardPressReleaseChar('\n');  // Select "All demonstration"
    Emulator_Run(25 * 15);
    Test_CheckScreenshot(_T("data\\test06_07.bmp"));  // Circles
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 23);
    Test_CheckScreenshot(_T("data\\test06_08.bmp"));
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(5 * 23);
    Test_CheckScreenshot(_T("data\\test06_09.bmp"));  // Circles
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 20);
    Test_CheckScreenshot(_T("data\\test06_10.bmp"));  // Blocks
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(260);
    Test_CheckScreenshot(_T("data\\test06_11.bmp"));  // Blocks
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 23);
    Test_CheckScreenshot(_T("data\\test06_12.bmp"));  // Line
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(75);
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 15);
    Test_CheckScreenshot(_T("data\\test06_14.bmp"));  // Lines
    Emulator_KeyboardPressReleaseChar(' ');
    Emulator_Run(5 * 9);
    Test_CheckScreenshot(_T("data\\test06_15.bmp"));  // Pages
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(5 * 44);
    Test_CheckScreenshot(_T("data\\test06_16.bmp"));  // Space frogs
    Emulator_KeyboardPressReleaseChar(' ', 6);
    Emulator_Run(100);
    Test_CheckScreenshot(_T("data\\test06_17.bmp"));

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
    Emulator_Run(1500);
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
    Emulator_Run(550);
    // Здесь была проблама: "Не загружается, сколько не жди, но передёргивание дисковода помогает"; исправилось в r397
    Test_CheckScreenshot(_T("data\\test10_05.bmp"));
    Emulator_KeyboardPressRelease(0153, 6);  // "Enter" on the title screen
    Emulator_Run(1000);
    //NOTE: Тут непонятно -- не появляется надпись "Conan"
    Test_SaveScreenshot(_T("test10_06.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Sammy" selected
    Emulator_Run(800);
    // Здесь была проблама: "Не загружается, сколько не жди, но передёргивание дисковода помогает"; исправилось в r397
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
    Emulator_Run(500);
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
    Emulator_Run(650);
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
    Emulator_Run(225);
    Test_CheckScreenshot(_T("data\\test10_14.bmp"));
    Emulator_Run(225);
    Test_CheckScreenshot(_T("data\\test10_15.bmp"));
    Emulator_Run(225);
    Test_CheckScreenshot(_T("data\\test10_16.bmp"));  // Ready!
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- start the game
    Emulator_Run(100);
    Test_CheckScreenshot(_T("data\\test10_17.bmp"));

    Emulator_Reset();

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(375);  // Boot and wait for menu
    Emulator_KeyboardPressRelease(0134);  // "Down arrow"
    Emulator_Run(3);
    Emulator_KeyboardPressRelease(0153);  // "Enter" -- "Arkanoid" selected
    Emulator_Run(300);
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
    Emulator_Run(550);
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
    Emulator_Run(15 * 25);
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
    // Здесь была проблама: "Не загружается, сколько не жди, но передёргивание дисковода помогает"; исправилось в r397
    Emulator_Run(200);
    //NOTE: Полученное изображение отличается от изображения на реальной машине
    Test_SaveScreenshot(_T("test12_01.bmp"));

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
    Emulator_Run(12 * 25);
    Test_CheckScreenshot(_T("data\\test13_01.bmp"));

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
    Test_Init(_T("TEST 15: Various"));

    Test_CopyFile(_T("data\\disk1.dsk"), _T("temp\\disk1.dsk"));
    Test_AttachFloppyImage(0, _T("temp\\disk1.dsk"));
    Test_CopyFile(_T("data\\various.dsk"), _T("temp\\various.dsk"));
    Test_AttachFloppyImage(1, _T("temp\\various.dsk"));

    Emulator_Run(75);  // Boot: 3 seconds
    Emulator_KeyboardSequence("1\n");
    Emulator_Run(200);  // Boot: 8 seconds
    Emulator_KeyboardSequence("01-01-99\n\n\n");  // Date
    Emulator_Run(75);  // Boot: 3 seconds
    
    // TEST SYSTEM V01.01 Трушин А. МНПП "Техноком", Зеленоград, 1992 г.
    Emulator_KeyboardSequence("RU MZ1:TS\n");
    Emulator_Run(75);
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
    Emulator_Run(100);
    //NOTE: В этом месте почему-то зависает

    //Test_SaveScreenshotSeria(_T("video\\test15_%04u.bmp"), 20, 25);

    Test_Done();
}

int _tmain(int argc, _TCHAR* argv[])
{
    SYSTEMTIME timeFrom;  ::GetLocalTime(&timeFrom);
    Test_LogInfo(_T("Initialization..."));

    Test0_ListBusDevices();
    Test1_MenuAndSelfTest();
    Test2_Basic();
    Test3_FODOSTM1();
    Test4_Games();
    Test5_Disks();
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

    Test_LogInfo(_T("Finalization..."));
    SYSTEMTIME timeTo;  ::GetLocalTime(&timeTo);
    FILETIME fileTimeFrom;
    SystemTimeToFileTime(&timeFrom, &fileTimeFrom);
    FILETIME fileTimeTo;
    SystemTimeToFileTime(&timeTo, &fileTimeTo);

    DWORD diff = fileTimeTo.dwLowDateTime - fileTimeFrom.dwLowDateTime;  // number of 100-nanosecond intervals
    Test_LogFormat('i', _T("Time spent: %.3f seconds"), (float)diff / 10000000.0);

    Test_LogSummary();

    return 0;
}
