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


CMotherboard* g_pBoard = nullptr;

bool g_okEmulatorInitialized = false;
bool g_okEmulatorRunning = false;

uint16_t m_wEmulatorCPUBreakpoint = 0177777;
uint16_t m_wEmulatorPPUBreakpoint = 0177777;

//bool m_okEmulatorSound = false;

//bool m_okEmulatorSerial = false;
//HANDLE m_hEmulatorComPort = INVALID_HANDLE_VALUE;

//bool m_okEmulatorParallel = false;
//FILE* m_fpEmulatorParallelOut = nullptr;

uint8_t* m_pEmulatorTerminalBuffer = nullptr;
int m_nEmulatorTerminalBufferSize = 0;
int m_nEmulatorTerminalBufferIndex = 0;

//long m_nFrameCount = 0;
uint32_t m_dwTickCount = 0;
uint32_t m_dwEmulatorUptime = 0;  // UKNC uptime, seconds, from turn on or reset, increments every 25 frames
long m_nUptimeFrameCount = 0;

HWAVPCMFILE m_hTapeWavPcmFile = (HWAVPCMFILE) INVALID_HANDLE_VALUE;
#define TAPE_BUFFER_SIZE 624
uint8_t m_TapeBuffer[TAPE_BUFFER_SIZE];
bool CALLBACK Emulator_TapeReadCallback(UINT samples);
bool CALLBACK Emulator_TapeWriteCallback(UINT samples);


const uint32_t ScreenView_StandardRGBColors[16 * 8] =
{
    0x000000, 0x000080, 0x008000, 0x008080, 0x800000, 0x800080, 0x808000, 0x808080,
    0x000000, 0x0000FF, 0x00FF00, 0x00FFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF,
    0x000000, 0x000060, 0x008000, 0x008060, 0x800000, 0x800060, 0x808000, 0x808060,
    0x000000, 0x0000DF, 0x00FF00, 0x00FFDF, 0xFF0000, 0xFF00DF, 0xFFFF00, 0xFFFFDF,
    0x000000, 0x000080, 0x006000, 0x006080, 0x800000, 0x800080, 0x806000, 0x806080,
    0x000000, 0x0000FF, 0x00DF00, 0x00DFFF, 0xFF0000, 0xFF00FF, 0xFFDF00, 0xFFDFFF,
    0x000000, 0x000060, 0x006000, 0x006060, 0x800000, 0x800060, 0x806000, 0x806060,
    0x000000, 0x0000DF, 0x00DF00, 0x00DFDF, 0xFF0000, 0xFF00DF, 0xFFDF00, 0xFFDFDF,
    0x000000, 0x000080, 0x008000, 0x008080, 0x600000, 0x600080, 0x608000, 0x608080,
    0x000000, 0x0000FF, 0x00FF00, 0x00FFFF, 0xDF0000, 0xDF00FF, 0xDFFF00, 0xDFFFFF,
    0x000000, 0x000060, 0x008000, 0x008060, 0x600000, 0x600060, 0x608000, 0x608060,
    0x000000, 0x0000DF, 0x00FF00, 0x00FFDF, 0xDF0000, 0xDF00DF, 0xDFFF00, 0xDFFFDF,
    0x000000, 0x000080, 0x006000, 0x006080, 0x600000, 0x600080, 0x606000, 0x606080,
    0x000000, 0x0000FF, 0x00DF00, 0x00DFFF, 0xDF0000, 0xDF00FF, 0xDFDF00, 0xDFDFFF,
    0x000000, 0x000060, 0x006000, 0x006060, 0x600000, 0x600060, 0x606000, 0x606060,
    0x000000, 0x0000DF, 0x00DF00, 0x00DFDF, 0xDF0000, 0xDF00DF, 0xDFDF00, 0xDFDFDF,
};


//////////////////////////////////////////////////////////////////////


const LPCTSTR FILE_NAME_UKNC_ROM = _T("uknc_rom.bin");

