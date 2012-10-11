/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

// Emulator.cpp

#include "stdafx.h"
#include <stdio.h>
#include <Share.h>
#include "Emulator.h"
#include "emubase\Emubase.h"
#include "util/WavPcmFile.h"

//NOTE: I know, we use unsafe string functions
#pragma warning( disable: 4996 )


//////////////////////////////////////////////////////////////////////


CMotherboard* g_pBoard = NULL;

BOOL g_okEmulatorInitialized = FALSE;
BOOL g_okEmulatorRunning = FALSE;

WORD m_wEmulatorCPUBreakpoint = 0177777;
WORD m_wEmulatorPPUBreakpoint = 0177777;

//BOOL m_okEmulatorSound = FALSE;

//BOOL m_okEmulatorSerial = FALSE;
//HANDLE m_hEmulatorComPort = INVALID_HANDLE_VALUE;

//BOOL m_okEmulatorParallel = FALSE;
//FILE* m_fpEmulatorParallelOut = NULL;

//long m_nFrameCount = 0;
DWORD m_dwTickCount = 0;
DWORD m_dwEmulatorUptime = 0;  // UKNC uptime, seconds, from turn on or reset, increments every 25 frames
long m_nUptimeFrameCount = 0;

HWAVPCMFILE m_hTapeWavPcmFile = (HWAVPCMFILE) INVALID_HANDLE_VALUE;
#define TAPE_BUFFER_SIZE 624
BYTE m_TapeBuffer[TAPE_BUFFER_SIZE];
BOOL CALLBACK Emulator_TapeReadCallback(UINT samples);
BOOL CALLBACK Emulator_TapeWriteCallback(UINT samples);


const DWORD ScreenView_StandardRGBColors[16] = {
    0x000000, 0x000080, 0x008000, 0x008080, 0x800000, 0x800080, 0x808000, 0x808080,
    0x000000, 0x0000FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF,
};


//////////////////////////////////////////////////////////////////////


const LPCTSTR FILE_NAME_UKNC_ROM = _T("uknc_rom.bin");

BOOL Emulator_Init()
{
    ASSERT(g_pBoard == NULL);

    CProcessor::Init();

    g_pBoard = new CMotherboard();

    BYTE buffer[32768];
    size_t dwBytesRead;

    // Load ROM file
    memset(buffer, 0, 32768);
    FILE* fpRomFile = ::_tfsopen(FILE_NAME_UKNC_ROM, _T("rb"), _SH_DENYWR);
    if (fpRomFile == NULL)
    {
        AlertWarning(_T("Failed to load ROM file."));
        return false;
    }
    dwBytesRead = ::fread(buffer, 1, 32256, fpRomFile);
    ASSERT(dwBytesRead == 32256);
    ::fclose(fpRomFile);

    g_pBoard->LoadROM(buffer);

    g_pBoard->Reset();

    //if (m_okEmulatorSound)
    //{
    //    SoundGen_Initialize(Settings_GetSoundVolume());
    //    g_pBoard->SetSoundGenCallback(SoundGen_FeedDAC);
    //}

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    //// Allocate memory for old RAM values
    //for (int i = 0; i < 3; i++)
    //{
    //    g_pEmulatorRam[i] = (BYTE*) ::malloc(65536);  memset(g_pEmulatorRam[i], 0, 65536);
    //    g_pEmulatorChangedRam[i] = (BYTE*) ::malloc(65536);  memset(g_pEmulatorChangedRam[i], 0, 65536);
    //}

    g_okEmulatorInitialized = TRUE;
    return TRUE;
}

void Emulator_Done()
{
    ASSERT(g_pBoard != NULL);

    CProcessor::Done();

    //g_pBoard->SetSoundGenCallback(NULL);
    //SoundGen_Finalize();

    //g_pBoard->SetSerialCallbacks(NULL, NULL);
    //if (m_hEmulatorComPort != INVALID_HANDLE_VALUE)
    //{
    //    ::CloseHandle(m_hEmulatorComPort);
    //    m_hEmulatorComPort = INVALID_HANDLE_VALUE;
    //}

    delete g_pBoard;
    g_pBoard = NULL;

    //// Free memory used for old RAM values
    //for (int i = 0; i < 3; i++)
    //{
    //    ::free(g_pEmulatorRam[i]);
    //    ::free(g_pEmulatorChangedRam[i]);
    //}

    g_okEmulatorInitialized = FALSE;
}

