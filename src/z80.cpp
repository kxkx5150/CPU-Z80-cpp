#include "z80.h"
#include "PC.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

// #include "tms9918.h"

z80::z80(PC *_pc)
{
    msx = _pc;

    for (int i = 0; i < 4; i++) {
        memReadMap[i] = new int[65536]{255};
    }
    for (int i = 0; i < 32; i++) {
        rom[i] = new int[8192]{0};
    }
    reset();
    file_read();
}
z80::~z80()
{
    for (int i = 0; i < 4; i++) {
        delete[] memReadMap[i];
    }

    for (int i = 0; i < 32; i++) {
        delete[] rom[i];
    }
}
void z80::reset()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 65536; j++) {
            memReadMap[i][j] = 255;
        }
    }

    for (int i = 0; i < 256; i++) {
        int flg = 1;
        for (int i_0_ = 0; i_0_ < 8; i_0_++) {
            if ((i & 1 << i_0_) != 0)
                flg ^= 1;
        }
        parity[i] = flg;
    }

    _PC = 0;
    _SP = 65520;
    _A  = 0;
    setF(0);
    setBC(0);
    setDE(0);
    setHL(0);

    int i = HL();
    setHL(_HL_);
    _HL_ = i;
    i    = DE();
    setDE(_DE_);
    _DE_ = i;
    i    = BC();
    setBC(_BC_);
    _BC_ = i;

    int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
             (fN ? 2 : 0) | (fC ? 1 : 0));
    i     = _A << 8 | f;
    _A    = (_AF_ >> 8);
    setF(_AF_ & 0xff);
    _AF_ = i;
    _A   = 0;

    setF(0);
    setBC(0);
    setDE(0);
    setHL(0);

    _IX = (0);
    _IY = (0);
    _R  = i;
    _R7 = i & 0x80;

    _I = (0);
    setIFF1(false);
    setIFF2(false);
    _IM = (0);

    for (i = 0; i < 256; i++)
        portos[i] = -1;
}
void z80::load(std::vector<int> &bin, int idx, int offset, int size)
{
    for (int i = offset; i < size; i++) {
        memReadMap[idx][i] = bin[i - offset];
    }
}
int z80::file_read()
{
    logcheck      = false;
    stepinfo      = false;
    filename      = "logs/log0.txt";
    filecheck_end = 10000;

    if (logcheck) {
        string   line;
        ifstream input_file(filename);
        if (!input_file.is_open()) {
            logcheck = false;
            stepinfo = false;
        }

        while (getline(input_file, line)) {
            lines.push_back(line);
        }

        input_file.close();

        // if (filename == "logs/log0.txt") {
        //     filecheck_start = 1;
        //     filecheck_end   = 1000000;
        // } else if (filename == "logs/log1.txt") {
        //     filecheck_start = 1000001;
        //     filecheck_end   = 2000000;
        //     fileoffset      = 1000000;
        // } else if (filename == "logs/log2.txt") {
        //     filecheck_start = 2000001;
        //     filecheck_end   = 3000000;
        //     fileoffset      = 2000000;
        // }
    }
    return EXIT_SUCCESS;
}
void z80::dump(int opcode, int i)
{
    count++;

    if (logcheck && filecheck_start <= count && count <= filecheck_end) {

        char buf1[1000];
        sprintf(buf1, "i=%d STEPS=%lu OPCODE=%d", i, count, opcode);
        // printf("%s\n", buf1);

        char buf2[1000];
        sprintf(buf2, "A=%02X B=%02X C=%02X D=%02X", _A, _B, _C, _D);
        // printf("%s\n", buf2);

        char buf3[1000];
        sprintf(buf3, "E=%02X H=%02X L=%02X", _E, _H, _L);
        // printf("%s\n", buf3);

        char buf4[1000];
        sprintf(buf4, "fS=%d fZ=%d f5=%d fH=%d f3=%d fPV=%d fN=%d fC=%d", fS, fZ, f5, fH, f3, fPV, fN, fC);
        // printf("%s\n", buf4);

        char buf5[1000];
        sprintf(buf5, "AF=%04X HL=%04X BC=%04X DE=%04X", _AF_, _HL_, _BC_, _DE_);
        // printf("%s\n", buf5);

        char buf6[1000];
        sprintf(buf6, "IX=%04X IY=%04X ID=%04X", _IX, _IY, _ID);
        // printf("%s\n", buf6);

        char buf7[1000];
        sprintf(buf7, "SP=%04X PC=%04X", _SP, _PC);
        // printf("%s\n", buf7);

        char buf8[1000];
        sprintf(buf8, "I=%04X R=%08X R7=%04X IM=%04X", _I, _R, _R7, _IM);
        // printf("%s\n", buf8);

        char buf9[1000];
        sprintf(buf9, "IFF1=%d IFF2=%d", _IFF1, _IFF2);
        // printf("%s\n", buf9);

        char buf10[1000];
        sprintf(buf10, "PPA=%04X PPC=%04X PPD=%04X", PPIPortA, PPIPortC, PPIPortD);
        // printf("%s\n", buf10);

        if (stepinfo) {
            printf("count : %lu\n", count);
            printf("%s\n", lines[count - 1 - fileoffset].c_str());
            printf("%s %s %s %s %s %s %s %s %s %s\n", buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9, buf10);
        }

        if (count < filecheck_end) {
            int len = lines[0].size() + 10;

            char *sbf = new char[len];
            sprintf(sbf, "%s", lines[count - 1 - fileoffset].c_str());
            std::string s = sbf;

            char *tbf = new char[len];
            sprintf(tbf, "%s %s %s %s %s %s %s %s %s %s", buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9, buf10);
            std::string t = tbf;

            if (std::equal(t.begin(), t.end(), s.begin())) {
                // printf("ok !\n");
            } else {
                printf("\n\n\n***************\n");
                printf("*** Error ! ***\n");
                printf("***************\n\n\n");

                printf("count : %lu\n", count);
                printf("OK : %s\n", lines[count - 1 - fileoffset].c_str());
                printf("NG : %s %s %s %s %s %s %s %s %s %s\n\n", buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9,
                       buf10);
                exit(1);
            }

            delete[] sbf;
            delete[] tbf;
        } else {
            printf("\n\n\n\n\n-- Compare OK ! ---\n");
            printf("count : %lu\n\n", count);
            printf("\n\n\n\n\n\n");
            exit(1);
        }
    }
}
void z80::run()
{
    int i      = -(T_STATES_PER_INTERRUPT - z80_interrupt());
    int i_2_   = 0;
    int i_3_   = 0;
    int opcode = 0;

    while (i < 0) {
        _R += 1;
        opcode = readMem(_PC++);

        if (count == 263551) {
            printf(" ");
        }
        dump(opcode, i);

        switch (opcode) {
            case 0:
                i += 4;
                break;
            case 8: {
                int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
                         (fN ? 2 : 0) | (fC ? 1 : 0));
                int ii = _A << 8 | f;
                _A     = (_AF_ >> 8);
                setF(_AF_ & 0xff);
                _AF_ = ii;
                i += 4;
                break;
            }
            case 16: {
                _B = (i_3_ = _B - 1 & 0xff);

                if (i_3_ != 0) {
                    i_2_ = bytef(readMem(_PC++));
                    _PC  = (_PC + i_2_ & 0xffff);
                    i += 13;
                } else {
                    _PC = _PC + 1 & 0xffff;
                    i += 8;
                }
                break;
            }
            case 24: {
                i_2_ = bytef(readMem(_PC++));
                _PC  = (_PC + i_2_ & 0xffff);
                i += 12;
                break;
            }
            case 32:
                if (!fZ) {
                    i_2_ = bytef(readMem(_PC++));
                    _PC  = (_PC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    _PC = _PC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 40:
                if (fZ) {
                    i_2_ = bytef(readMem(_PC++));
                    _PC  = (_PC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    _PC = _PC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 48:
                if (!fC) {
                    i_2_ = bytef(readMem(_PC++));
                    _PC  = (_PC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    _PC = _PC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 56:
                if (fC) {
                    i_2_ = bytef(readMem(_PC++));
                    _PC  = (_PC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    _PC = _PC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 1:
                setBC(readMemWord((_PC = _PC + 2) - 2));
                i += 10;
                break;
            case 9:
                setHL(add16(HL(), BC()));
                i += 11;
                break;
            case 17:
                setDE(readMemWord((_PC = _PC + 2) - 2));
                i += 10;
                break;
            case 25:
                setHL(add16(HL(), DE()));
                i += 11;
                break;
            case 33:
                setHL(readMemWord((_PC = _PC + 2) - 2));
                i += 10;
                break;
            case 41: {
                i_2_ = HL();
                setHL(add16(i_2_, i_2_));
                i += 11;
                break;
            }
            case 49:
                _SP = (readMemWord((_PC = _PC + 2) - 2));
                i += 10;
                break;
            case 57:
                setHL(add16(HL(), _SP));
                i += 11;
                break;
            case 2:
                writeMem(BC(), _A);
                i += 7;
                break;
            case 10:
                _A = (readMem(BC()));
                i += 7;
                break;
            case 18:
                writeMem(DE(), _A);
                i += 7;
                break;
            case 26:
                _A = (readMem(DE()));
                i += 7;
                break;
            case 34:
                writeMemWord(readMemWord((_PC = _PC + 2) - 2), HL());
                i += 16;
                break;
            case 42:
                setHL(readMemWord(readMemWord((_PC = _PC + 2) - 2)));
                i += 16;
                break;
            case 50:
                writeMem(readMemWord((_PC = _PC + 2) - 2), _A);
                i += 13;
                break;
            case 58:
                _A = (readMem(readMemWord((_PC = _PC + 2) - 2)));
                i += 13;
                break;
            case 3:
                setBC(BC() + 1 & 0xffff);
                i += 6;
                break;
            case 11:
                setBC(BC() - 1 & 0xffff);
                i += 6;
                break;
            case 19:
                setDE(DE() + 1 & 0xffff);
                i += 6;
                break;
            case 27:
                setDE(DE() - 1 & 0xffff);
                i += 6;
                break;
            case 35:
                setHL(HL() + 1 & 0xffff);
                i += 6;
                break;
            case 43:
                setHL(HL() - 1 & 0xffff);
                i += 6;
                break;
            case 51:
                _SP = _SP + 1 & 0xffff;
                i += 6;
                break;
            case 59:
                _SP = _SP - 1 & 0xffff;
                i += 6;
                break;
            case 4:
                _B = (inc8(_B));
                i += 4;
                break;
            case 12:
                _C = (inc8(_C));
                i += 4;
                break;
            case 20:
                _D = (inc8(_D));
                i += 4;
                break;
            case 28:
                _E = (inc8(_E));
                i += 4;
                break;
            case 36:
                _H = (inc8(_H));
                i += 4;
                break;
            case 44:
                _L = (inc8(_L));
                i += 4;
                break;
            case 52: {
                i_2_ = HL();
                writeMem(i_2_, inc8(readMem(i_2_)));
                i += 11;
                break;
            }
            case 60:
                _A = (inc8(_A));
                i += 4;
                break;
            case 5:
                _B = (dec8(_B));
                i += 4;
                break;
            case 13:
                _C = (dec8(_C));
                i += 4;
                break;
            case 21:
                _D = (dec8(_D));
                i += 4;
                break;
            case 29:
                _E = (dec8(_E));
                i += 4;
                break;
            case 37:
                _H = (dec8(_H));
                i += 4;
                break;
            case 45:
                _L = (dec8(_L));
                i += 4;
                break;
            case 53: {
                i_2_ = HL();
                writeMem(i_2_, dec8(readMem(i_2_)));
                i += 11;
                break;
            }
            case 61:
                _A = (dec8(_A));
                i += 4;
                break;
            case 6:
                _B = (readMem(_PC++));
                i += 7;
                break;
            case 14:
                _C = (readMem(_PC++));
                i += 7;
                break;
            case 22:
                _D = (readMem(_PC++));
                i += 7;
                break;
            case 30:
                _E = (readMem(_PC++));
                i += 7;
                break;
            case 38:
                _H = (readMem(_PC++));
                i += 7;
                break;
            case 46:
                _L = (readMem(_PC++));
                i += 7;
                break;
            case 54:
                writeMem(HL(), readMem(_PC++));
                i += 10;
                break;
            case 62:
                _A = (readMem(_PC++));
                i += 7;
                break;
            case 7: {
                int  ii  = _A;
                bool flg = (ii & 0x80) != 0;
                if (flg)
                    ii = ii << 1 | 0x1;
                else
                    ii <<= 1;
                ii &= 0xff;
                f3 = ((ii & 0x8) != 0);
                f5 = ((ii & 0x20) != 0);
                fN = (false);
                fH = (false);
                fC = (flg);
                _A = (ii);
                i += 4;
            } break;
            case 15: {
                int  ii  = _A;
                bool flg = (ii & 0x1) != 0;
                if (flg)
                    ii = ii >> 1 | 0x80;
                else
                    ii >>= 1;
                f3 = ((ii & 0x8) != 0);
                f5 = ((ii & 0x20) != 0);
                fN = (false);
                fH = (false);
                fC = (flg);
                _A = (ii);
                i += 4;
            } break;
            case 23: {
                int  ii  = _A;
                bool flg = (ii & 0x80) != 0;
                if (fC)
                    ii = ii << 1 | 0x1;
                else
                    ii <<= 1;
                ii &= 0xff;
                f3 = ((ii & 0x8) != 0);
                f5 = ((ii & 0x20) != 0);
                fN = (false);
                fH = (false);
                fC = (flg);
                _A = (ii);
                i += 4;
            } break;
            case 31: {
                int  ii  = _A;
                bool flg = (ii & 0x1) != 0;
                if (fC)
                    ii = ii >> 1 | 0x80;
                else
                    ii >>= 1;
                f3 = ((ii & 0x8) != 0);
                f5 = ((ii & 0x20) != 0);
                fN = (false);
                fH = (false);
                fC = (flg);
                _A = (ii);
                i += 4;
            } break;
            case 39: {
                int  ii    = _A;
                int  i_20_ = 0;
                bool flg   = fC;
                if (fH || (ii & 0xf) > 9)
                    i_20_ |= 0x6;
                if (flg || ii > 159 || ii > 143 && (ii & 0xf) > 9)
                    i_20_ |= 0x60;
                if (ii > 153)
                    flg = true;
                if (fN)
                    sub_a(i_20_);
                else
                    add_a(i_20_);
                ii  = _A;
                fC  = (flg);
                fPV = (parity[ii]);
                i += 4;
            } break;
            case 47: {
                int ii = _A ^ 0xff;
                f3     = ((_A & 0x8) != 0);
                f5     = ((_A & 0x20) != 0);
                fH     = (true);
                fN     = (true);
                _A     = (ii);
                i += 4;
            } break;
            case 55: {
                int ii = _A;
                f3     = ((ii & 0x8) != 0);
                f5     = ((ii & 0x20) != 0);
                fN     = (false);
                fH     = (false);
                fC     = (true);
                i += 4;
            } break;
            case 63: {
                int ii = _A;
                f3     = ((ii & 0x8) != 0);
                f5     = ((ii & 0x20) != 0);
                fN     = (false);
                fC     = (!fC);
                i += 4;
            } break;
            case 64:
                i += 4;
                break;
            case 65:
                _B = (_C);
                i += 4;
                break;
            case 66:
                _B = (_D);
                i += 4;
                break;
            case 67:
                _B = (_E);
                i += 4;
                break;
            case 68:
                _B = (_H);
                i += 4;
                break;
            case 69:
                _B = (_L);
                i += 4;
                break;
            case 70:
                _B = (readMem(HL()));
                i += 7;
                break;
            case 71:
                _B = (_A);
                i += 4;
                break;
            case 72:
                _C = (_B);
                i += 4;
                break;
            case 73:
                i += 4;
                break;
            case 74:
                _C = (_D);
                i += 4;
                break;
            case 75:
                _C = (_E);
                i += 4;
                break;
            case 76:
                _C = (_H);
                i += 4;
                break;
            case 77:
                _C = (_L);
                i += 4;
                break;
            case 78:
                _C = (readMem(HL()));
                i += 7;
                break;
            case 79:
                _C = (_A);
                i += 4;
                break;
            case 80:
                _D = (_B);
                i += 4;
                break;
            case 81:
                _D = (_C);
                i += 4;
                break;
            case 82:
                i += 4;
                break;
            case 83:
                _D = (_E);
                i += 4;
                break;
            case 84:
                _D = (_H);
                i += 4;
                break;
            case 85:
                _D = (_L);
                i += 4;
                break;
            case 86:
                _D = (readMem(HL()));
                i += 7;
                break;
            case 87:
                _D = (_A);
                i += 4;
                break;
            case 88:
                _E = (_B);
                i += 4;
                break;
            case 89:
                _E = (_C);
                i += 4;
                break;
            case 90:
                _E = (_D);
                i += 4;
                break;
            case 91:
                i += 4;
                break;
            case 92:
                _E = (_H);
                i += 4;
                break;
            case 93:
                _E = (_L);
                i += 4;
                break;
            case 94:
                _E = (readMem(HL()));
                i += 7;
                break;
            case 95:
                _E = (_A);
                i += 4;
                break;
            case 96:
                _H = (_B);
                i += 4;
                break;
            case 97:
                _H = (_C);
                i += 4;
                break;
            case 98:
                _H = (_D);
                i += 4;
                break;
            case 99:
                _H = (_E);
                i += 4;
                break;
            case 100:
                i += 4;
                break;
            case 101:
                _H = (_L);
                i += 4;
                break;
            case 102:
                _H = (readMem(HL()));
                i += 7;
                break;
            case 103:
                _H = (_A);
                i += 4;
                break;
            case 104:
                _L = (_B);
                i += 4;
                break;
            case 105:
                _L = (_C);
                i += 4;
                break;
            case 106:
                _L = (_D);
                i += 4;
                break;
            case 107:
                _L = (_E);
                i += 4;
                break;
            case 108:
                _L = (_H);
                i += 4;
                break;
            case 109:
                i += 4;
                break;
            case 110:
                _L = (readMem(HL()));
                i += 7;
                break;
            case 111:
                _L = (_A);
                i += 4;
                break;
            case 112:
                writeMem(HL(), _B);
                i += 7;
                break;
            case 113:
                writeMem(HL(), _C);
                i += 7;
                break;
            case 114:
                writeMem(HL(), _D);
                i += 7;
                break;
            case 115:
                writeMem(HL(), _E);
                i += 7;
                break;
            case 116:
                writeMem(HL(), _H);
                i += 7;
                break;
            case 117:
                writeMem(HL(), _L);
                i += 7;
                break;
            case 118: {
                i_2_ = (-i - 1) / 4 + 1;
                i += i_2_ * 4;
                _R += (i_2_ - 1);
                _R &= 0xffff;
                break;
            }
            case 119:
                writeMem(HL(), _A);
                i += 7;
                break;
            case 120:
                _A = (_B);
                i += 4;
                break;
            case 121:
                _A = (_C);
                i += 4;
                break;
            case 122:
                _A = (_D);
                i += 4;
                break;
            case 123:
                _A = (_E);
                i += 4;
                break;
            case 124:
                _A = (_H);
                i += 4;
                break;
            case 125:
                _A = (_L);
                i += 4;
                break;
            case 126:
                _A = (readMem(HL()));
                i += 7;
                break;
            case 127:
                i += 4;
                break;
            case 128:
                add_a(_B);
                i += 4;
                break;
            case 129:
                add_a(_C);
                i += 4;
                break;
            case 130:
                add_a(_D);
                i += 4;
                break;
            case 131:
                add_a(_E);
                i += 4;
                break;
            case 132:
                add_a(_H);
                i += 4;
                break;
            case 133:
                add_a(_L);
                i += 4;
                break;
            case 134:
                add_a(readMem(HL()));
                i += 7;
                break;
            case 135:
                add_a(_A);
                i += 4;
                break;
            case 136:
                adc_a(_B);
                i += 4;
                break;
            case 137:
                adc_a(_C);
                i += 4;
                break;
            case 138:
                adc_a(_D);
                i += 4;
                break;
            case 139:
                adc_a(_E);
                i += 4;
                break;
            case 140:
                adc_a(_H);
                i += 4;
                break;
            case 141:
                adc_a(_L);
                i += 4;
                break;
            case 142:
                adc_a(readMem(HL()));
                i += 7;
                break;
            case 143:
                adc_a(_A);
                i += 4;
                break;
            case 144:
                sub_a(_B);
                i += 4;
                break;
            case 145:
                sub_a(_C);
                i += 4;
                break;
            case 146:
                sub_a(_D);
                i += 4;
                break;
            case 147:
                sub_a(_E);
                i += 4;
                break;
            case 148:
                sub_a(_H);
                i += 4;
                break;
            case 149:
                sub_a(_L);
                i += 4;
                break;
            case 150:
                sub_a(readMem(HL()));
                i += 7;
                break;
            case 151:
                sub_a(_A);
                i += 4;
                break;
            case 152:
                sbc_a(_B);
                i += 4;
                break;
            case 153:
                sbc_a(_C);
                i += 4;
                break;
            case 154:
                sbc_a(_D);
                i += 4;
                break;
            case 155:
                sbc_a(_E);
                i += 4;
                break;
            case 156:
                sbc_a(_H);
                i += 4;
                break;
            case 157:
                sbc_a(_L);
                i += 4;
                break;
            case 158:
                sbc_a(readMem(HL()));
                i += 7;
                break;
            case 159:
                sbc_a(_A);
                i += 4;
                break;
            case 160:
                and_a(_B);
                i += 4;
                break;
            case 161:
                and_a(_C);
                i += 4;
                break;
            case 162:
                and_a(_D);
                i += 4;
                break;
            case 163:
                and_a(_E);
                i += 4;
                break;
            case 164:
                and_a(_H);
                i += 4;
                break;
            case 165:
                and_a(_L);
                i += 4;
                break;
            case 166:
                and_a(readMem(HL()));
                i += 7;
                break;
            case 167:
                and_a(_A);
                i += 4;
                break;
            case 168:
                xor_a(_B);
                i += 4;
                break;
            case 169:
                xor_a(_C);
                i += 4;
                break;
            case 170:
                xor_a(_D);
                i += 4;
                break;
            case 171:
                xor_a(_E);
                i += 4;
                break;
            case 172:
                xor_a(_H);
                i += 4;
                break;
            case 173:
                xor_a(_L);
                i += 4;
                break;
            case 174:
                xor_a(readMem(HL()));
                i += 7;
                break;
            case 175:
                xor_a(_A);
                i += 4;
                break;
            case 176:
                or_a(_B);
                i += 4;
                break;
            case 177:
                or_a(_C);
                i += 4;
                break;
            case 178:
                or_a(_D);
                i += 4;
                break;
            case 179:
                or_a(_E);
                i += 4;
                break;
            case 180:
                or_a(_H);
                i += 4;
                break;
            case 181:
                or_a(_L);
                i += 4;
                break;
            case 182:
                or_a(readMem(HL()));
                i += 7;
                break;
            case 183:
                or_a(_A);
                i += 4;
                break;
            case 184:
                cp_a(_B);
                i += 4;
                break;
            case 185:
                cp_a(_C);
                i += 4;
                break;
            case 186:
                cp_a(_D);
                i += 4;
                break;
            case 187:
                cp_a(_E);
                i += 4;
                break;
            case 188:
                cp_a(_H);
                i += 4;
                break;
            case 189:
                cp_a(_L);
                i += 4;
                break;
            case 190:
                cp_a(readMem(HL()));
                i += 7;
                break;
            case 191:
                cp_a(_A);
                i += 4;
                break;
            case 192:
                if (!fZ) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 200:
                if (fZ) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 208:
                if (!fC) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 216:
                if (fC) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 224:
                if (!fPV) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 232:
                if (fPV) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 240:
                if (!fS) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 248:
                if (fS) {
                    poppc();
                    i += 11;
                } else
                    i += 5;
                break;
            case 193:
                setBC(popw());
                i += 10;
                break;
            case 201:
                poppc();
                i += 10;
                break;
            case 209:
                setDE(popw());
                i += 10;
                break;
            case 217: {
                int ii = HL();
                setHL(_HL_);
                _HL_ = ii;
                ii   = DE();
                setDE(_DE_);
                _DE_ = ii;
                ii   = BC();
                setBC(_BC_);
                _BC_ = ii;
                i += 4;
            } break;
            case 225:
                setHL(popw());
                i += 10;
                break;
            case 233:
                _PC = (HL());
                i += 4;
                break;
            case 241: {
                int ii = popw();
                _A     = (ii >> 8);
                setF(ii & 0xff);
                i += 10;
            } break;
            case 249:
                _SP = (HL());
                i += 6;
                break;
            case 194:
                if (!fZ)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 202:
                if (fZ)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 210:
                if (!fC)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 218:
                if (fC)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 226:
                if (!fPV)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 234:
                if (fPV)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 242:
                if (!fS)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 250:
                if (fS)
                    _PC = (readMemWord((_PC = _PC + 2) - 2));
                else
                    _PC = (_PC + 2 & 0xffff);
                i += 10;
                break;
            case 195:
                _PC = (readMemWord(_PC));
                i += 10;
                break;
            case 203: {
                int ii = HL();
                _R += (1);
                switch (readMem(_PC++)) {
                    case 0:
                        _B = (rlc(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 1:
                        _C = (rlc(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 2:
                        _D = (rlc(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 3:
                        _E = (rlc(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 4:
                        _H = (rlc(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 5:
                        _L = (rlc(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 6: {
                        // ii = HL();
                        writeMem(ii, rlc(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 7:
                        _A = (rlc(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 8:
                        _B = (rrc(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 9:
                        _C = (rrc(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 10:
                        _D = (rrc(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 11:
                        _E = (rrc(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 12:
                        _H = (rrc(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 13:
                        _L = (rrc(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 14: {
                        // ii = HL();
                        writeMem(ii, rrc(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 15:
                        _A = (rrc(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 16:
                        _B = (rl(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 17:
                        _C = (rl(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 18:
                        _D = (rl(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 19:
                        _E = (rl(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 20:
                        _H = (rl(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 21:
                        _L = (rl(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 22: {
                        // ii = HL();
                        writeMem(ii, rl(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 23:
                        _A = (rl(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 24:
                        _B = (rr(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 25:
                        _C = (rr(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 26:
                        _D = (rr(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 27:
                        _E = (rr(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 28:
                        _H = (rr(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 29:
                        _L = (rr(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 30: {
                        // ii = HL();
                        writeMem(ii, rr(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 31:
                        _A = (rr(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 32:
                        _B = (sla(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 33:
                        _C = (sla(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 34:
                        _D = (sla(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 35:
                        _E = (sla(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 36:
                        _H = (sla(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 37:
                        _L = (sla(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 38: {
                        // ii = HL();
                        writeMem(ii, sla(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 39:
                        _A = (sla(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 40:
                        _B = (sra(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 41:
                        _C = (sra(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 42:
                        _D = (sra(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 43:
                        _E = (sra(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 44:
                        _H = (sra(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 45:
                        _L = (sra(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 46: {
                        // ii = HL();
                        writeMem(ii, sra(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 47:
                        _A = (sra(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 48:
                        _B = (sls(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 49:
                        _C = (sls(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 50:
                        _D = (sls(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 51:
                        _E = (sls(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 52:
                        _H = (sls(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 53:
                        _L = (sls(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 54: {
                        // ii = HL();
                        writeMem(ii, sls(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 55:
                        _A = (sls(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 56:
                        _B = (srl(_B));
                        i += 8;
                        goto EXEC_CB;
                    case 57:
                        _C = (srl(_C));
                        i += 8;
                        goto EXEC_CB;
                    case 58:
                        _D = (srl(_D));
                        i += 8;
                        goto EXEC_CB;
                    case 59:
                        _E = (srl(_E));
                        i += 8;
                        goto EXEC_CB;
                    case 60:
                        _H = (srl(_H));
                        i += 8;
                        goto EXEC_CB;
                    case 61:
                        _L = (srl(_L));
                        i += 8;
                        goto EXEC_CB;
                    case 62: {
                        // ii = HL();
                        writeMem(ii, srl(readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 63:
                        _A = (srl(_A));
                        i += 8;
                        goto EXEC_CB;
                    case 64:
                        bit(1, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 65:
                        bit(1, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 66:
                        bit(1, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 67:
                        bit(1, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 68:
                        bit(1, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 69:
                        bit(1, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 70:
                        bit(1, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 71:
                        bit(1, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 72:
                        bit(2, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 73:
                        bit(2, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 74:
                        bit(2, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 75:
                        bit(2, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 76:
                        bit(2, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 77:
                        bit(2, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 78:
                        bit(2, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 79:
                        bit(2, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 80:
                        bit(4, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 81:
                        bit(4, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 82:
                        bit(4, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 83:
                        bit(4, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 84:
                        bit(4, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 85:
                        bit(4, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 86:
                        bit(4, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 87:
                        bit(4, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 88:
                        bit(8, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 89:
                        bit(8, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 90:
                        bit(8, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 91:
                        bit(8, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 92:
                        bit(8, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 93:
                        bit(8, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 94:
                        bit(8, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 95:
                        bit(8, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 96:
                        bit(16, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 97:
                        bit(16, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 98:
                        bit(16, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 99:
                        bit(16, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 100:
                        bit(16, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 101:
                        bit(16, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 102:
                        bit(16, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 103:
                        bit(16, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 104:
                        bit(32, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 105:
                        bit(32, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 106:
                        bit(32, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 107:
                        bit(32, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 108:
                        bit(32, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 109:
                        bit(32, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 110:
                        bit(32, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 111:
                        bit(32, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 112:
                        bit(64, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 113:
                        bit(64, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 114:
                        bit(64, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 115:
                        bit(64, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 116:
                        bit(64, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 117:
                        bit(64, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 118:
                        bit(64, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 119:
                        bit(64, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 120:
                        bit(128, _B);
                        i += 8;
                        goto EXEC_CB;
                    case 121:
                        bit(128, _C);
                        i += 8;
                        goto EXEC_CB;
                    case 122:
                        bit(128, _D);
                        i += 8;
                        goto EXEC_CB;
                    case 123:
                        bit(128, _E);
                        i += 8;
                        goto EXEC_CB;
                    case 124:
                        bit(128, _H);
                        i += 8;
                        goto EXEC_CB;
                    case 125:
                        bit(128, _L);
                        i += 8;
                        goto EXEC_CB;
                    case 126:
                        bit(128, readMem(HL()));
                        i += 12;
                        goto EXEC_CB;
                    case 127:
                        bit(128, _A);
                        i += 8;
                        goto EXEC_CB;
                    case 128:
                        _B = (res(1, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 129:
                        _C = (res(1, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 130:
                        _D = (res(1, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 131:
                        _E = (res(1, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 132:
                        _H = (res(1, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 133:
                        _L = (res(1, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 134: {
                        // ii = HL();
                        writeMem(ii, res(1, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 135:
                        _A = (res(1, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 136:
                        _B = (res(2, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 137:
                        _C = (res(2, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 138:
                        _D = (res(2, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 139:
                        _E = (res(2, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 140:
                        _H = (res(2, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 141:
                        _L = (res(2, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 142: {
                        // ii = HL();
                        writeMem(ii, res(2, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 143:
                        _A = (res(2, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 144:
                        _B = (res(4, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 145:
                        _C = (res(4, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 146:
                        _D = (res(4, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 147:
                        _E = (res(4, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 148:
                        _H = (res(4, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 149:
                        _L = (res(4, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 150: {
                        // ii = HL();
                        writeMem(ii, res(4, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 151:
                        _A = (res(4, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 152:
                        _B = (res(8, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 153:
                        _C = (res(8, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 154:
                        _D = (res(8, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 155:
                        _E = (res(8, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 156:
                        _H = (res(8, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 157:
                        _L = (res(8, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 158: {
                        // ii = HL();
                        writeMem(ii, res(8, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 159:
                        _A = (res(8, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 160:
                        _B = (res(16, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 161:
                        _C = (res(16, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 162:
                        _D = (res(16, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 163:
                        _E = (res(16, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 164:
                        _H = (res(16, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 165:
                        _L = (res(16, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 166: {
                        // ii = HL();
                        writeMem(ii, res(16, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 167:
                        _A = (res(16, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 168:
                        _B = (res(32, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 169:
                        _C = (res(32, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 170:
                        _D = (res(32, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 171:
                        _E = (res(32, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 172:
                        _H = (res(32, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 173:
                        _L = (res(32, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 174: {
                        // ii = HL();
                        writeMem(ii, res(32, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 175:
                        _A = (res(32, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 176:
                        _B = (res(64, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 177:
                        _C = (res(64, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 178:
                        _D = (res(64, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 179:
                        _E = (res(64, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 180:
                        _H = (res(64, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 181:
                        _L = (res(64, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 182: {
                        // ii = HL();
                        writeMem(ii, res(64, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 183:
                        _A = (res(64, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 184:
                        _B = (res(128, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 185:
                        _C = (res(128, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 186:
                        _D = (res(128, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 187:
                        _E = (res(128, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 188:
                        _H = (res(128, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 189:
                        _L = (res(128, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 190: {
                        // ii = HL();
                        writeMem(ii, res(128, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 191:
                        _A = (res(128, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 192:
                        _B = (set(1, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 193:
                        _C = (set(1, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 194:
                        _D = (set(1, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 195:
                        _E = (set(1, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 196:
                        _H = (set(1, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 197:
                        _L = (set(1, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 198: {
                        // ii = HL();
                        writeMem(ii, set(1, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 199:
                        _A = (set(1, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 200:
                        _B = (set(2, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 201:
                        _C = (set(2, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 202:
                        _D = (set(2, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 203:
                        _E = (set(2, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 204:
                        _H = (set(2, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 205:
                        _L = (set(2, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 206: {
                        // ii = HL();
                        writeMem(ii, set(2, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 207:
                        _A = (set(2, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 208:
                        _B = (set(4, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 209:
                        _C = (set(4, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 210:
                        _D = (set(4, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 211:
                        _E = (set(4, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 212:
                        _H = (set(4, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 213:
                        _L = (set(4, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 214: {
                        // ii = HL();
                        writeMem(ii, set(4, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 215:
                        _A = (set(4, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 216:
                        _B = (set(8, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 217:
                        _C = (set(8, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 218:
                        _D = (set(8, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 219:
                        _E = (set(8, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 220:
                        _H = (set(8, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 221:
                        _L = (set(8, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 222: {
                        // ii = HL();
                        writeMem(ii, set(8, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 223:
                        _A = (set(8, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 224:
                        _B = (set(16, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 225:
                        _C = (set(16, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 226:
                        _D = (set(16, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 227:
                        _E = (set(16, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 228:
                        _H = (set(16, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 229:
                        _L = (set(16, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 230: {
                        // ii = HL();
                        writeMem(ii, set(16, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 231:
                        _A = (set(16, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 232:
                        _B = (set(32, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 233:
                        _C = (set(32, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 234:
                        _D = (set(32, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 235:
                        _E = (set(32, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 236:
                        _H = (set(32, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 237:
                        _L = (set(32, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 238: {
                        // ii = HL();
                        writeMem(ii, set(32, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 239:
                        _A = (set(32, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 240:
                        _B = (set(64, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 241:
                        _C = (set(64, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 242:
                        _D = (set(64, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 243:
                        _E = (set(64, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 244:
                        _H = (set(64, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 245:
                        _L = (set(64, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 246: {
                        // ii = HL();
                        writeMem(ii, set(64, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 247:
                        _A = (set(64, _A));
                        i += 8;
                        goto EXEC_CB;
                    case 248:
                        _B = (set(128, _B));
                        i += 8;
                        goto EXEC_CB;
                    case 249:
                        _C = (set(128, _C));
                        i += 8;
                        goto EXEC_CB;
                    case 250:
                        _D = (set(128, _D));
                        i += 8;
                        goto EXEC_CB;
                    case 251:
                        _E = (set(128, _E));
                        i += 8;
                        goto EXEC_CB;
                    case 252:
                        _H = (set(128, _H));
                        i += 8;
                        goto EXEC_CB;
                    case 253:
                        _L = (set(128, _L));
                        i += 8;
                        goto EXEC_CB;
                    case 254: {
                        // ii = HL();
                        writeMem(ii, set(128, readMem(ii)));
                        i += 15;
                    }
                        goto EXEC_CB;
                    case 255:
                        _A = (set(128, _A));
                        i += 8;
                        goto EXEC_CB;
                    default:
                        i += 0;
                }
            EXEC_CB:;
            } break;
            case 211:
                outb(readMem(_PC++), _A);
                i += 11;
                break;
            case 219:
                _A = (inb(readMem(_PC++)));
                i += 11;
                break;
            case 227: {
                i_3_ = HL();
                i_2_ = _SP;
                setHL(readMemWord(i_2_));
                writeMemWord(i_2_, i_3_);
                i += 19;
                break;
            }
            case 235: {
                i_2_ = HL();
                setHL(DE());
                setDE(i_2_);
                i += 4;
                break;
            }
            case 243:
                setIFF1(false);
                setIFF2(false);
                i += 4;
                break;
            case 251:
                setIFF1(true);
                setIFF2(true);
                i += 4;
                break;
            case 196:
                if (!fZ) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 204:
                if (fZ) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 212:
                if (!fC) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 220:
                if (fC) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 228:
                if (!fPV) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 236:
                if (fPV) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 244:
                if (!fS) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 252:
                if (fS) {
                    i_2_ = readMemWord((_PC = _PC + 2) - 2);
                    pushpc();
                    _PC = (i_2_);
                    i += 17;
                } else {
                    _PC = (_PC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 197:
                pushw(BC());
                i += 11;
                break;
            case 205: {
                i_2_ = readMemWord((_PC = _PC + 2) - 2);
                pushpc();
                _PC = (i_2_);
                i += 17;
                break;
            }
            case 213:
                pushw(DE());
                i += 11;
                break;

            case 221:
            case 253:
                if (opcode == 221) {
                    _ID = _IX;
                } else {
                    _ID = _IY;
                }
                {
                    int ii;
                    int ii2;
                    _R += (1);
                    switch (readMem(_PC++)) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 10:
                        case 11:
                        case 12:
                        case 13:
                        case 14:
                        case 15:
                        case 16:
                        case 17:
                        case 18:
                        case 19:
                        case 20:
                        case 21:
                        case 22:
                        case 23:
                        case 24:
                        case 26:
                        case 27:
                        case 28:
                        case 29:
                        case 30:
                        case 31:
                        case 32:
                        case 39:
                        case 40:
                        case 47:
                        case 48:
                        case 49:
                        case 50:
                        case 51:
                        case 55:
                        case 56:
                        case 58:
                        case 59:
                        case 60:
                        case 61:
                        case 62:
                        case 63:
                        case 64:
                        case 65:
                        case 66:
                        case 67:
                        case 71:
                        case 72:
                        case 73:
                        case 74:
                        case 75:
                        case 79:
                        case 80:
                        case 81:
                        case 82:
                        case 83:
                        case 87:
                        case 88:
                        case 89:
                        case 90:
                        case 91:
                        case 95:
                        case 120:
                        case 121:
                        case 122:
                        case 123:
                        case 127:
                        case 128:
                        case 129:
                        case 130:
                        case 131:
                        case 135:
                        case 136:
                        case 137:
                        case 138:
                        case 139:
                        case 143:
                        case 144:
                        case 145:
                        case 146:
                        case 147:
                        case 151:
                        case 152:
                        case 153:
                        case 154:
                        case 155:
                        case 159:
                        case 160:
                        case 161:
                        case 162:
                        case 163:
                        case 167:
                        case 168:
                        case 169:
                        case 170:
                        case 171:
                        case 175:
                        case 176:
                        case 177:
                        case 178:
                        case 179:
                        case 183:
                        case 184:
                        case 185:
                        case 186:
                        case 187:
                        case 191:
                        case 192:
                        case 193:
                        case 194:
                        case 195:
                        case 196:
                        case 197:
                        case 198:
                        case 199:
                        case 200:
                        case 201:
                        case 202:
                        case 204:
                        case 205:
                        case 206:
                        case 207:
                        case 208:
                        case 209:
                        case 210:
                        case 211:
                        case 212:
                        case 213:
                        case 214:
                        case 215:
                        case 216:
                        case 217:
                        case 218:
                        case 219:
                        case 220:
                        case 221:
                        case 222:
                        case 223:
                        case 224:
                        case 226:
                        case 228:
                        case 230:
                        case 231:
                        case 232:
                        case 234:
                        case 235:
                        case 236:
                        case 237:
                        case 238:
                        case 239:
                        case 240:
                        case 241:
                        case 242:
                        case 243:
                        case 244:
                        case 245:
                        case 246:
                        case 247:
                        case 248:
                            _PC = _PC - 1 & 0xffff;
                            _R += (-1);
                            i += 4;
                            goto EXEC_ID;
                        case 9:
                            _ID = add16(_ID, BC());
                            i += 15;
                            goto EXEC_ID;
                        case 25:
                            _ID = add16(_ID, DE());
                            i += 15;
                            goto EXEC_ID;
                        case 41: {
                            ii  = _ID;
                            _ID = add16(ii, ii);
                            i += 15;
                        }
                            goto EXEC_ID;
                        case 57:
                            _ID = add16(_ID, _SP);
                            i += 15;
                            goto EXEC_ID;
                        case 33:
                            _ID = readMemWord((_PC = _PC + 2) - 2);
                            i += 14;
                            goto EXEC_ID;
                        case 34:
                            writeMemWord(readMemWord((_PC = _PC + 2) - 2), _ID);
                            i += 20;
                            goto EXEC_ID;
                        case 42:
                            _ID = readMemWord(readMemWord((_PC = _PC + 2) - 2));
                            i += 20;
                            goto EXEC_ID;
                        case 35:
                            _ID = _ID + 1 & 0xffff;
                            i += 10;
                            goto EXEC_ID;
                        case 43:
                            _ID = _ID - 1 & 0xffff;
                            i += 10;
                            goto EXEC_ID;
                        case 36:
                            setIDH(inc8(_ID >> 8));
                            i += 8;
                            goto EXEC_ID;
                        case 44:
                            setIDL(inc8(_ID & 0xff));
                            i += 8;
                            goto EXEC_ID;
                        case 52: {
                            ii = ID_d();
                            writeMem(ii, inc8(readMem(ii)));
                            i += 23;
                        }
                            goto EXEC_ID;
                        case 37:
                            setIDH(dec8(_ID >> 8));
                            i += 8;
                            goto EXEC_ID;
                        case 45:
                            setIDL(dec8(_ID & 0xff));
                            i += 8;
                            goto EXEC_ID;
                        case 53: {
                            ii = ID_d();
                            writeMem(ii, dec8(readMem(ii)));
                            i += 23;
                        }
                            goto EXEC_ID;
                        case 38:
                            setIDH(readMem(_PC++));
                            i += 11;
                            goto EXEC_ID;
                        case 46:
                            setIDL(nxtpcb());
                            i += 11;
                            goto EXEC_ID;
                        case 54: {
                            ii = ID_d();
                            writeMem(ii, readMem(_PC++));
                            i += 19;
                        }
                            goto EXEC_ID;
                        case 68:
                            _B = (_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 69:
                            _B = (_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 70:
                            _B = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 76:
                            _C = (_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 77:
                            _C = (_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 78:
                            _C = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 84:
                            _D = (_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 85:
                            _D = (_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 86:
                            _D = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 92:
                            _E = (_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 93:
                            _E = (_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 94:
                            _E = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 96:
                            setIDH(_B);
                            i += 8;
                            goto EXEC_ID;
                        case 97:
                            setIDH(_C);
                            i += 8;
                            goto EXEC_ID;
                        case 98:
                            setIDH(_D);
                            i += 8;
                            goto EXEC_ID;
                        case 99:
                            setIDH(_E);
                            i += 8;
                            goto EXEC_ID;
                        case 100:
                            i += 8;
                            goto EXEC_ID;
                        case 101:
                            setIDH(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 102:
                            _H = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 103:
                            setIDH(_A);
                            i += 8;
                            goto EXEC_ID;
                        case 104:
                            setIDL(_B);
                            i += 8;
                            goto EXEC_ID;
                        case 105:
                            setIDL(_C);
                            i += 8;
                            goto EXEC_ID;
                        case 106:
                            setIDL(_D);
                            i += 8;
                            goto EXEC_ID;
                        case 107:
                            setIDL(_E);
                            i += 8;
                            goto EXEC_ID;
                        case 108:
                            setIDL(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 109:
                            i += 8;
                            goto EXEC_ID;
                        case 110:
                            _L = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 111:
                            setIDL(_A);
                            i += 8;
                            goto EXEC_ID;
                        case 112:
                            writeMem(ID_d(), _B);
                            i += 19;
                            goto EXEC_ID;
                        case 113:
                            writeMem(ID_d(), _C);
                            i += 19;
                            goto EXEC_ID;
                        case 114:
                            writeMem(ID_d(), _D);
                            i += 19;
                            goto EXEC_ID;
                        case 115:
                            writeMem(ID_d(), _E);
                            i += 19;
                            goto EXEC_ID;
                        case 116:
                            writeMem(ID_d(), _H);
                            i += 19;
                            goto EXEC_ID;
                        case 117:
                            writeMem(ID_d(), _L);
                            i += 19;
                            goto EXEC_ID;
                        case 119:
                            writeMem(ID_d(), _A);
                            i += 19;
                            goto EXEC_ID;
                        case 124:
                            _A = (_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 125:
                            _A = (_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 126:
                            _A = (readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 132:
                            add_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 133:
                            add_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 134:
                            add_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 140:
                            adc_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 141:
                            adc_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 142:
                            adc_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 148:
                            sub_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 149:
                            sub_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 150:
                            sub_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 156:
                            sbc_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 157:
                            sbc_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 158:
                            sbc_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 164:
                            and_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 165:
                            and_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 166:
                            and_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 172:
                            xor_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 173:
                            xor_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 174:
                            xor_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 180:
                            or_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 181:
                            or_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 182:
                            or_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 188:
                            cp_a(_ID >> 8);
                            i += 8;
                            goto EXEC_ID;
                        case 189:
                            cp_a(_ID & 0xff);
                            i += 8;
                            goto EXEC_ID;
                        case 190:
                            cp_a(readMem(ID_d()));
                            i += 19;
                            goto EXEC_ID;
                        case 225:
                            _ID = (popw());
                            i += 14;
                            goto EXEC_ID;
                        case 233:
                            _PC = (_ID);
                            i += 8;
                            goto EXEC_ID;
                        case 249:
                            _SP = (_ID);
                            i += 10;
                            goto EXEC_ID;
                        case 203: {
                            ii  = ID_d();
                            ii2 = readMem(_PC++);
                            {
                                switch (ii2) {
                                    case 0:
                                        _B = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 1:
                                        _C = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 2:
                                        _D = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 3:
                                        _E = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 4:
                                        _H = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 5:
                                        _L = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 6:
                                        writeMem(ii, rlc(readMem(ii)));
                                        break;
                                    case 7:
                                        _A = (ii2 = rlc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 8:
                                        _B = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 9:
                                        _C = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 10:
                                        _D = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 11:
                                        _E = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 12:
                                        _H = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 13:
                                        _L = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 14:
                                        writeMem(ii, rrc(readMem(ii)));
                                        break;
                                    case 15:
                                        _A = (ii2 = rrc(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 16:
                                        _B = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 17:
                                        _C = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 18:
                                        _D = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 19:
                                        _E = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 20:
                                        _H = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 21:
                                        _L = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 22:
                                        writeMem(ii, rl(readMem(ii)));
                                        break;
                                    case 23:
                                        _A = (ii2 = rl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 24:
                                        _B = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 25:
                                        _C = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 26:
                                        _D = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 27:
                                        _E = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 28:
                                        _H = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 29:
                                        _L = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 30:
                                        writeMem(ii, rr(readMem(ii)));
                                        break;
                                    case 31:
                                        _A = (ii2 = rr(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 32:
                                        _B = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 33:
                                        _C = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 34:
                                        _D = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 35:
                                        _E = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 36:
                                        _H = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 37:
                                        _L = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 38:
                                        writeMem(ii, sla(readMem(ii)));
                                        break;
                                    case 39:
                                        _A = (ii2 = sla(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 40:
                                        _B = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 41:
                                        _C = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 42:
                                        _D = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 43:
                                        _E = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 44:
                                        _H = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 45:
                                        _L = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 46:
                                        writeMem(ii, sra(readMem(ii)));
                                        break;
                                    case 47:
                                        _A = (ii2 = sra(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 48:
                                        _B = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 49:
                                        _C = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 50:
                                        _D = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 51:
                                        _E = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 52:
                                        _H = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 53:
                                        _L = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 54:
                                        writeMem(ii, sls(readMem(ii)));
                                        break;
                                    case 55:
                                        _A = (ii2 = sls(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 56:
                                        _B = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 57:
                                        _C = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 58:
                                        _D = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 59:
                                        _E = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 60:
                                        _H = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 61:
                                        _L = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 62:
                                        writeMem(ii, srl(readMem(ii)));
                                        break;
                                    case 63:
                                        _A = (ii2 = srl(readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 64:
                                    case 65:
                                    case 66:
                                    case 67:
                                    case 68:
                                    case 69:
                                    case 70:
                                    case 71:
                                        bit(1, readMem(ii));
                                        break;
                                    case 72:
                                    case 73:
                                    case 74:
                                    case 75:
                                    case 76:
                                    case 77:
                                    case 78:
                                    case 79:
                                        bit(2, readMem(ii));
                                        break;
                                    case 80:
                                    case 81:
                                    case 82:
                                    case 83:
                                    case 84:
                                    case 85:
                                    case 86:
                                    case 87:
                                        bit(4, readMem(ii));
                                        break;
                                    case 88:
                                    case 89:
                                    case 90:
                                    case 91:
                                    case 92:
                                    case 93:
                                    case 94:
                                    case 95:
                                        bit(8, readMem(ii));
                                        break;
                                    case 96:
                                    case 97:
                                    case 98:
                                    case 99:
                                    case 100:
                                    case 101:
                                    case 102:
                                    case 103:
                                        bit(16, readMem(ii));
                                        break;
                                    case 104:
                                    case 105:
                                    case 106:
                                    case 107:
                                    case 108:
                                    case 109:
                                    case 110:
                                    case 111:
                                        bit(32, readMem(ii));
                                        break;
                                    case 112:
                                    case 113:
                                    case 114:
                                    case 115:
                                    case 116:
                                    case 117:
                                    case 118:
                                    case 119:
                                        bit(64, readMem(ii));
                                        break;
                                    case 120:
                                    case 121:
                                    case 122:
                                    case 123:
                                    case 124:
                                    case 125:
                                    case 126:
                                    case 127:
                                        bit(128, readMem(ii));
                                        break;
                                    case 128:
                                        _B = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 129:
                                        _C = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 130:
                                        _D = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 131:
                                        _E = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 132:
                                        _H = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 133:
                                        _L = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 134:
                                        writeMem(ii, res(1, readMem(ii)));
                                        break;
                                    case 135:
                                        _A = (ii2 = res(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 136:
                                        _B = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 137:
                                        _C = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 138:
                                        _D = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 139:
                                        _E = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 140:
                                        _H = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 141:
                                        _L = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 142:
                                        writeMem(ii, res(2, readMem(ii)));
                                        break;
                                    case 143:
                                        _A = (ii2 = res(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 144:
                                        _B = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 145:
                                        _C = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 146:
                                        _D = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 147:
                                        _E = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 148:
                                        _H = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 149:
                                        _L = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 150:
                                        writeMem(ii, res(4, readMem(ii)));
                                        break;
                                    case 151:
                                        _A = (ii2 = res(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 152:
                                        _B = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 153:
                                        _C = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 154:
                                        _D = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 155:
                                        _E = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 156:
                                        _H = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 157:
                                        _L = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 158:
                                        writeMem(ii, res(8, readMem(ii)));
                                        break;
                                    case 159:
                                        _A = (ii2 = res(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 160:
                                        _B = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 161:
                                        _C = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 162:
                                        _D = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 163:
                                        _E = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 164:
                                        _H = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 165:
                                        _L = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 166:
                                        writeMem(ii, res(16, readMem(ii)));
                                        break;
                                    case 167:
                                        _A = (ii2 = res(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 168:
                                        _B = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 169:
                                        _C = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 170:
                                        _D = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 171:
                                        _E = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 172:
                                        _H = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 173:
                                        _L = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 174:
                                        writeMem(ii, res(32, readMem(ii)));
                                        break;
                                    case 175:
                                        _A = (ii2 = res(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 176:
                                        _B = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 177:
                                        _C = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 178:
                                        _D = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 179:
                                        _E = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 180:
                                        _H = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 181:
                                        _L = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 182:
                                        writeMem(ii, res(64, readMem(ii)));
                                        break;
                                    case 183:
                                        _A = (ii2 = res(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 184:
                                        _B = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 185:
                                        _C = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 186:
                                        _D = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 187:
                                        _E = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 188:
                                        _H = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 189:
                                        _L = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 190:
                                        writeMem(ii, res(128, readMem(ii)));
                                        break;
                                    case 191:
                                        _A = (ii2 = res(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 192:
                                        _B = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 193:
                                        _C = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 194:
                                        _D = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 195:
                                        _E = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 196:
                                        _H = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 197:
                                        _L = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 198:
                                        writeMem(ii, set(1, readMem(ii)));
                                        break;
                                    case 199:
                                        _A = (ii2 = set(1, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 200:
                                        _B = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 201:
                                        _C = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 202:
                                        _D = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 203:
                                        _E = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 204:
                                        _H = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 205:
                                        _L = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 206:
                                        writeMem(ii, set(2, readMem(ii)));
                                        break;
                                    case 207:
                                        _A = (ii2 = set(2, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 208:
                                        _B = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 209:
                                        _C = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 210:
                                        _D = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 211:
                                        _E = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 212:
                                        _H = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 213:
                                        _L = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 214:
                                        writeMem(ii, set(4, readMem(ii)));
                                        break;
                                    case 215:
                                        _A = (ii2 = set(4, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 216:
                                        _B = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 217:
                                        _C = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 218:
                                        _D = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 219:
                                        _E = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 220:
                                        _H = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 221:
                                        _L = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 222:
                                        writeMem(ii, set(8, readMem(ii)));
                                        break;
                                    case 223:
                                        _A = (ii2 = set(8, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 224:
                                        _B = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 225:
                                        _C = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 226:
                                        _D = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 227:
                                        _E = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 228:
                                        _H = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 229:
                                        _L = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 230:
                                        writeMem(ii, set(16, readMem(ii)));
                                        break;
                                    case 231:
                                        _A = (ii2 = set(16, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 232:
                                        _B = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 233:
                                        _C = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 234:
                                        _D = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 235:
                                        _E = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 236:
                                        _H = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 237:
                                        _L = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 238:
                                        writeMem(ii, set(32, readMem(ii)));
                                        break;
                                    case 239:
                                        _A = (ii2 = set(32, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 240:
                                        _B = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 241:
                                        _C = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 242:
                                        _D = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 243:
                                        _E = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 244:
                                        _H = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 245:
                                        _L = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 246:
                                        writeMem(ii, set(64, readMem(ii)));
                                        break;
                                    case 247:
                                        _A = (ii2 = set(64, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 248:
                                        _B = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 249:
                                        _C = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 250:
                                        _D = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 251:
                                        _E = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 252:
                                        _H = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 253:
                                        _L = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                    case 254:
                                        writeMem(ii, set(128, readMem(ii)));
                                        break;
                                    case 255:
                                        _A = (ii2 = set(128, readMem(ii)));
                                        writeMem(ii, ii2);
                                        break;
                                }
                            }
                            i += (ii2 & 0xc0) == 64 ? 20 : 23;
                        }
                            goto EXEC_ID;
                        case 227: {
                            ii  = _ID;
                            ii2 = _SP;
                            _ID = (readMemWord(ii2));
                            writeMemWord(ii2, ii);
                            i += 23;
                        }
                            goto EXEC_ID;
                        case 229:
                            pushw(_ID);
                            i += 15;
                            goto EXEC_ID;
                        default:
                            i += 0;
                            goto EXEC_ID;
                    }

                EXEC_ID:;
                }

                if (opcode == 221) {
                    _IX = _ID;
                } else {
                    _IY = _ID;
                }
                break;

            case 229:
                pushw(HL());
                i += 11;
                break;
            case 237: {
                int ii2;
                int ii3;
                _R += (1);
                switch (readMem(_PC++)) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                    case 28:
                    case 29:
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                    case 38:
                    case 39:
                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                    case 46:
                    case 47:
                    case 48:
                    case 49:
                    case 50:
                    case 51:
                    case 52:
                    case 53:
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 59:
                    case 60:
                    case 61:
                    case 62:
                    case 63:
                    case 127:
                    case 128:
                    case 129:
                    case 130:
                    case 131:
                    case 132:
                    case 133:
                    case 134:
                    case 135:
                    case 136:
                    case 137:
                    case 138:
                    case 139:
                    case 140:
                    case 141:
                    case 142:
                    case 143:
                    case 144:
                    case 145:
                    case 146:
                    case 147:
                    case 148:
                    case 149:
                    case 150:
                    case 151:
                    case 152:
                    case 153:
                    case 154:
                    case 155:
                    case 156:
                    case 157:
                    case 158:
                    case 159:
                    case 164:
                    case 165:
                    case 166:
                    case 167:
                    case 172:
                    case 173:
                    case 174:
                    case 175:
                    case 180:
                    case 181:
                    case 182:
                    case 183:
                        i += 8;
                        goto EXEC_ED;
                    case 64:
                        _B = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 72:
                        _C = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 80:
                        _D = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 88:
                        _E = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 96:
                        _H = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 104:
                        _L = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 112:
                        in_bc();
                        i += 12;
                        goto EXEC_ED;
                    case 120:
                        _A = (in_bc());
                        i += 12;
                        goto EXEC_ED;
                    case 65:
                        outb(_C, _B);
                        i += 12;
                        goto EXEC_ED;
                    case 73:
                        outb(_C, _C);
                        i += 12;
                        goto EXEC_ED;
                    case 81:
                        outb(_C, _D);
                        i += 12;
                        goto EXEC_ED;
                    case 89:
                        outb(_C, _E);
                        i += 12;
                        goto EXEC_ED;
                    case 97:
                        outb(_C, _H);
                        i += 12;
                        goto EXEC_ED;
                    case 105:
                        outb(_C, _L);
                        i += 12;
                        goto EXEC_ED;
                    case 113:
                        outb(_C, 0);
                        i += 12;
                        goto EXEC_ED;
                    case 121:
                        outb(_C, _A);
                        i += 12;
                        goto EXEC_ED;
                    case 66:
                        setHL(sbc16(HL(), BC()));
                        i += 15;
                        goto EXEC_ED;
                    case 74:
                        setHL(adc16(HL(), BC()));
                        i += 15;
                        goto EXEC_ED;
                    case 82:
                        setHL(sbc16(HL(), DE()));
                        i += 15;
                        goto EXEC_ED;
                    case 90:
                        setHL(adc16(HL(), DE()));
                        i += 15;
                        goto EXEC_ED;
                    case 98: {
                        ii2 = HL();
                        setHL(sbc16(ii2, ii2));
                        i += 15;
                    }
                        goto EXEC_ED;
                    case 106: {
                        ii2 = HL();
                        setHL(adc16(ii2, ii2));
                        i += 15;
                    }
                        goto EXEC_ED;
                    case 114:
                        setHL(sbc16(HL(), _SP));
                        i += 15;
                        goto EXEC_ED;
                    case 122:
                        setHL(adc16(HL(), _SP));
                        i += 15;
                        goto EXEC_ED;
                    case 67:
                        writeMemWord(readMemWord((_PC = _PC + 2) - 2), BC());
                        i += 20;
                        goto EXEC_ED;
                    case 75:
                        setBC(readMemWord(readMemWord((_PC = _PC + 2) - 2)));
                        i += 20;
                        goto EXEC_ED;
                    case 83:
                        writeMemWord(readMemWord((_PC = _PC + 2) - 2), DE());
                        i += 20;
                        goto EXEC_ED;
                    case 91:
                        setDE(readMemWord(readMemWord((_PC = _PC + 2) - 2)));
                        i += 20;
                        goto EXEC_ED;
                    case 99:
                        writeMemWord(readMemWord((_PC = _PC + 2) - 2), HL());
                        i += 20;
                        goto EXEC_ED;
                    case 107:
                        setHL(readMemWord(readMemWord((_PC = _PC + 2) - 2)));
                        i += 20;
                        goto EXEC_ED;
                    case 115:
                        writeMemWord(readMemWord((_PC = _PC + 2) - 2), _SP);
                        i += 20;
                        goto EXEC_ED;
                    case 123:
                        _SP = (readMemWord(readMemWord((_PC = _PC + 2) - 2)));
                        i += 20;
                        goto EXEC_ED;
                    case 68:
                    case 76:
                    case 84:
                    case 92:
                    case 100:
                    case 108:
                    case 116:
                    case 124: {
                        int ii = _A;
                        _A     = (0);
                        sub_a(ii);
                        i += 8;
                    }
                        goto EXEC_ED;
                    case 69:
                    case 85:
                    case 101:
                    case 117:
                        setIFF1(_IFF2);
                        poppc();
                        i += 14;
                        goto EXEC_ED;
                    case 77:
                    case 93:
                    case 109:
                    case 125:
                        poppc();
                        i += 14;
                        goto EXEC_ED;
                    case 70:
                    case 78:
                    case 102:
                    case 110:
                        _IM = (0);
                        i += 8;
                        goto EXEC_ED;
                    case 86:
                    case 118:
                        _IM = (1);
                        i += 8;
                        goto EXEC_ED;
                    case 94:
                    case 126:
                        _IM = (2);
                        i += 8;
                        goto EXEC_ED;
                    case 71:
                        _I = (_A);
                        i += 9;
                        goto EXEC_ED;
                    case 79:
                        _R  = _A;
                        _R7 = _A & 0x80;
                        i += 9;
                        goto EXEC_ED;
                    case 87: {
                        int ii = _I;
                        fS     = ((ii & 0x80) != 0);
                        f3     = ((ii & 0x8) != 0);
                        f5     = ((ii & 0x20) != 0);
                        fZ     = (ii == 0);
                        fPV    = (_IFF2);
                        fH     = (false);
                        fN     = (false);
                        _A     = (ii);
                        i += 9;
                    }
                        goto EXEC_ED;
                    case 95: {
                        int ii = _R & 0x7f | _R7;
                        fS     = ((ii & 0x80) != 0);
                        f3     = ((ii & 0x8) != 0);
                        f5     = ((ii & 0x20) != 0);
                        fZ     = (ii == 0);
                        fPV    = (_IFF2);
                        fH     = (false);
                        fN     = (false);
                        _A     = (ii);
                        i += 9;
                    }
                        goto EXEC_ED;
                    case 103: {
                        int ii    = _A;
                        int i_76_ = readMem(HL());
                        int i_77_ = i_76_;
                        i_76_     = i_76_ >> 4 | ii << 4;
                        ii        = ii & 0xf0 | i_77_ & 0xf;
                        writeMem(HL(), i_76_);
                        fS  = ((ii & 0x80) != 0);
                        f3  = ((ii & 0x8) != 0);
                        f5  = ((ii & 0x20) != 0);
                        fZ  = (ii == 0);
                        fPV = (parity[ii]);
                        fH  = (false);
                        fN  = (false);
                        _A  = (ii);
                        i += 18;
                    }
                        goto EXEC_ED;
                    case 111: {
                        int ii    = _A;
                        int i_74_ = readMem(HL());
                        int i_75_ = i_74_;
                        i_74_     = i_74_ << 4 | ii & 0xf;
                        ii        = ii & 0xf0 | i_75_ >> 4;
                        writeMem(HL(), i_74_ & 0xff);
                        fS  = ((ii & 0x80) != 0);
                        f3  = ((ii & 0x8) != 0);
                        f5  = ((ii & 0x20) != 0);
                        fZ  = (ii == 0);
                        fPV = (parity[ii]);
                        fH  = (false);
                        fN  = (false);
                        _A  = (ii);
                        i += 18;
                    }
                        goto EXEC_ED;
                    case 160:
                        writeMem(DE(), readMem(HL()));
                        setDE((_D << 8 | _E) + 1 & 0xffff);
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fH  = (false);
                        fN  = (false);
                        i += 16;
                        goto EXEC_ED;
                    case 161: {
                        ii2 = fC;
                        cp_a(readMem(HL()));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fC  = (ii2);
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 162:
                        writeMem(HL(), inb(_C));
                        _B = (dec8(_B));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        i += 16;
                        goto EXEC_ED;
                    case 163:
                        _B = (dec8(_B));
                        outb(_C, readMem(HL()));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        i += 16;
                        goto EXEC_ED;
                    case 168:
                        writeMem(DE(), readMem(HL()));
                        setDE((_D << 8 | _E) - 1 & 0xffff);
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fH  = (false);
                        fN  = (false);
                        i += 16;
                        goto EXEC_ED;
                    case 169: {
                        // var bool = fC;
                        cp_a(readMem(HL()));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 170:
                        writeMem(HL(), inb(_C));
                        _B = (dec8(_B));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        i += 16;
                        goto EXEC_ED;
                    case 171:
                        _B = (dec8(_B));
                        outb(_C, readMem(HL()));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        i += 16;
                        goto EXEC_ED;
                    case 176: {
                        // var bool = false;
                        writeMem(DE(), readMem(HL()));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        setDE((_D << 8 | _E) + 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        ii2 = 21;
                        _R += (4);
                        if (BC() != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            fH  = (false);
                            fN  = (false);
                            fPV = (true);
                        } else {
                            ii2 -= 5;
                            fH  = (false);
                            fN  = (false);
                            fPV = (false);
                        }
                        i += ii2;
                    }
                        goto EXEC_ED;
                    case 177: {
                        ii2 = fC;
                        cp_a(readMem(HL()));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        ii3 = BC() != 0;
                        fPV = (ii3);
                        fC  = (ii2);
                        if (ii3 && !fZ) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 178: {
                        // var bool = false;
                        ii2 = 0;
                        writeMem(HL(), inb(_C));
                        _B = (ii2 = dec8(_B));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        if (ii2 != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 179: {
                        // var bool = false;
                        ii2 = 0;
                        _B  = (ii2 = dec8(_B));
                        outb(_C, readMem(HL()));
                        setHL((_H << 8 | _L) + 1 & 0xffff);
                        if (ii2 != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 184: {
                        // var bool = false;
                        ii2 = 21;
                        _R += (4);
                        writeMem(DE(), readMem(HL()));
                        setDE((_D << 8 | _E) - 1 & 0xffff);
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        if (BC() != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            fH  = (false);
                            fN  = (false);
                            fPV = (true);
                        } else {
                            ii2 -= 5;
                            fH  = (false);
                            fN  = (false);
                            fPV = (false);
                        }
                        i += ii2;
                    }
                        goto EXEC_ED;
                    case 185: {
                        ii2 = fC;
                        cp_a(readMem(HL()));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        setBC((_B << 8 | _C) - 1 & 0xffff);
                        ii3 = BC() != 0;
                        fPV = (ii3);
                        fC  = (ii2);
                        if (ii3 && !fZ) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 186: {
                        ii2 = 0;
                        writeMem(HL(), inb(BC() & 0xff));
                        _B = (ii2 = dec8(_B));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        if (ii2 != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                    }
                        goto EXEC_ED;
                    case 187: {
                        ii2 = 0;
                        _B  = (ii2 = dec8(_B));
                        outb(_C, readMem(HL()));
                        setHL((_H << 8 | _L) - 1 & 0xffff);
                        if (ii2 != 0) {
                            _PC = (_PC - 2 & 0xffff);
                            i += 21;
                            goto EXEC_ED;
                        }
                        i += 16;
                        goto EXEC_ED;
                    }
                    default:
                        i += 8;
                        goto EXEC_ED;
                }
            EXEC_ED:;
            } break;
            case 245: {
                int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
                         (fN ? 2 : 0) | (fC ? 1 : 0));
                pushw(_A << 8 | f);
                i += 11;
            } break;
            case 198:
                add_a(readMem(_PC++));
                i += 7;
                break;
            case 206:
                adc_a(readMem(_PC++));
                i += 7;
                break;
            case 214:
                sub_a(readMem(_PC++));
                i += 7;
                break;
            case 222:
                sbc_a(readMem(_PC++));
                i += 7;
                break;
            case 230:
                and_a(readMem(_PC++));
                i += 7;
                break;
            case 238:
                xor_a(readMem(_PC++));
                i += 7;
                break;
            case 246:
                or_a(readMem(_PC++));
                i += 7;
                break;
            case 254:
                cp_a(readMem(_PC++));
                i += 7;
                break;
            case 199:
                pushpc();
                _PC = (0);
                i += 11;
                break;
            case 207:
                pushpc();
                _PC = (8);
                i += 11;
                break;
            case 215:
                pushpc();
                _PC = (16);
                i += 11;
                break;
            case 223:
                pushpc();
                _PC = (24);
                i += 11;
                break;
            case 231:
                pushpc();
                _PC = (32);
                i += 11;
                break;
            case 239:
                pushpc();
                _PC = (40);
                i += 11;
                break;
            case 247:
                pushpc();
                _PC = (48);
                i += 11;
                break;
            case 255:
                pushpc();
                _PC = (56);
                i += 11;
        }
    }
}

int z80::readMem(int address)
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
int z80::readMemWord(int address)
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
void z80::writeMem(int address, int value)
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
void z80::writeMemWord(int address, int value)
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
void z80::preparaMemoriaMegarom(string str)
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
void z80::setF(int i)
{
    fS  = (i & 0x80) != 0;
    fZ  = (i & 0x40) != 0;
    f5  = (i & 0x20) != 0;
    fH  = (i & 0x10) != 0;
    f3  = (i & 0x8) != 0;
    fPV = (i & 0x4) != 0;
    fN  = (i & 0x2) != 0;
    fC  = (i & 0x1) != 0;
}
int8_t z80::bytef(int i)
{    // returns i between -128 to +127
    return ((i & 0x80) != 0) ? i - 256 : i;
}
void z80::setBC(int i)
{
    _B = (i >> 8);
    _C = (i & 0xff);
}
int z80::HL()
{
    return _H << 8 | _L;
}
int z80::BC()
{
    return _B << 8 | _C;
}
int z80::add16(int i, int i_9_)
{
    int i_10_ = i + i_9_;
    int i_11_ = i_10_ & 0xffff;
    f3        = ((i_11_ & 0x800) != 0);
    f5        = ((i_11_ & 0x2000) != 0);
    fC        = ((i_10_ & 0x10000) != 0);
    fH        = (((i & 0xfff) + (i_9_ & 0xfff) & 0x1000) != 0);
    fN        = (false);
    return i_11_;
}
void z80::setHL(int i)
{
    _H = (i >> 8);
    _L = (i & 0xff);
}
void z80::setDE(int i)
{
    _D = (i >> 8);
    _E = (i & 0xff);
}
int z80::DE()
{
    return _D << 8 | _E;
}
int z80::inc8(int i)
{
    bool flg      = i == 127;
    bool bool_59_ = ((i & 0xf) + 1 & 0x10) != 0;
    i             = i + 1 & 0xff;
    fS            = ((i & 0x80) != 0);
    f3            = ((i & 0x8) != 0);
    f5            = ((i & 0x20) != 0);
    fZ            = (i == 0);
    fPV           = (flg);
    fH            = (bool_59_);
    fN            = (false);
    return i;
}
int z80::dec8(int i)
{
    bool flg      = i == 128;
    bool bool_21_ = ((i & 0xf) - 1 & 0x10) != 0;
    i             = i - 1 & 0xff;
    fS            = ((i & 0x80) != 0);
    f3            = ((i & 0x8) != 0);
    f5            = ((i & 0x20) != 0);
    fZ            = (i == 0);
    fPV           = (flg);
    fH            = (bool_21_);
    fN            = (true);
    return i;
}
void z80::sub_a(int i)
{
    int i_87_ = _A;
    int i_88_ = i_87_ - i;
    int i_89_ = i_88_ & 0xff;
    fS        = ((i_89_ & 0x80) != 0);
    f3        = ((i_89_ & 0x8) != 0);
    f5        = ((i_89_ & 0x20) != 0);
    fZ        = (i_89_ == 0);
    fC        = ((i_88_ & 0x100) != 0);
    fPV       = (((i_87_ ^ i) & (i_87_ ^ i_89_) & 0x80) != 0);
    fH        = (((i_87_ & 0xf) - (i & 0xf) & 0x10) != 0);
    fN        = (true);
    _A        = (i_89_);
}
void z80::add_a(int i)
{
    int i_12_ = _A;
    int i_13_ = i_12_ + i;
    int i_14_ = i_13_ & 0xff;
    fS        = ((i_14_ & 0x80) != 0);
    f3        = ((i_14_ & 0x8) != 0);
    f5        = ((i_14_ & 0x20) != 0);
    fZ        = (i_14_ == 0);
    fC        = ((i_13_ & 0x100) != 0);
    fPV       = (((i_12_ ^ (i ^ 0xffffffff)) & (i_12_ ^ i_14_) & 0x80) != 0);
    fH        = (((i_12_ & 0xf) + (i & 0xf) & 0x10) != 0);
    fN        = (false);
    _A        = (i_14_);
}
void z80::adc_a(int i)
{
    int i_5_ = _A;
    int i_6_ = fC ? 1 : 0;
    int i_7_ = i_5_ + i + i_6_;
    int i_8_ = i_7_ & 0xff;
    fS       = ((i_8_ & 0x80) != 0);
    f3       = ((i_8_ & 0x8) != 0);
    f5       = ((i_8_ & 0x20) != 0);
    fZ       = (i_8_ == 0);
    fC       = ((i_7_ & 0x100) != 0);
    fPV      = (((i_5_ ^ (i ^ 0xffffffff)) & (i_5_ ^ i_8_) & 0x80) != 0);
    fH       = (((i_5_ & 0xf) + (i & 0xf) + i_6_ & 0x10) != 0);
    fN       = (false);
    _A       = (i_8_);
}
void z80::sbc_a(int i)
{
    int i_82_ = _A;
    int i_83_ = fC ? 1 : 0;
    int i_84_ = i_82_ - i - i_83_;
    int i_85_ = i_84_ & 0xff;
    fS        = ((i_85_ & 0x80) != 0);
    f3        = ((i_85_ & 0x8) != 0);
    f5        = ((i_85_ & 0x20) != 0);
    fZ        = (i_85_ == 0);
    fC        = ((i_84_ & 0x100) != 0);
    fPV       = (((i_82_ ^ i) & (i_82_ ^ i_85_) & 0x80) != 0);
    fH        = (((i_82_ & 0xf) - (i & 0xf) - i_83_ & 0x10) != 0);
    fN        = (true);
    _A        = (i_85_);
}
void z80::and_a(int i)
{
    int i_15_ = _A & i;
    fS        = ((i_15_ & 0x80) != 0);
    f3        = ((i_15_ & 0x8) != 0);
    f5        = ((i_15_ & 0x20) != 0);
    fH        = (true);
    fPV       = (parity[i_15_ & 0xff]);
    fZ        = (i_15_ == 0);
    fN        = (false);
    fC        = (false);
    _A        = (i_15_);
}
void z80::xor_a(int i)
{
    int i_90_ = (_A ^ i) & 0xff;
    fS        = ((i_90_ & 0x80) != 0);
    f3        = ((i_90_ & 0x8) != 0);
    f5        = ((i_90_ & 0x20) != 0);
    fH        = (false);
    fPV       = (parity[i_90_]);
    fZ        = (i_90_ == 0);
    fN        = (false);
    fC        = (false);
    _A        = (i_90_);
}
void z80::or_a(int i)
{
    int i_62_ = _A | i;
    fS        = ((i_62_ & 0x80) != 0);
    f3        = ((i_62_ & 0x8) != 0);
    f5        = ((i_62_ & 0x20) != 0);
    fH        = (false);
    fPV       = (parity[i_62_]);
    fZ        = (i_62_ == 0);
    fN        = (false);
    fC        = (false);
    _A        = (i_62_);
}
void z80::cp_a(int i)
{
    int i_17_ = _A;
    int i_18_ = i_17_ - i;
    int i_19_ = i_18_ & 0xff;
    fS        = ((i_19_ & 0x80) != 0);
    f3        = ((i & 0x8) != 0);
    f5        = ((i & 0x20) != 0);
    fN        = (true);
    fZ        = (i_19_ == 0);
    fC        = ((i_18_ & 0x100) != 0);
    fH        = (((i_17_ & 0xf) - (i & 0xf) & 0x10) != 0);
    fPV       = (((i_17_ ^ i) & (i_17_ ^ i_19_) & 0x80) != 0);
}
void z80::poppc()
{
    _PC = popw();
}
int z80::popw()
{
    int i     = _SP;
    int i_67_ = readMemWord(i++);
    _SP       = (++i & 0xffff);
    return i_67_;
}
void z80::pushpc()
{
    pushw(_PC);
}
void z80::pushw(int i)
{
    int i_70_ = _SP - 2 & 0xffff;
    _SP       = (i_70_);
    writeMemWord(i_70_, i);
}

int z80::rlc(int i)
{
    bool flg = (i & 0x80) != 0;
    if (flg)
        i = i << 1 | 0x1;
    else
        i <<= 1;
    i &= 0xff;
    fS  = ((i & 0x80) != 0);
    f3  = ((i & 0x8) != 0);
    f5  = ((i & 0x20) != 0);
    fZ  = (i == 0);
    fPV = (parity[i]);
    fH  = (false);
    fN  = (false);
    fC  = (flg);
    return i;
}
int z80::rrc(int i)
{
    bool flg = (i & 0x1) != 0;
    if (flg)
        i = i >> 1 | 0x80;
    else
        i >>= 1;
    fS  = ((i & 0x80) != 0);
    f3  = ((i & 0x8) != 0);
    f5  = ((i & 0x20) != 0);
    fZ  = (i == 0);
    fPV = (parity[i]);
    fH  = (false);
    fN  = (false);
    fC  = (flg);
    return i;
}
int z80::rl(int i)
{
    bool flg = (i & 0x80) != 0;
    if (fC)
        i = i << 1 | 0x1;
    else
        i <<= 1;
    i &= 0xff;
    fS  = ((i & 0x80) != 0);
    f3  = ((i & 0x8) != 0);
    f5  = ((i & 0x20) != 0);
    fZ  = (i == 0);
    fPV = (parity[i]);
    fH  = (false);
    fN  = (false);
    fC  = (flg);
    return i;
}
int z80::rr(int i)
{
    bool flg = (i & 0x1) != 0;
    if (fC)
        i = i >> 1 | 0x80;
    else
        i >>= 1;
    fS  = ((i & 0x80) != 0);
    f3  = ((i & 0x8) != 0);
    f5  = ((i & 0x20) != 0);
    fZ  = (i == 0);
    fPV = (parity[i]);
    fH  = (false);
    fN  = (false);
    fC  = (flg);
    return i;
}
int z80::sla(int i)
{
    bool flg = (i & 0x80) != 0;
    i        = i << 1 & 0xff;
    fS       = ((i & 0x80) != 0);
    f3       = ((i & 0x8) != 0);
    f5       = ((i & 0x20) != 0);
    fZ       = (i == 0);
    fPV      = (parity[i]);
    fH       = (false);
    fN       = (false);
    fC       = (flg);
    return i;
}
int z80::sra(int i)
{
    bool flg = (i & 0x1) != 0;
    i        = i >> 1 | i & 0x80;
    fS       = ((i & 0x80) != 0);
    f3       = ((i & 0x8) != 0);
    f5       = ((i & 0x20) != 0);
    fZ       = (i == 0);
    fPV      = (parity[i]);
    fH       = (false);
    fN       = (false);
    fC       = (flg);
    return i;
}
int z80::sls(int i)
{
    bool flg = (i & 0x80) != 0;
    i        = (i << 1 | 0x1) & 0xff;
    fS       = ((i & 0x80) != 0);
    f3       = ((i & 0x8) != 0);
    f5       = ((i & 0x20) != 0);
    fZ       = (i == 0);
    fPV      = (parity[i]);
    fH       = (false);
    fN       = (false);
    fC       = (flg);
    return i;
}
int z80::srl(int i)
{
    bool flg = (i & 0x1) != 0;
    i >>= 1;
    fS  = ((i & 0x80) != 0);
    f3  = ((i & 0x8) != 0);
    f5  = ((i & 0x20) != 0);
    fZ  = (i == 0);
    fPV = (parity[i]);
    fH  = (false);
    fN  = (false);
    fC  = (flg);
    return i;
}
void z80::bit(int i, int i_16_)
{
    bool flg = (i_16_ & i) != 0;
    fN       = (false);
    fH       = (true);
    f3       = ((i_16_ & 0x8) != 0);
    f5       = ((i_16_ & 0x20) != 0);
    fS       = (i == 128 ? flg : false);
    fZ       = (flg ^ true);
    fPV      = (flg ^ true);
}
int z80::res(int i, int i_71_)
{
    return i_71_ & (i ^ 0xffffffff);
}
int z80::set(int i, int i_86_)
{
    return i_86_ | i;
}
void z80::setIFF1(bool flg)
{
    _IFF1 = flg;
}
void z80::setIFF2(bool flg)
{
    _IFF2 = flg;
}
void z80::setIDH(int i)
{
    _ID = i << 8 & 0xff00 | _ID & 0xff;
}
void z80::setIDL(int i)
{
    _ID = _ID & 0xff00 | i & 0xff;
}
int z80::ID_d()
{
    return _ID + bytef(readMem(_PC++)) & 0xffff;
}
int z80::nxtpcb()
{
    return readMem(_PC++);
}
int z80::in_bc()
{
    int i = inb(_C);
    fZ    = (i == 0);
    fS    = ((i & 0x80) != 0);
    f3    = ((i & 0x8) != 0);
    f5    = ((i & 0x20) != 0);
    fPV   = (parity[i]);
    fN    = (false);
    fH    = (false);
    return i;
}
int z80::sbc16(int i, int i_78_)
{
    int i_79_ = fC ? 1 : 0;
    int i_80_ = i - i_78_ - i_79_;
    int i_81_ = i_80_ & 0xffff;
    fS        = ((i_81_ & 0x8000) != 0);
    f3        = ((i_81_ & 0x800) != 0);
    f5        = ((i_81_ & 0x2000) != 0);
    fZ        = (i_81_ == 0);
    fC        = ((i_80_ & 0x10000) != 0);
    fPV       = (((i ^ i_78_) & (i ^ i_81_) & 0x8000) != 0);
    fH        = (((i & 0xfff) - (i_78_ & 0xfff) - i_79_ & 0x1000) != 0);
    fN        = (true);
    return i_81_;
}
int z80::adc16(int i, int i_1_)
{
    int i_2_ = fC ? 1 : 0;
    int i_3_ = i + i_1_ + i_2_;
    int i_4_ = i_3_ & 0xffff;
    fS       = ((i_4_ & 0x8000) != 0);
    f3       = ((i_4_ & 0x800) != 0);
    f5       = ((i_4_ & 0x2000) != 0);
    fZ       = (i_4_ == 0);
    fC       = ((i_3_ & 0x10000) != 0);
    fPV      = (((i ^ (i_1_ ^ 0xffffffff)) & (i ^ i_4_) & 0x8000) != 0);
    fH       = (((i & 0xfff) + (i_1_ & 0xfff) + i_2_ & 0x1000) != 0);
    fN       = (false);
    return i_4_;
}
int z80::z80_interrupt()
{
    if (!_IFF1)
        return 0;
    switch (_IM) {
        case 0:
        case 1:
            pushpc();
            setIFF1(false);
            setIFF2(false);
            _PC = (56);
            return 13;
        case 2: {
            pushpc();
            setIFF1(false);
            setIFF2(false);
            _PC = (readMemWord(_I << 8 | 0xff));
            return 19;
        }
        default:
            return 0;
    }
}
int z80::inb(int i)
{
    switch (i) {
        case 162:
            return msx->psg->lePortaDados();
        case 168:
            return PPIPortA;
        case 169:
            return msx->keyboard->state[PPIPortC & 0xf];
        case 170:
            return PPIPortC;
        case 171:
            return PPIPortD;
        case 152: {
            int val = msx->vdp->lePortaDados();
            return val;
        }
        case 153: {
            int val = msx->vdp->lePortaComandos();
            return val;
        }
        default:
            if (portos[i] != -1)
                return portos[i];
            return 255;
    }
    return 255;
}
void z80::outb(int i, int i_19_)
{
    switch (i) {
        case 142:
            megarom = true;
            break;
        case 160:
            msx->psg->escrevePortaEndereco(i_19_);
            break;
        case 161:
            msx->psg->escrevePortaDados(i_19_);
            break;
        case 168:
            PPIPortA = i_19_;
            break;
        case 169:
            // PPIPortB = i_19_;
            break;
        case 170:
            PPIPortC = i_19_;
            break;
        case 171:
            PPIPortD = i_19_;
            break;
        case 152:
            msx->vdp->escrevePortaDados(i_19_);
            break;
        case 153:
            msx->vdp->escrevePortaComandos(i_19_);
            break;
        default:
            portos[i] = i_19_;
    }
}