bool Emulator_Init()
{
    ASSERT(g_pBoard == nullptr);

    CProcessor::Init();

    g_pBoard = new CMotherboard();

    uint8_t buffer[32768];
    size_t dwBytesRead;

    // Load ROM file
    memset(buffer, 0, 32768);
    FILE* fpRomFile = ::_tfsopen(FILE_NAME_UKNC_ROM, _T("rb"), _SH_DENYWR);
    if (fpRomFile == nullptr)
    {
        AlertWarning(_T("Failed to load UKNC ROM image."));
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
    //    g_pEmulatorRam[i] = (uint8_t*) ::malloc(65536);  memset(g_pEmulatorRam[i], 0, 65536);
    //    g_pEmulatorChangedRam[i] = (uint8_t*) ::malloc(65536);  memset(g_pEmulatorChangedRam[i], 0, 65536);
    //}

    g_okEmulatorInitialized = true;
    return true;
}

void Emulator_Done()
{
    ASSERT(g_pBoard != nullptr);

    CProcessor::Done();

    //g_pBoard->SetSoundGenCallback(nullptr);
    //SoundGen_Finalize();

    //g_pBoard->SetSerialCallbacks(nullptr, nullptr);
    //if (m_hEmulatorComPort != INVALID_HANDLE_VALUE)
    //{
    //    ::CloseHandle(m_hEmulatorComPort);
    //    m_hEmulatorComPort = INVALID_HANDLE_VALUE;
    //}

    delete g_pBoard;
    g_pBoard = nullptr;

    //// Free memory used for old RAM values
    //for (int i = 0; i < 3; i++)
    //{
    //    ::free(g_pEmulatorRam[i]);
    //    ::free(g_pEmulatorChangedRam[i]);
    //}

    g_okEmulatorInitialized = false;
}

void Emulator_Start()
{
    g_okEmulatorRunning = true;

    //// Set title bar text
    //SetWindowText(g_hwnd, _T("UKNC Back to Life [run]"));
    //MainWindow_UpdateMenu();

    //m_nFrameCount = 0;
    //m_dwTickCount = GetTickCount();
}
void Emulator_Stop()
{
    g_okEmulatorRunning = false;
    m_wEmulatorCPUBreakpoint = 0177777;
    m_wEmulatorPPUBreakpoint = 0177777;

    //if (m_fpEmulatorParallelOut != nullptr)
    //    ::fflush(m_fpEmulatorParallelOut);

    //// Reset title bar message
    //SetWindowText(g_hwnd, _T("UKNC Back to Life [stop]"));
    //MainWindow_UpdateMenu();
    //// Reset FPS indicator
    //MainWindow_SetStatusbarText(StatusbarPartFPS, nullptr);

    //MainWindow_UpdateAllViews();
}

void Emulator_Reset()
{
    ASSERT(g_pBoard != nullptr);

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    //m_dwEmulatorUptime = 0;

    //MainWindow_UpdateAllViews();
}

void Emulator_SetCPUBreakpoint(uint16_t address)
{
    m_wEmulatorCPUBreakpoint = address;
}
void Emulator_SetPPUBreakpoint(uint16_t address)
{
    m_wEmulatorPPUBreakpoint = address;
}
bool Emulator_IsBreakpoint()
{
    uint16_t wCPUAddr = g_pBoard->GetCPU()->GetPC();
    if (wCPUAddr == m_wEmulatorCPUBreakpoint)
        return true;
    uint16_t wPPUAddr = g_pBoard->GetPPU()->GetPC();
    if (wPPUAddr == m_wEmulatorPPUBreakpoint)
        return true;
    return false;
}

bool Emulator_SystemFrame()
{
    g_pBoard->SetCPUBreakpoint(m_wEmulatorCPUBreakpoint);
    g_pBoard->SetPPUBreakpoint(m_wEmulatorPPUBreakpoint);

    if (!g_pBoard->SystemFrame())
        return false;

    // Calculate emulator uptime (25 frames per second)
    m_nUptimeFrameCount++;
    if (m_nUptimeFrameCount >= 25)
    {
        m_dwEmulatorUptime++;
        m_nUptimeFrameCount = 0;
    }

    return true;
}

uint32_t Emulator_GetUptime()
{
    return m_dwEmulatorUptime;
}

bool Emulator_Run(int frames)
{
    for (int i = 0; i < frames; i++)
    {
        int res = Emulator_SystemFrame();
        if (!res)
            return false;
    }

    return true;
}

bool Emulator_RunUntilMotorOff()
{
    for (;;)
    {
        int res = Emulator_SystemFrame();
        if (!res)
            return false;
        if (!g_pBoard->IsFloppyEngineOn())
            break;
    }
    return true;
}

bool Emulator_RunUntilCursorShown()
{
    for (;;)
    {
        uint16_t cursortiming = g_pBoard->GetRAMWord(0, 023162);
        bool cursorshown = (cursortiming & 0200) != 0;
        if (cursorshown)
            break;
        int res = Emulator_SystemFrame();
        if (!res)
            return false;
    }
    return true;
}

bool Emulator_RunAndWaitForCursor(int frames)
{
    if (!Emulator_Run(frames))
        return false;

    return Emulator_RunUntilCursorShown();
}

bool Emulator_LoadROMCartridge(int slot, LPCTSTR sFilePath)
{
    ASSERT(sFilePath != nullptr);

    // Open file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("rb"), _SH_DENYWR);
    if (fpFile == INVALID_HANDLE_VALUE)
    {
        AlertWarning(_T("Failed to load ROM cartridge image."));
        return false;
    }

    // Allocate memory
    uint8_t* pImage = (uint8_t*) ::malloc(24 * 1024);
    //TODO: Check for nullptr

    size_t dwBytesRead = ::fread(pImage, 1, 24 * 1024, fpFile);
    if (dwBytesRead != 24 * 1024)
        return false;

    g_pBoard->LoadROMCartridge(slot, pImage);

    // Free memory, close file
    ::free(pImage);
    ::fclose(fpFile);

    return true;
}

bool Emulator_AttachFloppyImage(int slot, LPCTSTR sFilePath)
{
    ASSERT(sFilePath != nullptr);

    return g_pBoard->AttachFloppyImage(slot, sFilePath);
}

bool Emulator_AttachHardImage(int slot, LPCTSTR sFilePath)
{
    ASSERT(sFilePath != nullptr);

    return g_pBoard->AttachHardImage(slot, sFilePath);
}

// Tape emulator callback used to read a tape recorded data.
// Input:
//   samples    Number of samples to play.
// Output:
//   result     Bit to put in tape input port.
bool CALLBACK Emulator_TapeReadCallback(unsigned int samples)
{
    if (samples == 0) return 0;

    // Scroll buffer
    memmove(m_TapeBuffer, m_TapeBuffer + samples, TAPE_BUFFER_SIZE - samples);

    UINT value = 0;
    for (UINT i = 0; i < samples; i++)
    {
        value = WavPcmFile_ReadOne(m_hTapeWavPcmFile);
        *(m_TapeBuffer + TAPE_BUFFER_SIZE - samples + i) = (uint8_t)((value >> 24) & 0xff);
    }
    bool result = (value >= UINT_MAX / 2);
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
        *(m_TapeBuffer + TAPE_BUFFER_SIZE - samples + i) = (uint8_t)((value >> 24) & 0xff);
    }
}