void Emulator_Start()
{
    g_okEmulatorRunning = TRUE;

    //// Set title bar text
    //SetWindowText(g_hwnd, _T("UKNC Back to Life [run]"));
    //MainWindow_UpdateMenu();

    //m_nFrameCount = 0;
    //m_dwTickCount = GetTickCount();
}
void Emulator_Stop()
{
    g_okEmulatorRunning = FALSE;
    m_wEmulatorCPUBreakpoint = 0177777;
    m_wEmulatorPPUBreakpoint = 0177777;

    //if (m_fpEmulatorParallelOut != NULL)
    //    ::fflush(m_fpEmulatorParallelOut);

    //// Reset title bar message
    //SetWindowText(g_hwnd, _T("UKNC Back to Life [stop]"));
    //MainWindow_UpdateMenu();
    //// Reset FPS indicator
    //MainWindow_SetStatusbarText(StatusbarPartFPS, _T(""));

    //MainWindow_UpdateAllViews();
}

void Emulator_Reset()
{
    ASSERT(g_pBoard != NULL);

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    //m_dwEmulatorUptime = 0;

    //MainWindow_UpdateAllViews();
}

void Emulator_SetCPUBreakpoint(WORD address)
{
    m_wEmulatorCPUBreakpoint = address;
}
void Emulator_SetPPUBreakpoint(WORD address)
{
    m_wEmulatorPPUBreakpoint = address;
}
BOOL Emulator_IsBreakpoint()
{
    WORD wCPUAddr = g_pBoard->GetCPU()->GetPC();
    if (wCPUAddr == m_wEmulatorCPUBreakpoint)
        return TRUE;
    WORD wPPUAddr = g_pBoard->GetPPU()->GetPC();
    if (wPPUAddr == m_wEmulatorPPUBreakpoint)
        return TRUE;
    return FALSE;
}

int Emulator_SystemFrame()
{
    //SoundGen_SetVolume(Settings_GetSoundVolume());

    g_pBoard->SetCPUBreakpoint(m_wEmulatorCPUBreakpoint);
    g_pBoard->SetPPUBreakpoint(m_wEmulatorPPUBreakpoint);

    //ScreenView_ScanKeyboard();
    
    if (!g_pBoard->SystemFrame())
        return 0;

    //// Calculate frames per second
    //m_nFrameCount++;
    //DWORD dwCurrentTicks = GetTickCount();
    //long nTicksElapsed = dwCurrentTicks - m_dwTickCount;
    //if (nTicksElapsed >= 1200)
    //{
    //    double dFramesPerSecond = m_nFrameCount * 1000.0 / nTicksElapsed;
    //    double dSpeed = dFramesPerSecond / 25.0 * 100;
    //    TCHAR buffer[16];
    //    _stprintf(buffer, _T("%03.f%%"), dSpeed);
    //    MainWindow_SetStatusbarText(StatusbarPartFPS, buffer);

    //    m_nFrameCount = 0;
    //    m_dwTickCount = dwCurrentTicks;
    //}

    // Calculate emulator uptime (25 frames per second)
    m_nUptimeFrameCount++;
    if (m_nUptimeFrameCount >= 25)
    {
        m_dwEmulatorUptime++;
        m_nUptimeFrameCount = 0;

        //int seconds = (int) (m_dwEmulatorUptime % 60);
        //int minutes = (int) (m_dwEmulatorUptime / 60 % 60);
        //int hours   = (int) (m_dwEmulatorUptime / 3600 % 60);

        //TCHAR buffer[20];
        //_stprintf(buffer, _T("Uptime: %02d:%02d:%02d"), hours, minutes, seconds);
        //MainWindow_SetStatusbarText(StatusbarPartUptime, buffer);
    }

    return 1;
}

DWORD Emulator_GetUptime()
{
    return m_dwEmulatorUptime;
}

BOOL Emulator_Run(int frames)
{
    for (int i = 0; i < frames; i++)
    {
        int res = Emulator_SystemFrame();
        if (!res)
            return FALSE;
    }

    return TRUE;
}

