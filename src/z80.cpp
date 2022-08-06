#include "z80.h"
#include "PC.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include "mem.h"

z80::z80(PC *_pc, Mem *_mem)
{
    msx = _pc;
    mem = _mem;

    reset();
    file_read();
}
z80::~z80()
{
}
void z80::reset()
{
    for (int i = 0; i < 256; i++) {
        int flg = 1;
        for (int i_0_ = 0; i_0_ < 8; i_0_++) {
            if ((i & 1 << i_0_) != 0)
                flg ^= 1;
        }
        parity[i] = flg;
    }

    rPC = 0;
    rSP = 65520;
    rA  = 0;
    setF(0);
    setBC(0);
    setDE(0);
    setHL(0);

    int i = HL();
    setHL(rHL);
    rHL = i;
    i   = DE();
    setDE(rDE);
    rDE = i;
    i   = BC();
    setBC(rBC);
    rBC = i;

    int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
             (fN ? 2 : 0) | (fC ? 1 : 0));
    i     = rA << 8 | f;
    rA    = (rAF >> 8);
    setF(rAF & 0xff);
    rAF = i;
    rA  = 0;

    setF(0);
    setBC(0);
    setDE(0);
    setHL(0);

    rIX = (0);
    rIY = (0);
    rR  = i;
    rR7 = i & 0x80;

    rI = (0);
    setIFF1(false);
    setIFF2(false);
    rIM = (0);

    for (i = 0; i < 256; i++)
        portos[i] = -1;
}
int z80::file_read()
{
    logcheck      = false;
    stepinfo      = false;
    filename      = "logs/log0.txt";
    filecheck_end = 10000;

    if (logcheck) {
        std::string   line;
        std::ifstream input_file(filename);
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
        sprintf(buf2, "A=%02X B=%02X C=%02X D=%02X", rA, rB, rC, rD);
        // printf("%s\n", buf2);

        char buf3[1000];
        sprintf(buf3, "E=%02X H=%02X L=%02X", rE, rH, rL);
        // printf("%s\n", buf3);

        char buf4[1000];
        sprintf(buf4, "fS=%d fZ=%d f5=%d fH=%d f3=%d fPV=%d fN=%d fC=%d", fS, fZ, f5, fH, f3, fPV, fN, fC);
        // printf("%s\n", buf4);

        char buf5[1000];
        sprintf(buf5, "AF=%04X HL=%04X BC=%04X DE=%04X", rAF, rHL, rBC, rDE);
        // printf("%s\n", buf5);

        char buf6[1000];
        sprintf(buf6, "IX=%04X IY=%04X ID=%04X", rIX, rIY, rID);
        // printf("%s\n", buf6);

        char buf7[1000];
        sprintf(buf7, "SP=%04X PC=%04X", rSP, rPC);
        // printf("%s\n", buf7);

        char buf8[1000];
        sprintf(buf8, "I=%04X R=%08X R7=%04X IM=%04X", rI, rR, rR7, rIM);
        // printf("%s\n", buf8);

        char buf9[1000];
        sprintf(buf9, "IFF1=%d IFF2=%d", IFF1, IFF2);
        // printf("%s\n", buf9);

        char buf10[1000];
        sprintf(buf10, "PPA=%04X PPC=%04X PPD=%04X", mem->PPIPortA, mem->PPIPortC, mem->PPIPortD);
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
    int ival   = -(T_STATES_PERrINTERRUPT - z80_interrupt());
    int i      = 0;
    int i_2_   = 0;
    int i_3_   = 0;
    int opcode = 0;

    while ((ival + i) < 0) {
        rR += 1;
        opcode = mem->readMem(rPC++);
        dump(opcode, i);

        switch (opcode) {
            case 0:
                i += 4;
                break;
            case 8: {
                int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
                         (fN ? 2 : 0) | (fC ? 1 : 0));
                int ii = rA << 8 | f;
                rA     = (rAF >> 8);
                setF(rAF & 0xff);
                rAF = ii;
                i += 4;
                break;
            }
            case 16: {
                rB = (i_3_ = rB - 1 & 0xff);

                if (i_3_ != 0) {
                    i_2_ = bytef(mem->readMem(rPC++));
                    rPC  = (rPC + i_2_ & 0xffff);
                    i += 13;
                } else {
                    rPC = rPC + 1 & 0xffff;
                    i += 8;
                }
                break;
            }
            case 24: {
                i_2_ = bytef(mem->readMem(rPC++));
                rPC  = (rPC + i_2_ & 0xffff);
                i += 12;
                break;
            }
            case 32:
                if (!fZ) {
                    i_2_ = bytef(mem->readMem(rPC++));
                    rPC  = (rPC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    rPC = rPC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 40:
                if (fZ) {
                    i_2_ = bytef(mem->readMem(rPC++));
                    rPC  = (rPC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    rPC = rPC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 48:
                if (!fC) {
                    i_2_ = bytef(mem->readMem(rPC++));
                    rPC  = (rPC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    rPC = rPC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 56:
                if (fC) {
                    i_2_ = bytef(mem->readMem(rPC++));
                    rPC  = (rPC + i_2_ & 0xffff);
                    i += 12;
                } else {
                    rPC = rPC + 1 & 0xffff;
                    i += 7;
                }
                break;
            case 1:
                setBC(mem->readMemWord((rPC = rPC + 2) - 2));
                i += 10;
                break;
            case 9:
                setHL(add16(HL(), BC()));
                i += 11;
                break;
            case 17:
                setDE(mem->readMemWord((rPC = rPC + 2) - 2));
                i += 10;
                break;
            case 25:
                setHL(add16(HL(), DE()));
                i += 11;
                break;
            case 33:
                setHL(mem->readMemWord((rPC = rPC + 2) - 2));
                i += 10;
                break;
            case 41: {
                i_2_ = HL();
                setHL(add16(i_2_, i_2_));
                i += 11;
                break;
            }
            case 49:
                rSP = (mem->readMemWord((rPC = rPC + 2) - 2));
                i += 10;
                break;
            case 57:
                setHL(add16(HL(), rSP));
                i += 11;
                break;
            case 2:
                mem->writeMem(BC(), rA);
                i += 7;
                break;
            case 10:
                rA = (mem->readMem(BC()));
                i += 7;
                break;
            case 18:
                mem->writeMem(DE(), rA);
                i += 7;
                break;
            case 26:
                rA = (mem->readMem(DE()));
                i += 7;
                break;
            case 34:
                mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), HL());
                i += 16;
                break;
            case 42:
                setHL(mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2)));
                i += 16;
                break;
            case 50:
                mem->writeMem(mem->readMemWord((rPC = rPC + 2) - 2), rA);
                i += 13;
                break;
            case 58:
                rA = (mem->readMem(mem->readMemWord((rPC = rPC + 2) - 2)));
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
                rSP = rSP + 1 & 0xffff;
                i += 6;
                break;
            case 59:
                rSP = rSP - 1 & 0xffff;
                i += 6;
                break;
            case 4:
                rB = (inc8(rB));
                i += 4;
                break;
            case 12:
                rC = (inc8(rC));
                i += 4;
                break;
            case 20:
                rD = (inc8(rD));
                i += 4;
                break;
            case 28:
                rE = (inc8(rE));
                i += 4;
                break;
            case 36:
                rH = (inc8(rH));
                i += 4;
                break;
            case 44:
                rL = (inc8(rL));
                i += 4;
                break;
            case 52: {
                i_2_ = HL();
                mem->writeMem(i_2_, inc8(mem->readMem(i_2_)));
                i += 11;
                break;
            }
            case 60:
                rA = (inc8(rA));
                i += 4;
                break;
            case 5:
                rB = (dec8(rB));
                i += 4;
                break;
            case 13:
                rC = (dec8(rC));
                i += 4;
                break;
            case 21:
                rD = (dec8(rD));
                i += 4;
                break;
            case 29:
                rE = (dec8(rE));
                i += 4;
                break;
            case 37:
                rH = (dec8(rH));
                i += 4;
                break;
            case 45:
                rL = (dec8(rL));
                i += 4;
                break;
            case 53: {
                i_2_ = HL();
                mem->writeMem(i_2_, dec8(mem->readMem(i_2_)));
                i += 11;
                break;
            }
            case 61:
                rA = (dec8(rA));
                i += 4;
                break;
            case 6:
                rB = (mem->readMem(rPC++));
                i += 7;
                break;
            case 14:
                rC = (mem->readMem(rPC++));
                i += 7;
                break;
            case 22:
                rD = (mem->readMem(rPC++));
                i += 7;
                break;
            case 30:
                rE = (mem->readMem(rPC++));
                i += 7;
                break;
            case 38:
                rH = (mem->readMem(rPC++));
                i += 7;
                break;
            case 46:
                rL = (mem->readMem(rPC++));
                i += 7;
                break;
            case 54:
                mem->writeMem(HL(), mem->readMem(rPC++));
                i += 10;
                break;
            case 62:
                rA = (mem->readMem(rPC++));
                i += 7;
                break;
            case 7: {
                int  ii  = rA;
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
                rA = (ii);
                i += 4;
            } break;
            case 15: {
                int  ii  = rA;
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
                rA = (ii);
                i += 4;
            } break;
            case 23: {
                int  ii  = rA;
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
                rA = (ii);
                i += 4;
            } break;
            case 31: {
                int  ii  = rA;
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
                rA = (ii);
                i += 4;
            } break;
            case 39: {
                int  ii    = rA;
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
                ii  = rA;
                fC  = (flg);
                fPV = (parity[ii]);
                i += 4;
            } break;
            case 47: {
                int ii = rA ^ 0xff;
                f3     = ((rA & 0x8) != 0);
                f5     = ((rA & 0x20) != 0);
                fH     = (true);
                fN     = (true);
                rA     = (ii);
                i += 4;
            } break;
            case 55: {
                int ii = rA;
                f3     = ((ii & 0x8) != 0);
                f5     = ((ii & 0x20) != 0);
                fN     = (false);
                fH     = (false);
                fC     = (true);
                i += 4;
            } break;
            case 63: {
                int ii = rA;
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
                rB = (rC);
                i += 4;
                break;
            case 66:
                rB = (rD);
                i += 4;
                break;
            case 67:
                rB = (rE);
                i += 4;
                break;
            case 68:
                rB = (rH);
                i += 4;
                break;
            case 69:
                rB = (rL);
                i += 4;
                break;
            case 70:
                rB = (mem->readMem(HL()));
                i += 7;
                break;
            case 71:
                rB = (rA);
                i += 4;
                break;
            case 72:
                rC = (rB);
                i += 4;
                break;
            case 73:
                i += 4;
                break;
            case 74:
                rC = (rD);
                i += 4;
                break;
            case 75:
                rC = (rE);
                i += 4;
                break;
            case 76:
                rC = (rH);
                i += 4;
                break;
            case 77:
                rC = (rL);
                i += 4;
                break;
            case 78:
                rC = (mem->readMem(HL()));
                i += 7;
                break;
            case 79:
                rC = (rA);
                i += 4;
                break;
            case 80:
                rD = (rB);
                i += 4;
                break;
            case 81:
                rD = (rC);
                i += 4;
                break;
            case 82:
                i += 4;
                break;
            case 83:
                rD = (rE);
                i += 4;
                break;
            case 84:
                rD = (rH);
                i += 4;
                break;
            case 85:
                rD = (rL);
                i += 4;
                break;
            case 86:
                rD = (mem->readMem(HL()));
                i += 7;
                break;
            case 87:
                rD = (rA);
                i += 4;
                break;
            case 88:
                rE = (rB);
                i += 4;
                break;
            case 89:
                rE = (rC);
                i += 4;
                break;
            case 90:
                rE = (rD);
                i += 4;
                break;
            case 91:
                i += 4;
                break;
            case 92:
                rE = (rH);
                i += 4;
                break;
            case 93:
                rE = (rL);
                i += 4;
                break;
            case 94:
                rE = (mem->readMem(HL()));
                i += 7;
                break;
            case 95:
                rE = (rA);
                i += 4;
                break;
            case 96:
                rH = (rB);
                i += 4;
                break;
            case 97:
                rH = (rC);
                i += 4;
                break;
            case 98:
                rH = (rD);
                i += 4;
                break;
            case 99:
                rH = (rE);
                i += 4;
                break;
            case 100:
                i += 4;
                break;
            case 101:
                rH = (rL);
                i += 4;
                break;
            case 102:
                rH = (mem->readMem(HL()));
                i += 7;
                break;
            case 103:
                rH = (rA);
                i += 4;
                break;
            case 104:
                rL = (rB);
                i += 4;
                break;
            case 105:
                rL = (rC);
                i += 4;
                break;
            case 106:
                rL = (rD);
                i += 4;
                break;
            case 107:
                rL = (rE);
                i += 4;
                break;
            case 108:
                rL = (rH);
                i += 4;
                break;
            case 109:
                i += 4;
                break;
            case 110:
                rL = (mem->readMem(HL()));
                i += 7;
                break;
            case 111:
                rL = (rA);
                i += 4;
                break;
            case 112:
                mem->writeMem(HL(), rB);
                i += 7;
                break;
            case 113:
                mem->writeMem(HL(), rC);
                i += 7;
                break;
            case 114:
                mem->writeMem(HL(), rD);
                i += 7;
                break;
            case 115:
                mem->writeMem(HL(), rE);
                i += 7;
                break;
            case 116:
                mem->writeMem(HL(), rH);
                i += 7;
                break;
            case 117:
                mem->writeMem(HL(), rL);
                i += 7;
                break;
            case 118: {
                i_2_ = (-i - 1) / 4 + 1;
                i += i_2_ * 4;
                rR += (i_2_ - 1);
                rR &= 0xffff;
                break;
            }
            case 119:
                mem->writeMem(HL(), rA);
                i += 7;
                break;
            case 120:
                rA = (rB);
                i += 4;
                break;
            case 121:
                rA = (rC);
                i += 4;
                break;
            case 122:
                rA = (rD);
                i += 4;
                break;
            case 123:
                rA = (rE);
                i += 4;
                break;
            case 124:
                rA = (rH);
                i += 4;
                break;
            case 125:
                rA = (rL);
                i += 4;
                break;
            case 126:
                rA = (mem->readMem(HL()));
                i += 7;
                break;
            case 127:
                i += 4;
                break;
            case 128:
                add_a(rB);
                i += 4;
                break;
            case 129:
                add_a(rC);
                i += 4;
                break;
            case 130:
                add_a(rD);
                i += 4;
                break;
            case 131:
                add_a(rE);
                i += 4;
                break;
            case 132:
                add_a(rH);
                i += 4;
                break;
            case 133:
                add_a(rL);
                i += 4;
                break;
            case 134:
                add_a(mem->readMem(HL()));
                i += 7;
                break;
            case 135:
                add_a(rA);
                i += 4;
                break;
            case 136:
                adc_a(rB);
                i += 4;
                break;
            case 137:
                adc_a(rC);
                i += 4;
                break;
            case 138:
                adc_a(rD);
                i += 4;
                break;
            case 139:
                adc_a(rE);
                i += 4;
                break;
            case 140:
                adc_a(rH);
                i += 4;
                break;
            case 141:
                adc_a(rL);
                i += 4;
                break;
            case 142:
                adc_a(mem->readMem(HL()));
                i += 7;
                break;
            case 143:
                adc_a(rA);
                i += 4;
                break;
            case 144:
                sub_a(rB);
                i += 4;
                break;
            case 145:
                sub_a(rC);
                i += 4;
                break;
            case 146:
                sub_a(rD);
                i += 4;
                break;
            case 147:
                sub_a(rE);
                i += 4;
                break;
            case 148:
                sub_a(rH);
                i += 4;
                break;
            case 149:
                sub_a(rL);
                i += 4;
                break;
            case 150:
                sub_a(mem->readMem(HL()));
                i += 7;
                break;
            case 151:
                sub_a(rA);
                i += 4;
                break;
            case 152:
                sbc_a(rB);
                i += 4;
                break;
            case 153:
                sbc_a(rC);
                i += 4;
                break;
            case 154:
                sbc_a(rD);
                i += 4;
                break;
            case 155:
                sbc_a(rE);
                i += 4;
                break;
            case 156:
                sbc_a(rH);
                i += 4;
                break;
            case 157:
                sbc_a(rL);
                i += 4;
                break;
            case 158:
                sbc_a(mem->readMem(HL()));
                i += 7;
                break;
            case 159:
                sbc_a(rA);
                i += 4;
                break;
            case 160:
                and_a(rB);
                i += 4;
                break;
            case 161:
                and_a(rC);
                i += 4;
                break;
            case 162:
                and_a(rD);
                i += 4;
                break;
            case 163:
                and_a(rE);
                i += 4;
                break;
            case 164:
                and_a(rH);
                i += 4;
                break;
            case 165:
                and_a(rL);
                i += 4;
                break;
            case 166:
                and_a(mem->readMem(HL()));
                i += 7;
                break;
            case 167:
                and_a(rA);
                i += 4;
                break;
            case 168:
                xor_a(rB);
                i += 4;
                break;
            case 169:
                xor_a(rC);
                i += 4;
                break;
            case 170:
                xor_a(rD);
                i += 4;
                break;
            case 171:
                xor_a(rE);
                i += 4;
                break;
            case 172:
                xor_a(rH);
                i += 4;
                break;
            case 173:
                xor_a(rL);
                i += 4;
                break;
            case 174:
                xor_a(mem->readMem(HL()));
                i += 7;
                break;
            case 175:
                xor_a(rA);
                i += 4;
                break;
            case 176:
                or_a(rB);
                i += 4;
                break;
            case 177:
                or_a(rC);
                i += 4;
                break;
            case 178:
                or_a(rD);
                i += 4;
                break;
            case 179:
                or_a(rE);
                i += 4;
                break;
            case 180:
                or_a(rH);
                i += 4;
                break;
            case 181:
                or_a(rL);
                i += 4;
                break;
            case 182:
                or_a(mem->readMem(HL()));
                i += 7;
                break;
            case 183:
                or_a(rA);
                i += 4;
                break;
            case 184:
                cp_a(rB);
                i += 4;
                break;
            case 185:
                cp_a(rC);
                i += 4;
                break;
            case 186:
                cp_a(rD);
                i += 4;
                break;
            case 187:
                cp_a(rE);
                i += 4;
                break;
            case 188:
                cp_a(rH);
                i += 4;
                break;
            case 189:
                cp_a(rL);
                i += 4;
                break;
            case 190:
                cp_a(mem->readMem(HL()));
                i += 7;
                break;
            case 191:
                cp_a(rA);
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
                setHL(rHL);
                rHL = ii;
                ii  = DE();
                setDE(rDE);
                rDE = ii;
                ii  = BC();
                setBC(rBC);
                rBC = ii;
                i += 4;
            } break;
            case 225:
                setHL(popw());
                i += 10;
                break;
            case 233:
                rPC = (HL());
                i += 4;
                break;
            case 241: {
                int ii = popw();
                rA     = (ii >> 8);
                setF(ii & 0xff);
                i += 10;
            } break;
            case 249:
                rSP = (HL());
                i += 6;
                break;
            case 194:
                if (!fZ)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 202:
                if (fZ)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 210:
                if (!fC)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 218:
                if (fC)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 226:
                if (!fPV)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 234:
                if (fPV)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 242:
                if (!fS)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 250:
                if (fS)
                    rPC = (mem->readMemWord((rPC = rPC + 2) - 2));
                else
                    rPC = (rPC + 2 & 0xffff);
                i += 10;
                break;
            case 195:
                rPC = (mem->readMemWord(rPC));
                i += 10;
                break;
            case 203: {
                int ii = HL();
                rR += (1);
                switch (mem->readMem(rPC++)) {
                    case 0:
                        rB = (rlc(rB));
                        i += 8;
                        goto EXECrCB;
                    case 1:
                        rC = (rlc(rC));
                        i += 8;
                        goto EXECrCB;
                    case 2:
                        rD = (rlc(rD));
                        i += 8;
                        goto EXECrCB;
                    case 3:
                        rE = (rlc(rE));
                        i += 8;
                        goto EXECrCB;
                    case 4:
                        rH = (rlc(rH));
                        i += 8;
                        goto EXECrCB;
                    case 5:
                        rL = (rlc(rL));
                        i += 8;
                        goto EXECrCB;
                    case 6: {
                        // ii = HL();
                        mem->writeMem(ii, rlc(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 7:
                        rA = (rlc(rA));
                        i += 8;
                        goto EXECrCB;
                    case 8:
                        rB = (rrc(rB));
                        i += 8;
                        goto EXECrCB;
                    case 9:
                        rC = (rrc(rC));
                        i += 8;
                        goto EXECrCB;
                    case 10:
                        rD = (rrc(rD));
                        i += 8;
                        goto EXECrCB;
                    case 11:
                        rE = (rrc(rE));
                        i += 8;
                        goto EXECrCB;
                    case 12:
                        rH = (rrc(rH));
                        i += 8;
                        goto EXECrCB;
                    case 13:
                        rL = (rrc(rL));
                        i += 8;
                        goto EXECrCB;
                    case 14: {
                        // ii = HL();
                        mem->writeMem(ii, rrc(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 15:
                        rA = (rrc(rA));
                        i += 8;
                        goto EXECrCB;
                    case 16:
                        rB = (rl(rB));
                        i += 8;
                        goto EXECrCB;
                    case 17:
                        rC = (rl(rC));
                        i += 8;
                        goto EXECrCB;
                    case 18:
                        rD = (rl(rD));
                        i += 8;
                        goto EXECrCB;
                    case 19:
                        rE = (rl(rE));
                        i += 8;
                        goto EXECrCB;
                    case 20:
                        rH = (rl(rH));
                        i += 8;
                        goto EXECrCB;
                    case 21:
                        rL = (rl(rL));
                        i += 8;
                        goto EXECrCB;
                    case 22: {
                        // ii = HL();
                        mem->writeMem(ii, rl(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 23:
                        rA = (rl(rA));
                        i += 8;
                        goto EXECrCB;
                    case 24:
                        rB = (rr(rB));
                        i += 8;
                        goto EXECrCB;
                    case 25:
                        rC = (rr(rC));
                        i += 8;
                        goto EXECrCB;
                    case 26:
                        rD = (rr(rD));
                        i += 8;
                        goto EXECrCB;
                    case 27:
                        rE = (rr(rE));
                        i += 8;
                        goto EXECrCB;
                    case 28:
                        rH = (rr(rH));
                        i += 8;
                        goto EXECrCB;
                    case 29:
                        rL = (rr(rL));
                        i += 8;
                        goto EXECrCB;
                    case 30: {
                        // ii = HL();
                        mem->writeMem(ii, rr(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 31:
                        rA = (rr(rA));
                        i += 8;
                        goto EXECrCB;
                    case 32:
                        rB = (sla(rB));
                        i += 8;
                        goto EXECrCB;
                    case 33:
                        rC = (sla(rC));
                        i += 8;
                        goto EXECrCB;
                    case 34:
                        rD = (sla(rD));
                        i += 8;
                        goto EXECrCB;
                    case 35:
                        rE = (sla(rE));
                        i += 8;
                        goto EXECrCB;
                    case 36:
                        rH = (sla(rH));
                        i += 8;
                        goto EXECrCB;
                    case 37:
                        rL = (sla(rL));
                        i += 8;
                        goto EXECrCB;
                    case 38: {
                        // ii = HL();
                        mem->writeMem(ii, sla(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 39:
                        rA = (sla(rA));
                        i += 8;
                        goto EXECrCB;
                    case 40:
                        rB = (sra(rB));
                        i += 8;
                        goto EXECrCB;
                    case 41:
                        rC = (sra(rC));
                        i += 8;
                        goto EXECrCB;
                    case 42:
                        rD = (sra(rD));
                        i += 8;
                        goto EXECrCB;
                    case 43:
                        rE = (sra(rE));
                        i += 8;
                        goto EXECrCB;
                    case 44:
                        rH = (sra(rH));
                        i += 8;
                        goto EXECrCB;
                    case 45:
                        rL = (sra(rL));
                        i += 8;
                        goto EXECrCB;
                    case 46: {
                        // ii = HL();
                        mem->writeMem(ii, sra(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 47:
                        rA = (sra(rA));
                        i += 8;
                        goto EXECrCB;
                    case 48:
                        rB = (sls(rB));
                        i += 8;
                        goto EXECrCB;
                    case 49:
                        rC = (sls(rC));
                        i += 8;
                        goto EXECrCB;
                    case 50:
                        rD = (sls(rD));
                        i += 8;
                        goto EXECrCB;
                    case 51:
                        rE = (sls(rE));
                        i += 8;
                        goto EXECrCB;
                    case 52:
                        rH = (sls(rH));
                        i += 8;
                        goto EXECrCB;
                    case 53:
                        rL = (sls(rL));
                        i += 8;
                        goto EXECrCB;
                    case 54: {
                        // ii = HL();
                        mem->writeMem(ii, sls(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 55:
                        rA = (sls(rA));
                        i += 8;
                        goto EXECrCB;
                    case 56:
                        rB = (srl(rB));
                        i += 8;
                        goto EXECrCB;
                    case 57:
                        rC = (srl(rC));
                        i += 8;
                        goto EXECrCB;
                    case 58:
                        rD = (srl(rD));
                        i += 8;
                        goto EXECrCB;
                    case 59:
                        rE = (srl(rE));
                        i += 8;
                        goto EXECrCB;
                    case 60:
                        rH = (srl(rH));
                        i += 8;
                        goto EXECrCB;
                    case 61:
                        rL = (srl(rL));
                        i += 8;
                        goto EXECrCB;
                    case 62: {
                        // ii = HL();
                        mem->writeMem(ii, srl(mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 63:
                        rA = (srl(rA));
                        i += 8;
                        goto EXECrCB;
                    case 64:
                        bit(1, rB);
                        i += 8;
                        goto EXECrCB;
                    case 65:
                        bit(1, rC);
                        i += 8;
                        goto EXECrCB;
                    case 66:
                        bit(1, rD);
                        i += 8;
                        goto EXECrCB;
                    case 67:
                        bit(1, rE);
                        i += 8;
                        goto EXECrCB;
                    case 68:
                        bit(1, rH);
                        i += 8;
                        goto EXECrCB;
                    case 69:
                        bit(1, rL);
                        i += 8;
                        goto EXECrCB;
                    case 70:
                        bit(1, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 71:
                        bit(1, rA);
                        i += 8;
                        goto EXECrCB;
                    case 72:
                        bit(2, rB);
                        i += 8;
                        goto EXECrCB;
                    case 73:
                        bit(2, rC);
                        i += 8;
                        goto EXECrCB;
                    case 74:
                        bit(2, rD);
                        i += 8;
                        goto EXECrCB;
                    case 75:
                        bit(2, rE);
                        i += 8;
                        goto EXECrCB;
                    case 76:
                        bit(2, rH);
                        i += 8;
                        goto EXECrCB;
                    case 77:
                        bit(2, rL);
                        i += 8;
                        goto EXECrCB;
                    case 78:
                        bit(2, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 79:
                        bit(2, rA);
                        i += 8;
                        goto EXECrCB;
                    case 80:
                        bit(4, rB);
                        i += 8;
                        goto EXECrCB;
                    case 81:
                        bit(4, rC);
                        i += 8;
                        goto EXECrCB;
                    case 82:
                        bit(4, rD);
                        i += 8;
                        goto EXECrCB;
                    case 83:
                        bit(4, rE);
                        i += 8;
                        goto EXECrCB;
                    case 84:
                        bit(4, rH);
                        i += 8;
                        goto EXECrCB;
                    case 85:
                        bit(4, rL);
                        i += 8;
                        goto EXECrCB;
                    case 86:
                        bit(4, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 87:
                        bit(4, rA);
                        i += 8;
                        goto EXECrCB;
                    case 88:
                        bit(8, rB);
                        i += 8;
                        goto EXECrCB;
                    case 89:
                        bit(8, rC);
                        i += 8;
                        goto EXECrCB;
                    case 90:
                        bit(8, rD);
                        i += 8;
                        goto EXECrCB;
                    case 91:
                        bit(8, rE);
                        i += 8;
                        goto EXECrCB;
                    case 92:
                        bit(8, rH);
                        i += 8;
                        goto EXECrCB;
                    case 93:
                        bit(8, rL);
                        i += 8;
                        goto EXECrCB;
                    case 94:
                        bit(8, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 95:
                        bit(8, rA);
                        i += 8;
                        goto EXECrCB;
                    case 96:
                        bit(16, rB);
                        i += 8;
                        goto EXECrCB;
                    case 97:
                        bit(16, rC);
                        i += 8;
                        goto EXECrCB;
                    case 98:
                        bit(16, rD);
                        i += 8;
                        goto EXECrCB;
                    case 99:
                        bit(16, rE);
                        i += 8;
                        goto EXECrCB;
                    case 100:
                        bit(16, rH);
                        i += 8;
                        goto EXECrCB;
                    case 101:
                        bit(16, rL);
                        i += 8;
                        goto EXECrCB;
                    case 102:
                        bit(16, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 103:
                        bit(16, rA);
                        i += 8;
                        goto EXECrCB;
                    case 104:
                        bit(32, rB);
                        i += 8;
                        goto EXECrCB;
                    case 105:
                        bit(32, rC);
                        i += 8;
                        goto EXECrCB;
                    case 106:
                        bit(32, rD);
                        i += 8;
                        goto EXECrCB;
                    case 107:
                        bit(32, rE);
                        i += 8;
                        goto EXECrCB;
                    case 108:
                        bit(32, rH);
                        i += 8;
                        goto EXECrCB;
                    case 109:
                        bit(32, rL);
                        i += 8;
                        goto EXECrCB;
                    case 110:
                        bit(32, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 111:
                        bit(32, rA);
                        i += 8;
                        goto EXECrCB;
                    case 112:
                        bit(64, rB);
                        i += 8;
                        goto EXECrCB;
                    case 113:
                        bit(64, rC);
                        i += 8;
                        goto EXECrCB;
                    case 114:
                        bit(64, rD);
                        i += 8;
                        goto EXECrCB;
                    case 115:
                        bit(64, rE);
                        i += 8;
                        goto EXECrCB;
                    case 116:
                        bit(64, rH);
                        i += 8;
                        goto EXECrCB;
                    case 117:
                        bit(64, rL);
                        i += 8;
                        goto EXECrCB;
                    case 118:
                        bit(64, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 119:
                        bit(64, rA);
                        i += 8;
                        goto EXECrCB;
                    case 120:
                        bit(128, rB);
                        i += 8;
                        goto EXECrCB;
                    case 121:
                        bit(128, rC);
                        i += 8;
                        goto EXECrCB;
                    case 122:
                        bit(128, rD);
                        i += 8;
                        goto EXECrCB;
                    case 123:
                        bit(128, rE);
                        i += 8;
                        goto EXECrCB;
                    case 124:
                        bit(128, rH);
                        i += 8;
                        goto EXECrCB;
                    case 125:
                        bit(128, rL);
                        i += 8;
                        goto EXECrCB;
                    case 126:
                        bit(128, mem->readMem(HL()));
                        i += 12;
                        goto EXECrCB;
                    case 127:
                        bit(128, rA);
                        i += 8;
                        goto EXECrCB;
                    case 128:
                        rB = (res(1, rB));
                        i += 8;
                        goto EXECrCB;
                    case 129:
                        rC = (res(1, rC));
                        i += 8;
                        goto EXECrCB;
                    case 130:
                        rD = (res(1, rD));
                        i += 8;
                        goto EXECrCB;
                    case 131:
                        rE = (res(1, rE));
                        i += 8;
                        goto EXECrCB;
                    case 132:
                        rH = (res(1, rH));
                        i += 8;
                        goto EXECrCB;
                    case 133:
                        rL = (res(1, rL));
                        i += 8;
                        goto EXECrCB;
                    case 134: {
                        // ii = HL();
                        mem->writeMem(ii, res(1, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 135:
                        rA = (res(1, rA));
                        i += 8;
                        goto EXECrCB;
                    case 136:
                        rB = (res(2, rB));
                        i += 8;
                        goto EXECrCB;
                    case 137:
                        rC = (res(2, rC));
                        i += 8;
                        goto EXECrCB;
                    case 138:
                        rD = (res(2, rD));
                        i += 8;
                        goto EXECrCB;
                    case 139:
                        rE = (res(2, rE));
                        i += 8;
                        goto EXECrCB;
                    case 140:
                        rH = (res(2, rH));
                        i += 8;
                        goto EXECrCB;
                    case 141:
                        rL = (res(2, rL));
                        i += 8;
                        goto EXECrCB;
                    case 142: {
                        // ii = HL();
                        mem->writeMem(ii, res(2, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 143:
                        rA = (res(2, rA));
                        i += 8;
                        goto EXECrCB;
                    case 144:
                        rB = (res(4, rB));
                        i += 8;
                        goto EXECrCB;
                    case 145:
                        rC = (res(4, rC));
                        i += 8;
                        goto EXECrCB;
                    case 146:
                        rD = (res(4, rD));
                        i += 8;
                        goto EXECrCB;
                    case 147:
                        rE = (res(4, rE));
                        i += 8;
                        goto EXECrCB;
                    case 148:
                        rH = (res(4, rH));
                        i += 8;
                        goto EXECrCB;
                    case 149:
                        rL = (res(4, rL));
                        i += 8;
                        goto EXECrCB;
                    case 150: {
                        // ii = HL();
                        mem->writeMem(ii, res(4, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 151:
                        rA = (res(4, rA));
                        i += 8;
                        goto EXECrCB;
                    case 152:
                        rB = (res(8, rB));
                        i += 8;
                        goto EXECrCB;
                    case 153:
                        rC = (res(8, rC));
                        i += 8;
                        goto EXECrCB;
                    case 154:
                        rD = (res(8, rD));
                        i += 8;
                        goto EXECrCB;
                    case 155:
                        rE = (res(8, rE));
                        i += 8;
                        goto EXECrCB;
                    case 156:
                        rH = (res(8, rH));
                        i += 8;
                        goto EXECrCB;
                    case 157:
                        rL = (res(8, rL));
                        i += 8;
                        goto EXECrCB;
                    case 158: {
                        // ii = HL();
                        mem->writeMem(ii, res(8, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 159:
                        rA = (res(8, rA));
                        i += 8;
                        goto EXECrCB;
                    case 160:
                        rB = (res(16, rB));
                        i += 8;
                        goto EXECrCB;
                    case 161:
                        rC = (res(16, rC));
                        i += 8;
                        goto EXECrCB;
                    case 162:
                        rD = (res(16, rD));
                        i += 8;
                        goto EXECrCB;
                    case 163:
                        rE = (res(16, rE));
                        i += 8;
                        goto EXECrCB;
                    case 164:
                        rH = (res(16, rH));
                        i += 8;
                        goto EXECrCB;
                    case 165:
                        rL = (res(16, rL));
                        i += 8;
                        goto EXECrCB;
                    case 166: {
                        // ii = HL();
                        mem->writeMem(ii, res(16, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 167:
                        rA = (res(16, rA));
                        i += 8;
                        goto EXECrCB;
                    case 168:
                        rB = (res(32, rB));
                        i += 8;
                        goto EXECrCB;
                    case 169:
                        rC = (res(32, rC));
                        i += 8;
                        goto EXECrCB;
                    case 170:
                        rD = (res(32, rD));
                        i += 8;
                        goto EXECrCB;
                    case 171:
                        rE = (res(32, rE));
                        i += 8;
                        goto EXECrCB;
                    case 172:
                        rH = (res(32, rH));
                        i += 8;
                        goto EXECrCB;
                    case 173:
                        rL = (res(32, rL));
                        i += 8;
                        goto EXECrCB;
                    case 174: {
                        // ii = HL();
                        mem->writeMem(ii, res(32, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 175:
                        rA = (res(32, rA));
                        i += 8;
                        goto EXECrCB;
                    case 176:
                        rB = (res(64, rB));
                        i += 8;
                        goto EXECrCB;
                    case 177:
                        rC = (res(64, rC));
                        i += 8;
                        goto EXECrCB;
                    case 178:
                        rD = (res(64, rD));
                        i += 8;
                        goto EXECrCB;
                    case 179:
                        rE = (res(64, rE));
                        i += 8;
                        goto EXECrCB;
                    case 180:
                        rH = (res(64, rH));
                        i += 8;
                        goto EXECrCB;
                    case 181:
                        rL = (res(64, rL));
                        i += 8;
                        goto EXECrCB;
                    case 182: {
                        // ii = HL();
                        mem->writeMem(ii, res(64, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 183:
                        rA = (res(64, rA));
                        i += 8;
                        goto EXECrCB;
                    case 184:
                        rB = (res(128, rB));
                        i += 8;
                        goto EXECrCB;
                    case 185:
                        rC = (res(128, rC));
                        i += 8;
                        goto EXECrCB;
                    case 186:
                        rD = (res(128, rD));
                        i += 8;
                        goto EXECrCB;
                    case 187:
                        rE = (res(128, rE));
                        i += 8;
                        goto EXECrCB;
                    case 188:
                        rH = (res(128, rH));
                        i += 8;
                        goto EXECrCB;
                    case 189:
                        rL = (res(128, rL));
                        i += 8;
                        goto EXECrCB;
                    case 190: {
                        // ii = HL();
                        mem->writeMem(ii, res(128, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 191:
                        rA = (res(128, rA));
                        i += 8;
                        goto EXECrCB;
                    case 192:
                        rB = (set(1, rB));
                        i += 8;
                        goto EXECrCB;
                    case 193:
                        rC = (set(1, rC));
                        i += 8;
                        goto EXECrCB;
                    case 194:
                        rD = (set(1, rD));
                        i += 8;
                        goto EXECrCB;
                    case 195:
                        rE = (set(1, rE));
                        i += 8;
                        goto EXECrCB;
                    case 196:
                        rH = (set(1, rH));
                        i += 8;
                        goto EXECrCB;
                    case 197:
                        rL = (set(1, rL));
                        i += 8;
                        goto EXECrCB;
                    case 198: {
                        // ii = HL();
                        mem->writeMem(ii, set(1, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 199:
                        rA = (set(1, rA));
                        i += 8;
                        goto EXECrCB;
                    case 200:
                        rB = (set(2, rB));
                        i += 8;
                        goto EXECrCB;
                    case 201:
                        rC = (set(2, rC));
                        i += 8;
                        goto EXECrCB;
                    case 202:
                        rD = (set(2, rD));
                        i += 8;
                        goto EXECrCB;
                    case 203:
                        rE = (set(2, rE));
                        i += 8;
                        goto EXECrCB;
                    case 204:
                        rH = (set(2, rH));
                        i += 8;
                        goto EXECrCB;
                    case 205:
                        rL = (set(2, rL));
                        i += 8;
                        goto EXECrCB;
                    case 206: {
                        // ii = HL();
                        mem->writeMem(ii, set(2, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 207:
                        rA = (set(2, rA));
                        i += 8;
                        goto EXECrCB;
                    case 208:
                        rB = (set(4, rB));
                        i += 8;
                        goto EXECrCB;
                    case 209:
                        rC = (set(4, rC));
                        i += 8;
                        goto EXECrCB;
                    case 210:
                        rD = (set(4, rD));
                        i += 8;
                        goto EXECrCB;
                    case 211:
                        rE = (set(4, rE));
                        i += 8;
                        goto EXECrCB;
                    case 212:
                        rH = (set(4, rH));
                        i += 8;
                        goto EXECrCB;
                    case 213:
                        rL = (set(4, rL));
                        i += 8;
                        goto EXECrCB;
                    case 214: {
                        // ii = HL();
                        mem->writeMem(ii, set(4, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 215:
                        rA = (set(4, rA));
                        i += 8;
                        goto EXECrCB;
                    case 216:
                        rB = (set(8, rB));
                        i += 8;
                        goto EXECrCB;
                    case 217:
                        rC = (set(8, rC));
                        i += 8;
                        goto EXECrCB;
                    case 218:
                        rD = (set(8, rD));
                        i += 8;
                        goto EXECrCB;
                    case 219:
                        rE = (set(8, rE));
                        i += 8;
                        goto EXECrCB;
                    case 220:
                        rH = (set(8, rH));
                        i += 8;
                        goto EXECrCB;
                    case 221:
                        rL = (set(8, rL));
                        i += 8;
                        goto EXECrCB;
                    case 222: {
                        // ii = HL();
                        mem->writeMem(ii, set(8, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 223:
                        rA = (set(8, rA));
                        i += 8;
                        goto EXECrCB;
                    case 224:
                        rB = (set(16, rB));
                        i += 8;
                        goto EXECrCB;
                    case 225:
                        rC = (set(16, rC));
                        i += 8;
                        goto EXECrCB;
                    case 226:
                        rD = (set(16, rD));
                        i += 8;
                        goto EXECrCB;
                    case 227:
                        rE = (set(16, rE));
                        i += 8;
                        goto EXECrCB;
                    case 228:
                        rH = (set(16, rH));
                        i += 8;
                        goto EXECrCB;
                    case 229:
                        rL = (set(16, rL));
                        i += 8;
                        goto EXECrCB;
                    case 230: {
                        // ii = HL();
                        mem->writeMem(ii, set(16, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 231:
                        rA = (set(16, rA));
                        i += 8;
                        goto EXECrCB;
                    case 232:
                        rB = (set(32, rB));
                        i += 8;
                        goto EXECrCB;
                    case 233:
                        rC = (set(32, rC));
                        i += 8;
                        goto EXECrCB;
                    case 234:
                        rD = (set(32, rD));
                        i += 8;
                        goto EXECrCB;
                    case 235:
                        rE = (set(32, rE));
                        i += 8;
                        goto EXECrCB;
                    case 236:
                        rH = (set(32, rH));
                        i += 8;
                        goto EXECrCB;
                    case 237:
                        rL = (set(32, rL));
                        i += 8;
                        goto EXECrCB;
                    case 238: {
                        // ii = HL();
                        mem->writeMem(ii, set(32, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 239:
                        rA = (set(32, rA));
                        i += 8;
                        goto EXECrCB;
                    case 240:
                        rB = (set(64, rB));
                        i += 8;
                        goto EXECrCB;
                    case 241:
                        rC = (set(64, rC));
                        i += 8;
                        goto EXECrCB;
                    case 242:
                        rD = (set(64, rD));
                        i += 8;
                        goto EXECrCB;
                    case 243:
                        rE = (set(64, rE));
                        i += 8;
                        goto EXECrCB;
                    case 244:
                        rH = (set(64, rH));
                        i += 8;
                        goto EXECrCB;
                    case 245:
                        rL = (set(64, rL));
                        i += 8;
                        goto EXECrCB;
                    case 246: {
                        // ii = HL();
                        mem->writeMem(ii, set(64, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 247:
                        rA = (set(64, rA));
                        i += 8;
                        goto EXECrCB;
                    case 248:
                        rB = (set(128, rB));
                        i += 8;
                        goto EXECrCB;
                    case 249:
                        rC = (set(128, rC));
                        i += 8;
                        goto EXECrCB;
                    case 250:
                        rD = (set(128, rD));
                        i += 8;
                        goto EXECrCB;
                    case 251:
                        rE = (set(128, rE));
                        i += 8;
                        goto EXECrCB;
                    case 252:
                        rH = (set(128, rH));
                        i += 8;
                        goto EXECrCB;
                    case 253:
                        rL = (set(128, rL));
                        i += 8;
                        goto EXECrCB;
                    case 254: {
                        // ii = HL();
                        mem->writeMem(ii, set(128, mem->readMem(ii)));
                        i += 15;
                    }
                        goto EXECrCB;
                    case 255:
                        rA = (set(128, rA));
                        i += 8;
                        goto EXECrCB;
                    default:
                        i += 0;
                }
            EXECrCB:;
            } break;
            case 211:
                outb(mem->readMem(rPC++), rA);
                i += 11;
                break;
            case 219:
                rA = (inb(mem->readMem(rPC++)));
                i += 11;
                break;
            case 227: {
                i_3_ = HL();
                i_2_ = rSP;
                setHL(mem->readMemWord(i_2_));
                mem->writeMemWord(i_2_, i_3_);
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
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 204:
                if (fZ) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 212:
                if (!fC) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 220:
                if (fC) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 228:
                if (!fPV) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 236:
                if (fPV) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 244:
                if (!fS) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 252:
                if (fS) {
                    i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                    pushpc();
                    rPC = (i_2_);
                    i += 17;
                } else {
                    rPC = (rPC + 2 & 0xffff);
                    i += 10;
                }
                break;
            case 197:
                pushw(BC());
                i += 11;
                break;
            case 205: {
                i_2_ = mem->readMemWord((rPC = rPC + 2) - 2);
                pushpc();
                rPC = (i_2_);
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
                    rID = rIX;
                } else {
                    rID = rIY;
                }
                {
                    int ii;
                    int ii2;
                    rR += (1);
                    switch (mem->readMem(rPC++)) {
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
                            rPC = rPC - 1 & 0xffff;
                            rR += (-1);
                            i += 4;
                            goto EXECrID;
                        case 9:
                            rID = add16(rID, BC());
                            i += 15;
                            goto EXECrID;
                        case 25:
                            rID = add16(rID, DE());
                            i += 15;
                            goto EXECrID;
                        case 41: {
                            ii  = rID;
                            rID = add16(ii, ii);
                            i += 15;
                        }
                            goto EXECrID;
                        case 57:
                            rID = add16(rID, rSP);
                            i += 15;
                            goto EXECrID;
                        case 33:
                            rID = mem->readMemWord((rPC = rPC + 2) - 2);
                            i += 14;
                            goto EXECrID;
                        case 34:
                            mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), rID);
                            i += 20;
                            goto EXECrID;
                        case 42:
                            rID = mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2));
                            i += 20;
                            goto EXECrID;
                        case 35:
                            rID = rID + 1 & 0xffff;
                            i += 10;
                            goto EXECrID;
                        case 43:
                            rID = rID - 1 & 0xffff;
                            i += 10;
                            goto EXECrID;
                        case 36:
                            setIDH(inc8(rID >> 8));
                            i += 8;
                            goto EXECrID;
                        case 44:
                            setIDL(inc8(rID & 0xff));
                            i += 8;
                            goto EXECrID;
                        case 52: {
                            ii = ID_d();
                            mem->writeMem(ii, inc8(mem->readMem(ii)));
                            i += 23;
                        }
                            goto EXECrID;
                        case 37:
                            setIDH(dec8(rID >> 8));
                            i += 8;
                            goto EXECrID;
                        case 45:
                            setIDL(dec8(rID & 0xff));
                            i += 8;
                            goto EXECrID;
                        case 53: {
                            ii = ID_d();
                            mem->writeMem(ii, dec8(mem->readMem(ii)));
                            i += 23;
                        }
                            goto EXECrID;
                        case 38:
                            setIDH(mem->readMem(rPC++));
                            i += 11;
                            goto EXECrID;
                        case 46:
                            setIDL(nxtpcb());
                            i += 11;
                            goto EXECrID;
                        case 54: {
                            ii = ID_d();
                            mem->writeMem(ii, mem->readMem(rPC++));
                            i += 19;
                        }
                            goto EXECrID;
                        case 68:
                            rB = (rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 69:
                            rB = (rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 70:
                            rB = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 76:
                            rC = (rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 77:
                            rC = (rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 78:
                            rC = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 84:
                            rD = (rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 85:
                            rD = (rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 86:
                            rD = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 92:
                            rE = (rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 93:
                            rE = (rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 94:
                            rE = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 96:
                            setIDH(rB);
                            i += 8;
                            goto EXECrID;
                        case 97:
                            setIDH(rC);
                            i += 8;
                            goto EXECrID;
                        case 98:
                            setIDH(rD);
                            i += 8;
                            goto EXECrID;
                        case 99:
                            setIDH(rE);
                            i += 8;
                            goto EXECrID;
                        case 100:
                            i += 8;
                            goto EXECrID;
                        case 101:
                            setIDH(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 102:
                            rH = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 103:
                            setIDH(rA);
                            i += 8;
                            goto EXECrID;
                        case 104:
                            setIDL(rB);
                            i += 8;
                            goto EXECrID;
                        case 105:
                            setIDL(rC);
                            i += 8;
                            goto EXECrID;
                        case 106:
                            setIDL(rD);
                            i += 8;
                            goto EXECrID;
                        case 107:
                            setIDL(rE);
                            i += 8;
                            goto EXECrID;
                        case 108:
                            setIDL(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 109:
                            i += 8;
                            goto EXECrID;
                        case 110:
                            rL = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 111:
                            setIDL(rA);
                            i += 8;
                            goto EXECrID;
                        case 112:
                            mem->writeMem(ID_d(), rB);
                            i += 19;
                            goto EXECrID;
                        case 113:
                            mem->writeMem(ID_d(), rC);
                            i += 19;
                            goto EXECrID;
                        case 114:
                            mem->writeMem(ID_d(), rD);
                            i += 19;
                            goto EXECrID;
                        case 115:
                            mem->writeMem(ID_d(), rE);
                            i += 19;
                            goto EXECrID;
                        case 116:
                            mem->writeMem(ID_d(), rH);
                            i += 19;
                            goto EXECrID;
                        case 117:
                            mem->writeMem(ID_d(), rL);
                            i += 19;
                            goto EXECrID;
                        case 119:
                            mem->writeMem(ID_d(), rA);
                            i += 19;
                            goto EXECrID;
                        case 124:
                            rA = (rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 125:
                            rA = (rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 126:
                            rA = (mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 132:
                            add_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 133:
                            add_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 134:
                            add_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 140:
                            adc_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 141:
                            adc_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 142:
                            adc_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 148:
                            sub_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 149:
                            sub_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 150:
                            sub_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 156:
                            sbc_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 157:
                            sbc_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 158:
                            sbc_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 164:
                            and_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 165:
                            and_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 166:
                            and_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 172:
                            xor_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 173:
                            xor_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 174:
                            xor_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 180:
                            or_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 181:
                            or_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 182:
                            or_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 188:
                            cp_a(rID >> 8);
                            i += 8;
                            goto EXECrID;
                        case 189:
                            cp_a(rID & 0xff);
                            i += 8;
                            goto EXECrID;
                        case 190:
                            cp_a(mem->readMem(ID_d()));
                            i += 19;
                            goto EXECrID;
                        case 225:
                            rID = (popw());
                            i += 14;
                            goto EXECrID;
                        case 233:
                            rPC = (rID);
                            i += 8;
                            goto EXECrID;
                        case 249:
                            rSP = (rID);
                            i += 10;
                            goto EXECrID;
                        case 203: {
                            ii  = ID_d();
                            ii2 = mem->readMem(rPC++);
                            {
                                switch (ii2) {
                                    case 0:
                                        rB = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 1:
                                        rC = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 2:
                                        rD = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 3:
                                        rE = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 4:
                                        rH = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 5:
                                        rL = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 6:
                                        mem->writeMem(ii, rlc(mem->readMem(ii)));
                                        break;
                                    case 7:
                                        rA = (ii2 = rlc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 8:
                                        rB = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 9:
                                        rC = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 10:
                                        rD = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 11:
                                        rE = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 12:
                                        rH = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 13:
                                        rL = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 14:
                                        mem->writeMem(ii, rrc(mem->readMem(ii)));
                                        break;
                                    case 15:
                                        rA = (ii2 = rrc(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 16:
                                        rB = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 17:
                                        rC = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 18:
                                        rD = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 19:
                                        rE = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 20:
                                        rH = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 21:
                                        rL = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 22:
                                        mem->writeMem(ii, rl(mem->readMem(ii)));
                                        break;
                                    case 23:
                                        rA = (ii2 = rl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 24:
                                        rB = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 25:
                                        rC = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 26:
                                        rD = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 27:
                                        rE = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 28:
                                        rH = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 29:
                                        rL = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 30:
                                        mem->writeMem(ii, rr(mem->readMem(ii)));
                                        break;
                                    case 31:
                                        rA = (ii2 = rr(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 32:
                                        rB = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 33:
                                        rC = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 34:
                                        rD = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 35:
                                        rE = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 36:
                                        rH = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 37:
                                        rL = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 38:
                                        mem->writeMem(ii, sla(mem->readMem(ii)));
                                        break;
                                    case 39:
                                        rA = (ii2 = sla(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 40:
                                        rB = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 41:
                                        rC = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 42:
                                        rD = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 43:
                                        rE = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 44:
                                        rH = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 45:
                                        rL = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 46:
                                        mem->writeMem(ii, sra(mem->readMem(ii)));
                                        break;
                                    case 47:
                                        rA = (ii2 = sra(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 48:
                                        rB = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 49:
                                        rC = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 50:
                                        rD = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 51:
                                        rE = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 52:
                                        rH = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 53:
                                        rL = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 54:
                                        mem->writeMem(ii, sls(mem->readMem(ii)));
                                        break;
                                    case 55:
                                        rA = (ii2 = sls(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 56:
                                        rB = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 57:
                                        rC = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 58:
                                        rD = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 59:
                                        rE = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 60:
                                        rH = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 61:
                                        rL = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 62:
                                        mem->writeMem(ii, srl(mem->readMem(ii)));
                                        break;
                                    case 63:
                                        rA = (ii2 = srl(mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 64:
                                    case 65:
                                    case 66:
                                    case 67:
                                    case 68:
                                    case 69:
                                    case 70:
                                    case 71:
                                        bit(1, mem->readMem(ii));
                                        break;
                                    case 72:
                                    case 73:
                                    case 74:
                                    case 75:
                                    case 76:
                                    case 77:
                                    case 78:
                                    case 79:
                                        bit(2, mem->readMem(ii));
                                        break;
                                    case 80:
                                    case 81:
                                    case 82:
                                    case 83:
                                    case 84:
                                    case 85:
                                    case 86:
                                    case 87:
                                        bit(4, mem->readMem(ii));
                                        break;
                                    case 88:
                                    case 89:
                                    case 90:
                                    case 91:
                                    case 92:
                                    case 93:
                                    case 94:
                                    case 95:
                                        bit(8, mem->readMem(ii));
                                        break;
                                    case 96:
                                    case 97:
                                    case 98:
                                    case 99:
                                    case 100:
                                    case 101:
                                    case 102:
                                    case 103:
                                        bit(16, mem->readMem(ii));
                                        break;
                                    case 104:
                                    case 105:
                                    case 106:
                                    case 107:
                                    case 108:
                                    case 109:
                                    case 110:
                                    case 111:
                                        bit(32, mem->readMem(ii));
                                        break;
                                    case 112:
                                    case 113:
                                    case 114:
                                    case 115:
                                    case 116:
                                    case 117:
                                    case 118:
                                    case 119:
                                        bit(64, mem->readMem(ii));
                                        break;
                                    case 120:
                                    case 121:
                                    case 122:
                                    case 123:
                                    case 124:
                                    case 125:
                                    case 126:
                                    case 127:
                                        bit(128, mem->readMem(ii));
                                        break;
                                    case 128:
                                        rB = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 129:
                                        rC = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 130:
                                        rD = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 131:
                                        rE = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 132:
                                        rH = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 133:
                                        rL = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 134:
                                        mem->writeMem(ii, res(1, mem->readMem(ii)));
                                        break;
                                    case 135:
                                        rA = (ii2 = res(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 136:
                                        rB = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 137:
                                        rC = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 138:
                                        rD = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 139:
                                        rE = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 140:
                                        rH = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 141:
                                        rL = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 142:
                                        mem->writeMem(ii, res(2, mem->readMem(ii)));
                                        break;
                                    case 143:
                                        rA = (ii2 = res(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 144:
                                        rB = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 145:
                                        rC = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 146:
                                        rD = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 147:
                                        rE = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 148:
                                        rH = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 149:
                                        rL = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 150:
                                        mem->writeMem(ii, res(4, mem->readMem(ii)));
                                        break;
                                    case 151:
                                        rA = (ii2 = res(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 152:
                                        rB = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 153:
                                        rC = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 154:
                                        rD = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 155:
                                        rE = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 156:
                                        rH = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 157:
                                        rL = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 158:
                                        mem->writeMem(ii, res(8, mem->readMem(ii)));
                                        break;
                                    case 159:
                                        rA = (ii2 = res(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 160:
                                        rB = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 161:
                                        rC = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 162:
                                        rD = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 163:
                                        rE = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 164:
                                        rH = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 165:
                                        rL = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 166:
                                        mem->writeMem(ii, res(16, mem->readMem(ii)));
                                        break;
                                    case 167:
                                        rA = (ii2 = res(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 168:
                                        rB = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 169:
                                        rC = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 170:
                                        rD = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 171:
                                        rE = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 172:
                                        rH = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 173:
                                        rL = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 174:
                                        mem->writeMem(ii, res(32, mem->readMem(ii)));
                                        break;
                                    case 175:
                                        rA = (ii2 = res(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 176:
                                        rB = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 177:
                                        rC = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 178:
                                        rD = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 179:
                                        rE = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 180:
                                        rH = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 181:
                                        rL = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 182:
                                        mem->writeMem(ii, res(64, mem->readMem(ii)));
                                        break;
                                    case 183:
                                        rA = (ii2 = res(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 184:
                                        rB = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 185:
                                        rC = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 186:
                                        rD = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 187:
                                        rE = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 188:
                                        rH = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 189:
                                        rL = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 190:
                                        mem->writeMem(ii, res(128, mem->readMem(ii)));
                                        break;
                                    case 191:
                                        rA = (ii2 = res(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 192:
                                        rB = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 193:
                                        rC = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 194:
                                        rD = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 195:
                                        rE = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 196:
                                        rH = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 197:
                                        rL = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 198:
                                        mem->writeMem(ii, set(1, mem->readMem(ii)));
                                        break;
                                    case 199:
                                        rA = (ii2 = set(1, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 200:
                                        rB = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 201:
                                        rC = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 202:
                                        rD = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 203:
                                        rE = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 204:
                                        rH = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 205:
                                        rL = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 206:
                                        mem->writeMem(ii, set(2, mem->readMem(ii)));
                                        break;
                                    case 207:
                                        rA = (ii2 = set(2, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 208:
                                        rB = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 209:
                                        rC = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 210:
                                        rD = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 211:
                                        rE = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 212:
                                        rH = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 213:
                                        rL = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 214:
                                        mem->writeMem(ii, set(4, mem->readMem(ii)));
                                        break;
                                    case 215:
                                        rA = (ii2 = set(4, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 216:
                                        rB = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 217:
                                        rC = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 218:
                                        rD = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 219:
                                        rE = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 220:
                                        rH = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 221:
                                        rL = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 222:
                                        mem->writeMem(ii, set(8, mem->readMem(ii)));
                                        break;
                                    case 223:
                                        rA = (ii2 = set(8, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 224:
                                        rB = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 225:
                                        rC = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 226:
                                        rD = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 227:
                                        rE = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 228:
                                        rH = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 229:
                                        rL = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 230:
                                        mem->writeMem(ii, set(16, mem->readMem(ii)));
                                        break;
                                    case 231:
                                        rA = (ii2 = set(16, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 232:
                                        rB = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 233:
                                        rC = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 234:
                                        rD = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 235:
                                        rE = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 236:
                                        rH = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 237:
                                        rL = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 238:
                                        mem->writeMem(ii, set(32, mem->readMem(ii)));
                                        break;
                                    case 239:
                                        rA = (ii2 = set(32, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 240:
                                        rB = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 241:
                                        rC = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 242:
                                        rD = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 243:
                                        rE = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 244:
                                        rH = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 245:
                                        rL = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 246:
                                        mem->writeMem(ii, set(64, mem->readMem(ii)));
                                        break;
                                    case 247:
                                        rA = (ii2 = set(64, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 248:
                                        rB = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 249:
                                        rC = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 250:
                                        rD = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 251:
                                        rE = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 252:
                                        rH = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 253:
                                        rL = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                    case 254:
                                        mem->writeMem(ii, set(128, mem->readMem(ii)));
                                        break;
                                    case 255:
                                        rA = (ii2 = set(128, mem->readMem(ii)));
                                        mem->writeMem(ii, ii2);
                                        break;
                                }
                            }
                            i += (ii2 & 0xc0) == 64 ? 20 : 23;
                        }
                            goto EXECrID;
                        case 227: {
                            ii  = rID;
                            ii2 = rSP;
                            rID = (mem->readMemWord(ii2));
                            mem->writeMemWord(ii2, ii);
                            i += 23;
                        }
                            goto EXECrID;
                        case 229:
                            pushw(rID);
                            i += 15;
                            goto EXECrID;
                        default:
                            i += 0;
                            goto EXECrID;
                    }

                EXECrID:;
                }

                if (opcode == 221) {
                    rIX = rID;
                } else {
                    rIY = rID;
                }
                break;

            case 229:
                pushw(HL());
                i += 11;
                break;
            case 237: {
                int ii2;
                int ii3;
                rR += (1);
                switch (mem->readMem(rPC++)) {
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
                        goto EXECrED;
                    case 64:
                        rB = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 72:
                        rC = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 80:
                        rD = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 88:
                        rE = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 96:
                        rH = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 104:
                        rL = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 112:
                        in_bc();
                        i += 12;
                        goto EXECrED;
                    case 120:
                        rA = (in_bc());
                        i += 12;
                        goto EXECrED;
                    case 65:
                        outb(rC, rB);
                        i += 12;
                        goto EXECrED;
                    case 73:
                        outb(rC, rC);
                        i += 12;
                        goto EXECrED;
                    case 81:
                        outb(rC, rD);
                        i += 12;
                        goto EXECrED;
                    case 89:
                        outb(rC, rE);
                        i += 12;
                        goto EXECrED;
                    case 97:
                        outb(rC, rH);
                        i += 12;
                        goto EXECrED;
                    case 105:
                        outb(rC, rL);
                        i += 12;
                        goto EXECrED;
                    case 113:
                        outb(rC, 0);
                        i += 12;
                        goto EXECrED;
                    case 121:
                        outb(rC, rA);
                        i += 12;
                        goto EXECrED;
                    case 66:
                        setHL(sbc16(HL(), BC()));
                        i += 15;
                        goto EXECrED;
                    case 74:
                        setHL(adc16(HL(), BC()));
                        i += 15;
                        goto EXECrED;
                    case 82:
                        setHL(sbc16(HL(), DE()));
                        i += 15;
                        goto EXECrED;
                    case 90:
                        setHL(adc16(HL(), DE()));
                        i += 15;
                        goto EXECrED;
                    case 98: {
                        ii2 = HL();
                        setHL(sbc16(ii2, ii2));
                        i += 15;
                    }
                        goto EXECrED;
                    case 106: {
                        ii2 = HL();
                        setHL(adc16(ii2, ii2));
                        i += 15;
                    }
                        goto EXECrED;
                    case 114:
                        setHL(sbc16(HL(), rSP));
                        i += 15;
                        goto EXECrED;
                    case 122:
                        setHL(adc16(HL(), rSP));
                        i += 15;
                        goto EXECrED;
                    case 67:
                        mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), BC());
                        i += 20;
                        goto EXECrED;
                    case 75:
                        setBC(mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2)));
                        i += 20;
                        goto EXECrED;
                    case 83:
                        mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), DE());
                        i += 20;
                        goto EXECrED;
                    case 91:
                        setDE(mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2)));
                        i += 20;
                        goto EXECrED;
                    case 99:
                        mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), HL());
                        i += 20;
                        goto EXECrED;
                    case 107:
                        setHL(mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2)));
                        i += 20;
                        goto EXECrED;
                    case 115:
                        mem->writeMemWord(mem->readMemWord((rPC = rPC + 2) - 2), rSP);
                        i += 20;
                        goto EXECrED;
                    case 123:
                        rSP = (mem->readMemWord(mem->readMemWord((rPC = rPC + 2) - 2)));
                        i += 20;
                        goto EXECrED;
                    case 68:
                    case 76:
                    case 84:
                    case 92:
                    case 100:
                    case 108:
                    case 116:
                    case 124: {
                        int ii = rA;
                        rA     = (0);
                        sub_a(ii);
                        i += 8;
                    }
                        goto EXECrED;
                    case 69:
                    case 85:
                    case 101:
                    case 117:
                        setIFF1(IFF2);
                        poppc();
                        i += 14;
                        goto EXECrED;
                    case 77:
                    case 93:
                    case 109:
                    case 125:
                        poppc();
                        i += 14;
                        goto EXECrED;
                    case 70:
                    case 78:
                    case 102:
                    case 110:
                        rIM = (0);
                        i += 8;
                        goto EXECrED;
                    case 86:
                    case 118:
                        rIM = (1);
                        i += 8;
                        goto EXECrED;
                    case 94:
                    case 126:
                        rIM = (2);
                        i += 8;
                        goto EXECrED;
                    case 71:
                        rI = (rA);
                        i += 9;
                        goto EXECrED;
                    case 79:
                        rR  = rA;
                        rR7 = rA & 0x80;
                        i += 9;
                        goto EXECrED;
                    case 87: {
                        int ii = rI;
                        fS     = ((ii & 0x80) != 0);
                        f3     = ((ii & 0x8) != 0);
                        f5     = ((ii & 0x20) != 0);
                        fZ     = (ii == 0);
                        fPV    = (IFF2);
                        fH     = (false);
                        fN     = (false);
                        rA     = (ii);
                        i += 9;
                    }
                        goto EXECrED;
                    case 95: {
                        int ii = rR & 0x7f | rR7;
                        fS     = ((ii & 0x80) != 0);
                        f3     = ((ii & 0x8) != 0);
                        f5     = ((ii & 0x20) != 0);
                        fZ     = (ii == 0);
                        fPV    = (IFF2);
                        fH     = (false);
                        fN     = (false);
                        rA     = (ii);
                        i += 9;
                    }
                        goto EXECrED;
                    case 103: {
                        int ii    = rA;
                        int i_76_ = mem->readMem(HL());
                        int i_77_ = i_76_;
                        i_76_     = i_76_ >> 4 | ii << 4;
                        ii        = ii & 0xf0 | i_77_ & 0xf;
                        mem->writeMem(HL(), i_76_);
                        fS  = ((ii & 0x80) != 0);
                        f3  = ((ii & 0x8) != 0);
                        f5  = ((ii & 0x20) != 0);
                        fZ  = (ii == 0);
                        fPV = (parity[ii]);
                        fH  = (false);
                        fN  = (false);
                        rA  = (ii);
                        i += 18;
                    }
                        goto EXECrED;
                    case 111: {
                        int ii    = rA;
                        int i_74_ = mem->readMem(HL());
                        int i_75_ = i_74_;
                        i_74_     = i_74_ << 4 | ii & 0xf;
                        ii        = ii & 0xf0 | i_75_ >> 4;
                        mem->writeMem(HL(), i_74_ & 0xff);
                        fS  = ((ii & 0x80) != 0);
                        f3  = ((ii & 0x8) != 0);
                        f5  = ((ii & 0x20) != 0);
                        fZ  = (ii == 0);
                        fPV = (parity[ii]);
                        fH  = (false);
                        fN  = (false);
                        rA  = (ii);
                        i += 18;
                    }
                        goto EXECrED;
                    case 160:
                        mem->writeMem(DE(), mem->readMem(HL()));
                        setDE((rD << 8 | rE) + 1 & 0xffff);
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fH  = (false);
                        fN  = (false);
                        i += 16;
                        goto EXECrED;
                    case 161: {
                        ii2 = fC;
                        cp_a(mem->readMem(HL()));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fC  = (ii2);
                        i += 16;
                    }
                        goto EXECrED;
                    case 162:
                        mem->writeMem(HL(), inb(rC));
                        rB = (dec8(rB));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        i += 16;
                        goto EXECrED;
                    case 163:
                        rB = (dec8(rB));
                        outb(rC, mem->readMem(HL()));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        i += 16;
                        goto EXECrED;
                    case 168:
                        mem->writeMem(DE(), mem->readMem(HL()));
                        setDE((rD << 8 | rE) - 1 & 0xffff);
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        fH  = (false);
                        fN  = (false);
                        i += 16;
                        goto EXECrED;
                    case 169: {
                        // var bool = fC;
                        cp_a(mem->readMem(HL()));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        fPV = (BC() != 0);
                        i += 16;
                    }
                        goto EXECrED;
                    case 170:
                        mem->writeMem(HL(), inb(rC));
                        rB = (dec8(rB));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        i += 16;
                        goto EXECrED;
                    case 171:
                        rB = (dec8(rB));
                        outb(rC, mem->readMem(HL()));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        i += 16;
                        goto EXECrED;
                    case 176: {
                        // var bool = false;
                        mem->writeMem(DE(), mem->readMem(HL()));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        setDE((rD << 8 | rE) + 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        ii2 = 21;
                        rR += (4);
                        if (BC() != 0) {
                            rPC = (rPC - 2 & 0xffff);
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
                        goto EXECrED;
                    case 177: {
                        ii2 = fC;
                        cp_a(mem->readMem(HL()));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        ii3 = BC() != 0;
                        fPV = (ii3);
                        fC  = (ii2);
                        if (ii3 && !fZ) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                    }
                        goto EXECrED;
                    case 178: {
                        // var bool = false;
                        ii2 = 0;
                        mem->writeMem(HL(), inb(rC));
                        rB = (ii2 = dec8(rB));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        if (ii2 != 0) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                    }
                        goto EXECrED;
                    case 179: {
                        // var bool = false;
                        ii2 = 0;
                        rB  = (ii2 = dec8(rB));
                        outb(rC, mem->readMem(HL()));
                        setHL((rH << 8 | rL) + 1 & 0xffff);
                        if (ii2 != 0) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                    }
                        goto EXECrED;
                    case 184: {
                        // var bool = false;
                        ii2 = 21;
                        rR += (4);
                        mem->writeMem(DE(), mem->readMem(HL()));
                        setDE((rD << 8 | rE) - 1 & 0xffff);
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        if (BC() != 0) {
                            rPC = (rPC - 2 & 0xffff);
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
                        goto EXECrED;
                    case 185: {
                        ii2 = fC;
                        cp_a(mem->readMem(HL()));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        setBC((rB << 8 | rC) - 1 & 0xffff);
                        ii3 = BC() != 0;
                        fPV = (ii3);
                        fC  = (ii2);
                        if (ii3 && !fZ) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                    }
                        goto EXECrED;
                    case 186: {
                        ii2 = 0;
                        mem->writeMem(HL(), inb(BC() & 0xff));
                        rB = (ii2 = dec8(rB));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        if (ii2 != 0) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                    }
                        goto EXECrED;
                    case 187: {
                        ii2 = 0;
                        rB  = (ii2 = dec8(rB));
                        outb(rC, mem->readMem(HL()));
                        setHL((rH << 8 | rL) - 1 & 0xffff);
                        if (ii2 != 0) {
                            rPC = (rPC - 2 & 0xffff);
                            i += 21;
                            goto EXECrED;
                        }
                        i += 16;
                        goto EXECrED;
                    }
                    default:
                        i += 8;
                        goto EXECrED;
                }
            EXECrED:;
            } break;
            case 245: {
                int f = ((fS ? 128 : 0) | (fZ ? 64 : 0) | (f5 ? 32 : 0) | (fH ? 16 : 0) | (f3 ? 8 : 0) | (fPV ? 4 : 0) |
                         (fN ? 2 : 0) | (fC ? 1 : 0));
                pushw(rA << 8 | f);
                i += 11;
            } break;
            case 198:
                add_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 206:
                adc_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 214:
                sub_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 222:
                sbc_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 230:
                and_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 238:
                xor_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 246:
                or_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 254:
                cp_a(mem->readMem(rPC++));
                i += 7;
                break;
            case 199:
                pushpc();
                rPC = (0);
                i += 11;
                break;
            case 207:
                pushpc();
                rPC = (8);
                i += 11;
                break;
            case 215:
                pushpc();
                rPC = (16);
                i += 11;
                break;
            case 223:
                pushpc();
                rPC = (24);
                i += 11;
                break;
            case 231:
                pushpc();
                rPC = (32);
                i += 11;
                break;
            case 239:
                pushpc();
                rPC = (40);
                i += 11;
                break;
            case 247:
                pushpc();
                rPC = (48);
                i += 11;
                break;
            case 255:
                pushpc();
                rPC = (56);
                i += 11;
        }
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
{
    return ((i & 0x80) != 0) ? i - 256 : i;
}
void z80::setBC(int i)
{
    rB = (i >> 8);
    rC = (i & 0xff);
}
int z80::HL()
{
    return rH << 8 | rL;
}
int z80::BC()
{
    return rB << 8 | rC;
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
    rH = (i >> 8);
    rL = (i & 0xff);
}
void z80::setDE(int i)
{
    rD = (i >> 8);
    rE = (i & 0xff);
}
int z80::DE()
{
    return rD << 8 | rE;
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
    int i_87_ = rA;
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
    rA        = (i_89_);
}
void z80::add_a(int i)
{
    int i_12_ = rA;
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
    rA        = (i_14_);
}
void z80::adc_a(int i)
{
    int i_5_ = rA;
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
    rA       = (i_8_);
}
void z80::sbc_a(int i)
{
    int i_82_ = rA;
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
    rA        = (i_85_);
}
void z80::and_a(int i)
{
    int i_15_ = rA & i;
    fS        = ((i_15_ & 0x80) != 0);
    f3        = ((i_15_ & 0x8) != 0);
    f5        = ((i_15_ & 0x20) != 0);
    fH        = (true);
    fPV       = (parity[i_15_ & 0xff]);
    fZ        = (i_15_ == 0);
    fN        = (false);
    fC        = (false);
    rA        = (i_15_);
}
void z80::xor_a(int i)
{
    int i_90_ = (rA ^ i) & 0xff;
    fS        = ((i_90_ & 0x80) != 0);
    f3        = ((i_90_ & 0x8) != 0);
    f5        = ((i_90_ & 0x20) != 0);
    fH        = (false);
    fPV       = (parity[i_90_]);
    fZ        = (i_90_ == 0);
    fN        = (false);
    fC        = (false);
    rA        = (i_90_);
}
void z80::or_a(int i)
{
    int i_62_ = rA | i;
    fS        = ((i_62_ & 0x80) != 0);
    f3        = ((i_62_ & 0x8) != 0);
    f5        = ((i_62_ & 0x20) != 0);
    fH        = (false);
    fPV       = (parity[i_62_]);
    fZ        = (i_62_ == 0);
    fN        = (false);
    fC        = (false);
    rA        = (i_62_);
}
void z80::cp_a(int i)
{
    int i_17_ = rA;
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
    rPC = popw();
}
int z80::popw()
{
    int i     = rSP;
    int i_67_ = mem->readMemWord(i++);
    rSP       = (++i & 0xffff);
    return i_67_;
}
void z80::pushpc()
{
    pushw(rPC);
}
void z80::pushw(int i)
{
    int i_70_ = rSP - 2 & 0xffff;
    rSP       = (i_70_);
    mem->writeMemWord(i_70_, i);
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
    IFF1 = flg;
}
void z80::setIFF2(bool flg)
{
    IFF2 = flg;
}
void z80::setIDH(int i)
{
    rID = i << 8 & 0xff00 | rID & 0xff;
}
void z80::setIDL(int i)
{
    rID = rID & 0xff00 | i & 0xff;
}
int z80::ID_d()
{
    return rID + bytef(mem->readMem(rPC++)) & 0xffff;
}
int z80::nxtpcb()
{
    return mem->readMem(rPC++);
}
int z80::in_bc()
{
    int i = inb(rC);
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
    if (!IFF1)
        return 0;
    switch (rIM) {
        case 0:
        case 1:
            pushpc();
            setIFF1(false);
            setIFF2(false);
            rPC = (56);
            return 13;
        case 2: {
            pushpc();
            setIFF1(false);
            setIFF2(false);
            rPC = (mem->readMemWord(rI << 8 | 0xff));
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
            return mem->PPIPortA;
        case 169:
            return msx->keyboard->state[mem->PPIPortC & 0xf];
        case 170:
            return mem->PPIPortC;
        case 171:
            return mem->PPIPortD;
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
            mem->megarom = true;
            break;
        case 160:
            msx->psg->escrevePortaEndereco(i_19_);
            break;
        case 161:
            msx->psg->escrevePortaDados(i_19_);
            break;
        case 168:
            mem->PPIPortA = i_19_;
            break;
        case 169:
            // PPIPortB = i_19_;
            break;
        case 170:
            mem->PPIPortC = i_19_;
            break;
        case 171:
            mem->PPIPortD = i_19_;
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