bool Emulator_OpenTape(LPCTSTR sFilePath)
{
    ASSERT(sFilePath != nullptr);

    m_hTapeWavPcmFile = WavPcmFile_Open(sFilePath);
    if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
        return false;

    int sampleRate = WavPcmFile_GetFrequency(m_hTapeWavPcmFile);
    g_pBoard->SetTapeReadCallback(Emulator_TapeReadCallback, sampleRate);

    return true;
}

bool Emulator_CreateTape(LPCTSTR sFilePath)
{
    ASSERT(sFilePath != nullptr);

    m_hTapeWavPcmFile = WavPcmFile_Create(sFilePath, 22050);
    if (m_hTapeWavPcmFile == INVALID_HANDLE_VALUE)
        return false;

    int sampleRate = WavPcmFile_GetFrequency(m_hTapeWavPcmFile);
    g_pBoard->SetTapeWriteCallback(Emulator_TapeWriteCallback, sampleRate);

    return true;
}

void Emulator_CloseTape()
{
    g_pBoard->SetTapeReadCallback(nullptr, 0);
    g_pBoard->SetTapeWriteCallback(nullptr, 0);

    WavPcmFile_Close(m_hTapeWavPcmFile);
    m_hTapeWavPcmFile = (HWAVPCMFILE) INVALID_HANDLE_VALUE;
}

void CALLBACK Emulator_TerminalCallback(uint8_t symbol)
{
    if (m_pEmulatorTerminalBuffer == NULL)
        return;
    if (m_nEmulatorTerminalBufferIndex >= m_nEmulatorTerminalBufferSize)
        return;

    m_pEmulatorTerminalBuffer[m_nEmulatorTerminalBufferIndex] = symbol;
    m_nEmulatorTerminalBufferIndex++;
    m_pEmulatorTerminalBuffer[m_nEmulatorTerminalBufferIndex] = 0;
}