BOOL Emulator_LoadROMCartridge(int slot, LPCTSTR sFilePath)
{
    // Open file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("rb"), _SH_DENYWR);
    if (fpFile == INVALID_HANDLE_VALUE)
    {
        AlertWarning(_T("Failed to load ROM cartridge image."));
        return FALSE;
    }

    // Allocate memory
    BYTE* pImage = (BYTE*) ::malloc(24 * 1024);

    size_t dwBytesRead = ::fread(pImage, 1, 24 * 1024, fpFile);
    ASSERT(dwBytesRead == 24 * 1024);

    g_pBoard->LoadROMCartridge(slot, pImage);

    // Free memory, close file
    ::free(pImage);
    ::fclose(fpFile);

    return TRUE;
}

BOOL Emulator_AttachFloppyImage(int slot, LPCTSTR sFilePath)
{
    return g_pBoard->AttachFloppyImage(slot, sFilePath);
}

BOOL Emulator_AttachHardImage(int slot, LPCTSTR sFilePath)
{
    return g_pBoard->AttachHardImage(slot, sFilePath);
}

// Tape emulator callback used to read a tape recorded data.
// Input:
//   samples    Number of samples to play.
// Output:
//   result     Bit to put in tape input port.
BOOL CALLBACK Emulator_TapeReadCallback(unsigned int samples)
{
	if (samples == 0) return 0;

    // Scroll buffer
    memmove(m_TapeBuffer, m_TapeBuffer + samples, TAPE_BUFFER_SIZE - samples);

	UINT value = 0;
	for (UINT i = 0; i < samples; i++)
	{
		value = WavPcmFile_ReadOne(m_hTapeWavPcmFile);
        *(m_TapeBuffer + TAPE_BUFFER_SIZE - samples + i) = (BYTE)((value >> 24) & 0xff);
	}
	BOOL result = (value >= UINT_MAX / 2);
	return result;
}

void CALLBACK Emulator_TapeWriteCallback(int value, UINT samples)
{
    if (samples == 0) return;

    // Scroll buffer
    memmove(m_TapeBuffer, m_TapeBuffer + samples, TAPE_BUFFER_SIZE - samples);

    // Write samples to the file
    for (UINT i = 0; i < samples; i++)
    {
        WavPcmFile_WriteOne(m_hTapeWavPcmFile, value);
        //TODO: Check WavPcmFile_WriteOne result
        *(m_TapeBuffer + TAPE_BUFFER_SIZE - samples + i) = (BYTE)((value >> 24) & 0xff);
    }
}

BOOL Emulator_OpenTape(LPCTSTR sFilePath)
{
	m_hTapeWavPcmFile = WavPcmFile_Open(sFilePath);
	if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
		return FALSE;

    int sampleRate = WavPcmFile_GetFrequency(m_hTapeWavPcmFile);
    g_pBoard->SetTapeReadCallback(Emulator_TapeReadCallback, sampleRate);

    return TRUE;
}

BOOL Emulator_CreateTape(LPCTSTR sFilePath)
{
	m_hTapeWavPcmFile = WavPcmFile_Create(sFilePath, 22050);
	if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
		return FALSE;

    int sampleRate = WavPcmFile_GetFrequency(m_hTapeWavPcmFile);
    g_pBoard->SetTapeWriteCallback(Emulator_TapeWriteCallback, sampleRate);

    return TRUE;
}

void Emulator_CloseTape()
{
    g_pBoard->SetTapeReadCallback(NULL, 0);
    g_pBoard->SetTapeWriteCallback(NULL, 0);

    WavPcmFile_Close(m_hTapeWavPcmFile);
	m_hTapeWavPcmFile = (HWAVPCMFILE) INVALID_HANDLE_VALUE;
}

