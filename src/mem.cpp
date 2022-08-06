#include "mem.h"

Mem::Mem(PC *_pc)
{
    for (int i = 0; i < 4; i++) {
        memReadMap[i] = new int[65536]{255};
    }
    for (int i = 0; i < 32; i++) {
        rom[i] = new int[8192]{0};
    }
}
Mem::~Mem()
{
    for (int i = 0; i < 4; i++) {
        delete[] memReadMap[i];
    }

    for (int i = 0; i < 32; i++) {
        delete[] rom[i];
    }
}
void Mem::reset()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 65536; j++) {
            memReadMap[i][j] = 255;
        }
    }
}
void Mem::load(std::vector<int> &bin, int idx, int offset, int size)
{
    for (int i = offset; i < size; i++) {
        memReadMap[idx][i] = bin[i - offset];
    }
}
int Mem::readMem(int address)
{
    if (!megarom) {
        return memReadMap[0x3 & (PPIPortA >> ((address & 0xc000) >> 13))][address];
    } else {
        if (((address & 0xc000) >> 14) == cartSlot && address <= 49151 && address >= 16384)
            return rom[pagMegaRom[(address >> 13) - 2]][address % 8192];
        else
            return memReadMap[0x3 & (PPIPortA >> ((address & 0xc000) >> 13))][address];
    }
    return 0;
}
int Mem::readMemWord(int address)
{
    if (!megarom) {
        return memReadMap[0x3 & (PPIPortA >> (((address + 1) & 0xc000) >> 13))][address + 1] << 8 |
               memReadMap[0x3 & (PPIPortA >> ((address & 0xc000) >> 13))][address];
    } else {
        if (((address & 0xc000) >> 14) == cartSlot && address <= 49151 && address >= 16384)
            return rom[pagMegaRom[((address + 1) >> 13) - 2]][(address + 1) % 8192] << 8 |
                   rom[pagMegaRom[(address >> 13) - 2]][address % 8192];
        else
            return memReadMap[0x3 & (PPIPortA >> (((address + 1) & 0xc000) >> 13))][address + 1] << 8 |
                   memReadMap[0x3 & (PPIPortA >> ((address & 0xc000) >> 13))][address];
    }
}
void Mem::writeMem(int address, int value)
{
    int i = 0x3 & (PPIPortA >> ((address & 0xc000) >> 13));

    if (podeEscrever[i])
        memReadMap[i][address] = value & 0xff;
    if (address == 65535)
        memReadMap[i][65535] = 255;
    if (!megarom)
        return;

    if (i == cartSlot) {
        switch (tipoMegarom) {
            case 0:
                if (address == 16384 || address == 20480)
                    pagMegaRom[0] = value & 0xff;
                else if (address == 24576 || address == 28672)
                    pagMegaRom[1] = value & 0xff;
                else if (address == 32768 || address == 36864)
                    pagMegaRom[2] = value & 0xff;
                else if (address == 40960 || address == 45056)
                    pagMegaRom[3] = value & 0xff;
                break;
            case 1:
                if (address == 16384 || address == 20480) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                } else if (address == 32768 || address == 36864) {
                    pagMegaRom[2] = value & 0xff;
                    pagMegaRom[3] = pagMegaRom[2] + 1;
                }
                break;
            case 2:
                if (address >= 24576 && address <= 26623)
                    pagMegaRom[0] = value & 0xff;
                else if (address >= 26624 && address <= 28671)
                    pagMegaRom[1] = value & 0xff;
                else if (address >= 28672 && address <= 30719)
                    pagMegaRom[2] = value & 0xff;
                else if (address >= 30720 && address <= 32767)
                    pagMegaRom[3] = value & 0xff;
                break;
            case 3:
                if (address >= 24576 && address <= 26623) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                } else if (address >= 28672 && address <= 30719) {
                    pagMegaRom[2] = value & 0xff;
                    pagMegaRom[3] = pagMegaRom[2] + 1;
                }
                break;
        }
    }
}
void Mem::writeMemWord(int address, int value)
{
    int i = 0x3 & (PPIPortA >> ((address & 0xc000) >> 13));
    if (podeEscrever[i]) {
        memReadMap[i][address] = value & 0xff;
        if (++address < 65535)
            memReadMap[i][address] = value >> 8;
        if (address == 65535 || address == 65536)
            memReadMap[i][65535] = 255;
    }
    if (!megarom)
        return;

    if (i == cartSlot) {
        switch (tipoMegarom) {
            case 0:
                if (address == 16384 || address == 20480)
                    pagMegaRom[0] = value & 0xff;
                else if (address == 24576 || address == 28672)
                    pagMegaRom[1] = value & 0xff;
                else if (address == 32768 || address == 36864)
                    pagMegaRom[2] = value & 0xff;
                else if (address == 40960 || address == 45056)
                    pagMegaRom[3] = value & 0xff;
                else if (address == 24575 || address == 28671)
                    pagMegaRom[1] = value & 0xff;
                else if (address == 32767 || address == 36863)
                    pagMegaRom[2] = value & 0xff;
                else if (address == 40959 || address == 45055)
                    pagMegaRom[3] = value & 0xff;
                break;
            case 1:
                if (address == 16384 || address == 20480) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                } else if (address == 32768 || address == 36864) {
                    pagMegaRom[2] = value & 0xff;
                    pagMegaRom[3] = pagMegaRom[2] + 1;
                } else if (address == 16383 || address == 20479) {
                    pagMegaRom[0] = value >> 8 & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                } else if (address == 24575 || address == 28671) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                    pagMegaRom[2] = value >> 8 & 0xff;
                    pagMegaRom[3] = pagMegaRom[2] + 1;
                }
                break;
            case 2:
                if (address >= 24576 && address < 26623)
                    pagMegaRom[0] = value & 0xff;
                else if (address >= 26624 && address < 28671)
                    pagMegaRom[1] = value & 0xff;
                else if (address >= 28672 && address < 30719)
                    pagMegaRom[2] = value & 0xff;
                else if (address >= 30720 && address < 32767)
                    pagMegaRom[3] = value & 0xff;
                else if (address == 24575)
                    pagMegaRom[0] = value >> 8 & 0xff;
                else if (address == 26623) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = value >> 8 & 0xff;
                } else if (address == 28671) {
                    pagMegaRom[1] = value & 0xff;
                    pagMegaRom[2] = value >> 8 & 0xff;
                } else if (address == 30719) {
                    pagMegaRom[2] = value & 0xff;
                    pagMegaRom[3] = value >> 8 & 0xff;
                } else if (address == 32767)
                    pagMegaRom[3] = value & 0xff;
                break;
            case 3:
                if (address >= 24576 && address <= 26623) {
                    pagMegaRom[0] = value & 0xff;
                    pagMegaRom[1] = pagMegaRom[0] + 1;
                } else if (address >= 28672 && address <= 30719) {
                    pagMegaRom[2] = value & 0xff;
                    pagMegaRom[3] = pagMegaRom[2] + 1;
                }
                break;
        }
    }
}
void Mem::preparaMemoriaMegarom(std::string str)
{
    if (0 < str.length()) {
        if (str == "0")
            tipoMegarom = 0;
        else if (str == "1")
            tipoMegarom = 1;
        else if (str == "2")
            tipoMegarom = 2;
        else if (str == "3")
            tipoMegarom = 3;
    }
}