const uint8_t * Emulator_GetTerminalBuffer()
{
    return m_pEmulatorTerminalBuffer;
}
void Emulator_AttachTerminalBuffer(int bufferSize)
{
    ASSERT(bufferSize > 0);

    m_pEmulatorTerminalBuffer = static_cast<uint8_t*>(::calloc(bufferSize + 1, 1));
    m_nEmulatorTerminalBufferIndex = 0;
    m_nEmulatorTerminalBufferSize = bufferSize;
    m_pEmulatorTerminalBuffer[0] = 0;

    g_pBoard->SetTerminalCallback(Emulator_TerminalCallback);
}
void Emulator_DetachTerminalBuffer()
{
    g_pBoard->SetTerminalCallback(NULL);

    if (m_pEmulatorTerminalBuffer != NULL)
    {
        ::free(m_pEmulatorTerminalBuffer);
        m_pEmulatorTerminalBuffer = NULL;
    }
}

void Emulator_PrepareScreenRGB32(void* pImageBits, const uint32_t* colors)
{
    if (pImageBits == nullptr) return;
    ASSERT(colors != nullptr);
    if (!g_okEmulatorInitialized) return;

    // Tag parsing loop
    uint8_t cursorYRGB = 0;
    bool okCursorType = 0;
    uint8_t cursorPos = 128;
    bool cursorOn = false;
    uint8_t cursorAddress = 0;  // Address of graphical cursor
    uint16_t address = 0000270;  // Tag sequence start address
    bool okTagSize = false;  // Tag size: true - 4-word, false - 2-word (first tag is always 2-word)
    bool okTagType = false;  // Type of 4-word tag: true - set palette, false - set params
    int scale = 1;           // Horizontal scale: 1, 2, 4, or 8
    uint32_t palette = 0;       // Palette
    uint32_t palettecurrent[8];  memset(palettecurrent, 0, sizeof(palettecurrent)); // Current palette; update each time we change the "palette" variable
    uint8_t pbpgpr = 0;         // 3-bit Y-value modifier
    for (int yy = 0; yy < 307; yy++)
    {
        if (okTagSize)    // 4-word tag
        {
            uint16_t tag1 = g_pBoard->GetRAMWord(0, address);
            address += 2;
            uint16_t tag2 = g_pBoard->GetRAMWord(0, address);
            address += 2;

            if (okTagType)  // 4-word palette tag
            {
                palette = MAKELONG(tag1, tag2);
            }
            else  // 4-word params tag
            {
                scale = (tag2 >> 4) & 3;  // Bits 4-5 - new scale value
                pbpgpr = (uint8_t)((7 - (tag2 & 7)) << 4);  // Y-value modifier
                cursorYRGB = (uint8_t)(tag1 & 15);  // Cursor color
                okCursorType = ((tag1 & 16) != 0);  // true - graphical cursor, false - symbolic cursor
                ASSERT(okCursorType == 0); //DEBUG
                cursorPos = (uint8_t)(((tag1 >> 8) >> scale) & 0x7f);  // Cursor position in the line
                cursorAddress = (uint8_t)((tag1 >> 5) & 7);
                scale = 1 << scale;
            }
            for (uint8_t c = 0; c < 8; c++)  // Update palettecurrent
            {
                uint8_t valueYRGB = (uint8_t)(palette >> (c << 2)) & 15;
                palettecurrent[c] = colors[pbpgpr | valueYRGB];
                //if (pbpgpr != 0) DebugLogFormat(_T("pbpgpr %02x\r\n"), pbpgpr | valueYRGB);
            }
        }

        uint16_t addressBits = g_pBoard->GetRAMWord(0, address);  // The word before the last word - is address of bits from all three memory planes
        address += 2;

        // Calculate size, type and address of the next tag
        uint16_t tagB = g_pBoard->GetRAMWord(0, address);  // Last word of the tag - is address and type of the next tag
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
            uint32_t* pBits = ((uint32_t*)pImageBits) + (288 - 1 - y) * 640;
            for (int pos = 0; ; pos++)
            {
                // Get bit from planes 0,1,2
                uint8_t src0 = g_pBoard->GetRAMByte(0, addressBits);
                uint8_t src1 = g_pBoard->GetRAMByte(1, addressBits);
                uint8_t src2 = g_pBoard->GetRAMByte(2, addressBits);
                // Loop through the bits of the byte
                int bit = 0;
                for (;;)
                {
                    uint32_t valueRGB;
                    if (cursorOn && (pos == cursorPos) && (!okCursorType || (okCursorType && bit == cursorAddress)))
                        valueRGB = colors[cursorYRGB];  // 4-bit to 32-bit color
                    else
                    {
                        // Make 3-bit value from the bits
                        uint8_t value012 = (src0 & 1) | ((src1 & 1) << 1) | ((src2 & 1) << 2);
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

bool Emulator_SaveScreenshot(LPCTSTR sFileName, const uint32_t * bits)
{
    ASSERT(sFileName != nullptr);
    ASSERT(bits != nullptr);

    // Create file
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_WRITE, FILE_SHARE_READ, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    BITMAPFILEHEADER hdr;
    ::ZeroMemory(&hdr, sizeof(hdr));
    hdr.bfType = 0x4d42;  // "BM"
    BITMAPINFOHEADER bih;
    ::ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof( BITMAPINFOHEADER );
    bih.biWidth = UKNC_SCREEN_WIDTH;
    bih.biHeight = UKNC_SCREEN_HEIGHT;
    bih.biSizeImage = bih.biWidth * bih.biHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 8;
    bih.biCompression = BI_RGB;
    bih.biXPelsPerMeter = bih.biXPelsPerMeter = 2000;
    hdr.bfSize = (DWORD) sizeof(BITMAPFILEHEADER) + bih.biSize + bih.biSizeImage;
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + bih.biSize + sizeof(RGBQUAD) * 256;

    uint8_t * pData = (uint8_t *) ::malloc(bih.biSizeImage);

    // Prepare the image data
    const uint32_t * psrc = bits;
    uint8_t * pdst = pData;
    const uint32_t * palette = ScreenView_StandardRGBColors;
    for (int i = 0; i < 640 * 288; i++)
    {
        uint32_t rgb = *psrc;
        psrc++;
        uint8_t color = 0;
        for (uint8_t c = 0; c < 128; c++)
        {
            if (palette[c] == rgb)
            {
                color = c;
                break;
            }
        }
        *pdst = color;
        pdst++;
    }

    DWORD dwBytesWritten = 0;

    WriteFile(hFile, &hdr, sizeof(BITMAPFILEHEADER), &dwBytesWritten, nullptr);
    if (dwBytesWritten != sizeof(BITMAPFILEHEADER))
    {
        ::free(pData);
        return false;
    }
    WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwBytesWritten, nullptr);
    if (dwBytesWritten != sizeof(BITMAPINFOHEADER))
    {
        ::free(pData);
        return false;
    }
    WriteFile(hFile, palette, sizeof(RGBQUAD) * 128, &dwBytesWritten, nullptr);
    if (dwBytesWritten != sizeof(RGBQUAD) * 128)
    {
        ::free(pData);
        return false;
    }
    //NOTE: Write the palette for the second time, to fill colors #128-255
    WriteFile(hFile, palette, sizeof(RGBQUAD) * 128, &dwBytesWritten, nullptr);
    if (dwBytesWritten != sizeof(RGBQUAD) * 128)
    {
        ::free(pData);
        return false;
    }

    WriteFile(hFile, pData, bih.biSizeImage, &dwBytesWritten, nullptr);
    ::free(pData);
    if (dwBytesWritten != bih.biSizeImage)
        return false;

    // Close file
    CloseHandle(hFile);

    return true;
}

bool Emulator_SaveScreenshot(LPCTSTR sFileName)
{
    uint32_t * bits = (uint32_t *) ::malloc(640 * 288 * 4);

    Emulator_PrepareScreenRGB32(bits, ScreenView_StandardRGBColors);

    bool result = Emulator_SaveScreenshot(sFileName, bits);

    ::free(bits);

    return result;
}


// Returns: amount of different pixels
int Emulator_CompareScreens(const uint32_t * scr1, const uint32_t * scr2)
{
    const uint32_t * p1 = scr1;
    const uint32_t * p2 = scr2;

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
int Emulator_CheckScreenshot(LPCTSTR sFileName, const uint32_t * bits, uint32_t * tempbits)
{
    ASSERT(sFileName != nullptr);
    ASSERT(bits != nullptr);

    // Open file for reading
    HANDLE hFile = ::CreateFile(sFileName,
            GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

    BITMAPFILEHEADER hdr;
    BITMAPINFOHEADER bih;
    DWORD dwBytesRead = 0;

    ReadFile(hFile, &hdr, sizeof(BITMAPFILEHEADER), &dwBytesRead, nullptr);
    if (dwBytesRead != sizeof(BITMAPFILEHEADER))
        return -1;
    //TODO: Check the header
    ReadFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwBytesRead, nullptr);
    if (dwBytesRead != sizeof(BITMAPINFOHEADER))
        return -1;
    //TODO: Check the header
    if (bih.biSizeImage != 640 * 288)
        return -1;
    // Skip the palette
    SetFilePointer(hFile, sizeof(RGBQUAD) * 256, 0, FILE_CURRENT);

    uint8_t * pData = (uint8_t *) ::malloc(bih.biSizeImage);

    ReadFile(hFile, pData, bih.biSizeImage, &dwBytesRead, nullptr);
    if (dwBytesRead != bih.biSizeImage)
    {
        ::free(pData);
        return -1;
    }

    // Decode the image data
    uint8_t * psrc = pData;
    uint32_t * pdst = tempbits;
    for (int i = 0; i < 640 * 288; i++)
    {
        uint8_t color = *psrc;
        psrc++;
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

void Emulator_PrepareDiffScreenshot(const uint32_t * scr1, const uint32_t * scr2, uint32_t * dest)
{
    const uint32_t * p1 = scr1;
    const uint32_t * p2 = scr2;
    uint32_t * pdest = dest;
    for (int i = 640 * 288; i > 0; i--)
    {
        if (*p1 == *p2)
            *pdest = *p1;
        else
            *pdest = 0x00003f | (*p1 & 0x00ff00) | (*p2 & 0xff0000);
        p1++;  p2++;  pdest++;
    }
}

int Emulator_CheckScreenshot(LPCTSTR sFileName)
{
    uint32_t * bits = (uint32_t *) ::malloc(640 * 288 * 4);
    uint32_t * tempbits = (uint32_t *) ::malloc(640 * 288 * 4);

    Emulator_PrepareScreenRGB32(bits, ScreenView_StandardRGBColors);

    int result = Emulator_CheckScreenshot(sFileName, bits, tempbits);
    if (result != 0)
    {
        uint32_t * diffbits = (uint32_t *) ::malloc(640 * 288 * 4);
        Emulator_PrepareDiffScreenshot(bits, tempbits, diffbits);

        // Make diff fike name like "diff" + file name without a path from sFileName
        TCHAR sDiffFileName[MAX_PATH];
        const TCHAR * sSubFileName = _tcsrchr(sFileName, _T('\\'));
        sSubFileName = (sSubFileName == NULL) ? sFileName : sSubFileName + 1;
        wsprintf(sDiffFileName, _T("diff_%s"), sSubFileName);

        Emulator_SaveScreenshot(sDiffFileName, diffbits);

        ::free(diffbits);
    }

    ::free(tempbits);
    ::free(bits);

    return result;
}

void Emulator_KeyboardPressRelease(uint8_t ukncscan, int timeout)
{
    g_pBoard->KeyboardEvent(ukncscan, true);
    Emulator_Run(timeout);
    g_pBoard->KeyboardEvent(ukncscan, false);
    Emulator_Run(3);
}

const uint8_t arrChar2UkncScan[256] =
{
    /*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0153, 0000, 0000, 0153, 0000, 0000,
    /*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*2*/    0113, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0117, 0175, 0146, 0173,
    /*3*/    0176, 0030, 0031, 0032, 0013, 0034, 0035, 0016, 0017, 0177, 0174, 0007, 0000, 0000, 0000, 0000,
    /*4*/    0077, 0072, 0076, 0050, 0057, 0033, 0047, 0055, 0156, 0073, 0027, 0052, 0056, 0112, 0054, 0075,
    /*5*/    0053, 0067, 0074, 0111, 0114, 0051, 0137, 0071, 0115, 0070, 0157, 0036, 0136, 0037, 0110, 0155,
    /*6*/    0126, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0155, 0000, 0000, 0000, 0000,
    /*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*c*/    0007, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
};

const uint8_t arrChar2UkncScanShift[256] =
{
    /*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*2*/    0000, 0030, 0031, 0032, 0013, 0034, 0035, 0016, 0017, 0177, 0174, 0007, 0000, 0000, 0000, 0000,
    /*3*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0117, 0175, 0135, 0173,
    /*4*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*5*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*6*/    0077, 0072, 0076, 0050, 0057, 0033, 0047, 0055, 0156, 0073, 0027, 0052, 0056, 0112, 0054, 0075,
    /*7*/    0053, 0067, 0074, 0111, 0114, 0051, 0137, 0071, 0115, 0070, 0157, 0036, 0136, 0037, 0110, 0000,
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
    uint8_t scan = arrChar2UkncScanShift[(uint8_t)ch];
    if (scan != 0)
    {
        Emulator_KeyboardPressReleaseShift(scan);
        return;
    }

    scan = arrChar2UkncScan[(uint8_t)ch];
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

void Emulator_KeyboardPressReleaseShift(uint8_t ukncscan)
{
    g_pBoard->KeyboardEvent(0105, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, false);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(0105, false);
    Emulator_Run(3);
}

void Emulator_KeyboardPressReleaseAlt(uint8_t ukncscan)
{
    g_pBoard->KeyboardEvent(0107, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, false);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(0107, false);
    Emulator_Run(3);
}

void Emulator_KeyboardPressReleaseCtrl(uint8_t ukncscan)
{
    g_pBoard->KeyboardEvent(046, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, true);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(ukncscan, false);
    Emulator_Run(3);
    g_pBoard->KeyboardEvent(046, false);
    Emulator_Run(3);
}


//////////////////////////////////////////////////////////////////////


bool Emulator_SaveImage(LPCTSTR sFilePath)
{
    // Create file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("w+b"), _SH_DENYWR);
    if (fpFile == nullptr)
        return false;

    // Allocate memory
    uint8_t* pImage = (uint8_t*) ::malloc(UKNCIMAGE_SIZE);
    memset(pImage, 0, UKNCIMAGE_SIZE);
    // Prepare header
    uint32_t* pHeader = (uint32_t*)pImage;
    *pHeader++ = UKNCIMAGE_HEADER1;
    *pHeader++ = UKNCIMAGE_HEADER2;
    *pHeader++ = UKNCIMAGE_VERSION;
    *pHeader++ = UKNCIMAGE_SIZE;
    // Store emulator state to the image
    g_pBoard->SaveToImage(pImage);
    *(uint32_t*)(pImage + 16) = m_dwEmulatorUptime;

    // Save image to the file
    size_t dwBytesWritten = ::fwrite(pImage, 1, UKNCIMAGE_SIZE, fpFile);
    ::free(pImage);
    ::fclose(fpFile);
    if (dwBytesWritten != UKNCIMAGE_SIZE)
        return false;

    return true;
}

bool Emulator_LoadImage(LPCTSTR sFilePath)
{
    // Open file
    FILE* fpFile = ::_tfsopen(sFilePath, _T("rb"), _SH_DENYWR);
    if (fpFile == nullptr)
        return false;

    Emulator_Stop();

    // Read header
    uint32_t bufHeader[UKNCIMAGE_HEADER_SIZE / sizeof(uint32_t)];
    size_t dwBytesRead = ::fread(bufHeader, 1, UKNCIMAGE_HEADER_SIZE, fpFile);
    //TODO: Check if dwBytesRead != UKNCIMAGE_HEADER_SIZE

    //TODO: Check version and size

    // Allocate memory
    uint8_t* pImage = (uint8_t*) ::malloc(UKNCIMAGE_SIZE);
    //TODO: Check for nullptr

    // Read image
    ::fseek(fpFile, 0, SEEK_SET);
    dwBytesRead = ::fread(pImage, 1, UKNCIMAGE_SIZE, fpFile);
    //TODO: Check if dwBytesRead != UKNCIMAGE_SIZE

    // Restore emulator state from the image
    g_pBoard->LoadFromImage(pImage);

    m_dwEmulatorUptime = *(uint32_t*)(pImage + 16);

    // Free memory, close file
    ::free(pImage);
    ::fclose(fpFile);

    return true;
}


//////////////////////////////////////////////////////////////////////