void Emulator_PrepareScreenRGB32(void* pImageBits, const DWORD* colors)
{
    if (pImageBits == NULL) return;
    if (!g_okEmulatorInitialized) return;

    // Tag parsing loop
    BYTE cursorYRGB;
    BOOL okCursorType;
    BYTE cursorPos = 128;
    BOOL cursorOn = FALSE;
    BYTE cursorAddress;      // Address of graphical cursor
    WORD address = 0000270;  // Tag sequence start address
    BOOL okTagSize = FALSE;  // Tag size: TRUE - 4-word, FALSE - 2-word (first tag is always 2-word)
    BOOL okTagType = FALSE;  // Type of 4-word tag: TRUE - set palette, FALSE - set params
    int scale = 1;           // Horizontal scale: 1, 2, 4, or 8
    DWORD palette = 0;       // Palette
    DWORD palettecurrent[8];  memset(palettecurrent, 0, sizeof(palettecurrent)); // Current palette; update each time we change the "palette" variable
	BYTE pbpgpr = 7;         // 3-bit Y-value modifier
    for (int yy = 0; yy < 307; yy++)
    {
        if (okTagSize) {  // 4-word tag
            WORD tag1 = g_pBoard->GetRAMWord(0, address);
            address += 2;
            WORD tag2 = g_pBoard->GetRAMWord(0, address);
            address += 2;

            if (okTagType)  // 4-word palette tag
            {
                palette = MAKELONG(tag1, tag2);
                for (BYTE c = 0; c < 8; c++)  // Update palettecurrent
                {
                    BYTE valueYRGB = (BYTE) (palette >> (c << 2)) & 15;
                    palettecurrent[c] = colors[valueYRGB];
                }
            }
            else  // 4-word params tag
            {
                scale = (tag2 >> 4) & 3;  // Bits 4-5 - new scale value
                pbpgpr = tag2 & 7;  // Y-value modifier
                cursorYRGB = tag1 & 15;  // Cursor color
                okCursorType = ((tag1 & 16) != 0);  // TRUE - graphical cursor, FALSE - symbolic cursor
                ASSERT(okCursorType==0);  //DEBUG
                cursorPos = ((tag1 >> 8) >> scale) & 0x7f;  // Cursor position in the line
                cursorAddress = (tag1 >> 5) & 7;
                scale = 1 << scale;
            }
        }

        WORD addressBits = g_pBoard->GetRAMWord(0, address);  // The word before the last word - is address of bits from all three memory planes
        address += 2;

        // Calculate size, type and address of the next tag
        WORD tagB = g_pBoard->GetRAMWord(0, address);  // Last word of the tag - is address and type of the next tag
        okTagSize = (tagB & 2) != 0;  // Bit 1 shows size of the next tag
        if (okTagSize)
        {
            address = tagB & ~7;
            okTagType = (tagB & 4) != 0;  // Bit 2 shows type of the next tag
        }
        else
            address = tagB & ~3;
        if ((tagB & 1) != 0)
            cursorOn = !cursorOn;

        // Draw bits into the bitmap, from line 20 to line 307
        if (yy >= 19 && yy <= 306)
        {
            // Loop thru bits from addressBits, planes 0,1,2
            // For each pixel:
            //   Get bit from planes 0,1,2 and make value
            //   Map value to palette; result is 4-bit value YRGB
            //   Translate value to 24-bit RGB
            //   Put value to m_bits; repeat using scale value

            int xr = 640;
            int y = yy - 19;
            DWORD* pBits = ((DWORD*)pImageBits) + (288 - 1 - y) * 640;
            for (int pos = 0; ; pos++)
            {
                // Get bit from planes 0,1,2
                BYTE src0 = g_pBoard->GetRAMByte(0, addressBits);
                BYTE src1 = g_pBoard->GetRAMByte(1, addressBits);
                BYTE src2 = g_pBoard->GetRAMByte(2, addressBits);
                // Loop through the bits of the byte
                int bit = 0;
                while (true)
                {
                    DWORD valueRGB;
                    if (cursorOn && (pos == cursorPos) && (!okCursorType || (okCursorType && bit == cursorAddress)))
                        valueRGB = colors[cursorYRGB];  // 4-bit to 32-bit color
                    else
					{
	                    // Make 3-bit value from the bits
						BYTE value012 = (src0 & 1) | ((src1 & 1) << 1) | ((src2 & 1) << 2);
                        valueRGB = palettecurrent[value012];  // 3-bit to 32-bit color
					}

                    // Put value to m_bits; repeat using scale value
                    switch (scale)
                    {
                    case 8:
                        *pBits++ = valueRGB;
                        *pBits++ = valueRGB;
                        *pBits++ = valueRGB;
                        *pBits++ = valueRGB;
                    case 4:
                        *pBits++ = valueRGB;
                        *pBits++ = valueRGB;
                    case 2:
                        *pBits++ = valueRGB;
                    case 1:
                        *pBits++ = valueRGB;
                    default:
                        break;
                    }
                    //WAS: for (int s = 0; s < scale; s++) *pBits++ = valueRGB;

                    xr -= scale;

                    if (bit == 7)
                        break;
                    bit++;

                    // Shift to the next bit
                    src0 = src0 >> 1;
                    src1 = src1 >> 1;
                    src2 = src2 >> 1;
                }
                if (xr <= 0)
                    break;  // End of line
                addressBits++;  // Go to the next byte
            }
        }
    }
}

BOOL Emulator_SaveScreenshot(LPCTSTR sFileName, const DWORD * bits)
{
    ASSERT(sFileName != NULL);
    ASSERT(bits != NULL);

    // Create file
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    BITMAPFILEHEADER hdr;
    ::ZeroMemory(&hdr, sizeof(hdr));
    hdr.bfType = 0x4d42;  // "BM"
    BITMAPINFOHEADER bih;
    ::ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = 640;
    bih.biHeight = 288;
    bih.biSizeImage = bih.biWidth * bih.biHeight / 2;
    bih.biPlanes = 1;
    bih.biBitCount = 4;
    bih.biCompression = BI_RGB;
    bih.biXPelsPerMeter = bih.biXPelsPerMeter = 2000;
    hdr.bfSize = (DWORD) sizeof(BITMAPFILEHEADER) + bih.biSize + bih.biSizeImage;
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + bih.biSize + sizeof(RGBQUAD) * 16;

    DWORD dwBytesWritten = 0;

    BYTE * pData = (BYTE *) ::malloc(bih.biSizeImage);

    // Prepare the image data
    const DWORD * psrc = bits;
    BYTE * pdst = pData;
    const DWORD * palette = ScreenView_StandardRGBColors;
    for (int i = 0; i < 640 * 288; i++)
    {
        DWORD rgb = *psrc;
        psrc++;
        BYTE color = 0;
        for (BYTE c = 0; c < 16; c++)
        {
            if (palette[c] == rgb)
            {
                color = c;
                break;
            }
        }
        if ((i & 1) == 0)
            *pdst = (color << 4);
        else
        {
            *pdst = (*pdst) & 0xf0 | color;
            pdst++;
        }
    }

    WriteFile(hFile, &hdr, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    if (dwBytesWritten != sizeof(BITMAPFILEHEADER))
    {
        ::free(pData);
        return FALSE;
    }
    WriteFile(hFile, &bih, bih.biSize, &dwBytesWritten, NULL);
    if (dwBytesWritten != sizeof(BITMAPINFOHEADER))
    {
        ::free(pData);
        return FALSE;
    }
    WriteFile(hFile, palette, sizeof(RGBQUAD) * 16, &dwBytesWritten, NULL);
    if (dwBytesWritten != sizeof(RGBQUAD) * 16)
    {
        ::free(pData);
        return FALSE;
    }
    WriteFile(hFile, pData, bih.biSizeImage, &dwBytesWritten, NULL);
    ::free(pData);
    if (dwBytesWritten != bih.biSizeImage)
        return FALSE;

    // Close file
    CloseHandle(hFile);

    return TRUE;
}

BOOL Emulator_SaveScreenshot(LPCTSTR sFileName)
{
    DWORD * bits = (DWORD *) ::malloc(640 * 288 * 4);

    Emulator_PrepareScreenRGB32(bits, ScreenView_StandardRGBColors);

    BOOL result = Emulator_SaveScreenshot(sFileName, bits);

    ::free(bits);

    return result;
}

BOOL Emulator_SaveApngFrame(HAPNGFILE hFile)
{
    DWORD * bits = (DWORD *) ::malloc(640 * 288 * 4);

    Emulator_PrepareScreenRGB32(bits, ScreenView_StandardRGBColors);

    BOOL result = ApngFile_WriteFrame(hFile, bits, ScreenView_StandardRGBColors);

    ::free(bits);

    return result;
}


// Returns: amount of different pixels
int Emulator_CompareScreens(const DWORD * scr1, const DWORD * scr2)
{
    const DWORD * p1 = scr1;
    const DWORD * p2 = scr2;

    int result = 0;
    for (int i = 640 * 288; i > 0; i--)
    {
        if (*p1 != *p2)
            result++;
        p1++;  p2++;
    }

    return result;
}

// Returns: amount of different pixels
int Emulator_CheckScreenshot(LPCTSTR sFileName, const DWORD * bits, DWORD * tempbits)
{
    ASSERT(sFileName != NULL);
    ASSERT(bits != NULL);

    // Open file for reading
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    BITMAPFILEHEADER hdr;
    BITMAPINFOHEADER bih;
    DWORD dwBytesRead = 0;

    ReadFile(hFile, &hdr, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL);
    if (dwBytesRead != sizeof(BITMAPFILEHEADER))
        return -1;
    //TODO: Check the header
    ReadFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwBytesRead, NULL);
    if (dwBytesRead != sizeof(BITMAPINFOHEADER))
        return -1;
    //TODO: Check the header
    if (bih.biSizeImage != 640 * 288 / 2)
        return -1;
    // Skip the palette
    SetFilePointer(hFile, sizeof(RGBQUAD) * 16, 0, FILE_CURRENT);

    BYTE * pData = (BYTE *) ::malloc(bih.biSizeImage);

    ReadFile(hFile, pData, bih.biSizeImage, &dwBytesRead, NULL);
    if (dwBytesRead != bih.biSizeImage)
    {
        ::free(pData);
        return -1;
    }

    // Decode the image data
    BYTE * psrc = pData;
    DWORD * pdst = tempbits;
    for (int i = 0; i < 640 * 288; i++)
    {
        BYTE color = 0;
        if ((i & 1) == 0)
            color = (*psrc) >> 4;
        else
        {
            color = (*psrc) & 15;
            psrc++;
        }
        *pdst = ScreenView_StandardRGBColors[color];
        pdst++;
    }

    ::free(pData);

    // Compare the screenshots
    int result = Emulator_CompareScreens(bits, tempbits);

    // Close file
    CloseHandle(hFile);

    return result;
}

int Emulator_CheckScreenshot(LPCTSTR sFileName)
{
    DWORD * bits = (DWORD *) ::malloc(640 * 288 * 4);
    DWORD * tempbits = (DWORD *) ::malloc(640 * 288 * 4);

    Emulator_PrepareScreenRGB32(bits, ScreenView_StandardRGBColors);

    int result = Emulator_CheckScreenshot(sFileName, bits, tempbits);

    ::free(tempbits);
    ::free(bits);

    return result;
}

void Emulator_KeyboardPressRelease(BYTE ukncscan, int timeout)
{
	g_pBoard->KeyboardEvent(ukncscan, TRUE);
    Emulator_Run(timeout);
	g_pBoard->KeyboardEvent(ukncscan, FALSE);
    Emulator_Run(3);
}

const BYTE arrChar2UkncScan[256] = {
/*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
/*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0153, 0000, 0000, 0153, 0000, 0000, 
/*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*2*/    0113, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0117, 0175, 0146, 0173, 
/*3*/    0176, 0030, 0031, 0032, 0013, 0034, 0035, 0016, 0017, 0177, 0174, 0007, 0000, 0000, 0000, 0000, 
/*4*/    0000, 0072, 0076, 0050, 0057, 0033, 0047, 0055, 0156, 0073, 0027, 0052, 0056, 0112, 0054, 0075, 
/*5*/    0053, 0067, 0074, 0111, 0114, 0051, 0137, 0071, 0115, 0070, 0157, 0000, 0000, 0106, 0110, 0000, 
/*6*/    0126, 0127, 0147, 0167, 0130, 0150, 0170, 0125, 0145, 0165, 0025, 0000, 0000, 0005, 0146, 0131, 
/*7*/    0010, 0011, 0012, 0014, 0015, 0172, 0152, 0151, 0171, 0000, 0004, 0155, 0000, 0000, 0000, 0000, 
/*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*c*/    0007, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
};

const BYTE arrChar2UkncScanShift[256] = {
/*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
/*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*2*/    0000, 0000, 0031, 0000, 0013, 0000, 0000, 0000, 0017, 0177, 0174, 0000, 0000, 0000, 0000, 0000, 
/*3*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0175, 0000, 0000,
/*4*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*5*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*6*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*c*/    0007, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
/*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
};

void Emulator_KeyboardPressReleaseChar(char ch, int timeout)
{
    BYTE scan = arrChar2UkncScanShift[(BYTE)ch];
    if (scan != 0)
    {
        Emulator_KeyboardPressReleaseShift(scan);
        return;
    }

    scan = arrChar2UkncScan[(BYTE)ch];
    if (scan == 0)
        return;
    Emulator_KeyboardPressRelease(scan, timeout);
}

void Emulator_KeyboardSequence(const char * str)
{
    const char * p = str;
    while (*p != 0)
    {
        Emulator_KeyboardPressReleaseChar(*p);
        p++;
    }
}

void Emulator_KeyboardPressReleaseShift(BYTE ukncscan)
{
	g_pBoard->KeyboardEvent(0105, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, FALSE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(0105, FALSE);
    Emulator_Run(3);
}

void Emulator_KeyboardPressReleaseAlt(BYTE ukncscan)
{
	g_pBoard->KeyboardEvent(0107, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, FALSE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(0107, FALSE);
    Emulator_Run(3);
}

void Emulator_KeyboardPressReleaseCtrl(BYTE ukncscan)
{
	g_pBoard->KeyboardEvent(046, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, TRUE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(ukncscan, FALSE);
    Emulator_Run(3);
	g_pBoard->KeyboardEvent(046, FALSE);
    Emulator_Run(3);
}


//////////////////////////////////////////////////////////////////////


BOOL Emulator_SaveImage(LPCTSTR sFilePath)
{
    // Create file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("w+b"), _SH_DENYWR);
    if (fpFile == NULL)
        return FALSE;

    // Allocate memory
    BYTE* pImage = (BYTE*) ::malloc(UKNCIMAGE_SIZE);
    memset(pImage, 0, UKNCIMAGE_SIZE);
    // Prepare header
    DWORD* pHeader = (DWORD*) pImage;
    *pHeader++ = UKNCIMAGE_HEADER1;
    *pHeader++ = UKNCIMAGE_HEADER2;
    *pHeader++ = UKNCIMAGE_VERSION;
    *pHeader++ = UKNCIMAGE_SIZE;
    // Store emulator state to the image
    g_pBoard->SaveToImage(pImage);
    *(DWORD*)(pImage + 16) = m_dwEmulatorUptime;

    // Save image to the file
    size_t dwBytesWritten = ::fwrite(pImage, 1, UKNCIMAGE_SIZE, fpFile);
    //TODO: Check if dwBytesWritten != UKNCIMAGE_SIZE

    // Free memory, close file
    ::free(pImage);
    ::fclose(fpFile);

    return TRUE;
}

BOOL Emulator_LoadImage(LPCTSTR sFilePath)
{
    // Open file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("rb"), _SH_DENYWR);
    if (fpFile == NULL)
        return FALSE;

    Emulator_Stop();

    // Read header
    DWORD bufHeader[UKNCIMAGE_HEADER_SIZE / sizeof(DWORD)];
    size_t dwBytesRead = ::fread(bufHeader, 1, UKNCIMAGE_HEADER_SIZE, fpFile);
    //TODO: Check if dwBytesRead != UKNCIMAGE_HEADER_SIZE
    
    //TODO: Check version and size

    // Allocate memory
    BYTE* pImage = (BYTE*) ::malloc(UKNCIMAGE_SIZE);

    // Read image
    ::fseek(fpFile, 0, SEEK_SET);
    dwBytesRead = ::fread(pImage, 1, UKNCIMAGE_SIZE, fpFile);
    //TODO: Check if dwBytesRead != UKNCIMAGE_SIZE

    // Restore emulator state from the image
    g_pBoard->LoadFromImage(pImage);

    m_dwEmulatorUptime = *(DWORD*)(pImage + 16);

    // Free memory, close file
    ::free(pImage);
    ::fclose(fpFile);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
