#ifndef _H_TMS
#define _H_TMS
#include <cstdint>
#include <math.h>

class TMS9918 {

    bool updateWholeScreen = true;
    int  regStatus         = 0;
    int  screenAtual       = 0;

    int tabCor       = 0;
    int tabNome      = 0;
    int tabCar       = 0;
    int tabAtrSpt    = 0;
    int tabImgSpt    = 0;
    int regEnd       = 0;
    int byteLido     = 0;
    int ByteReadBuff = 0;
    int lidoByte     = false;
    int primeiro     = -1;
    int ultimo       = -1;

  public:
    uint8_t registros[8]          = {};
    uint8_t vidMem[16384]         = {};
    int     dirtyVidMem[960]      = {};
    uint8_t imagemTela[256 * 192] = {};

  public:
    void reset()
    {
        updateWholeScreen = true;
        regStatus         = 0;
        screenAtual       = 0;

        tabCor       = 0;
        tabNome      = 0;
        tabCar       = 0;
        tabAtrSpt    = 0;
        tabImgSpt    = 0;
        regEnd       = 0;
        byteLido     = 0;
        ByteReadBuff = 0;
        lidoByte     = false;
        primeiro     = -1;
        ultimo       = -1;

        for (int i = 0; i < 8; i++) {
            registros[i] = 0;
        }
        for (int i = 0; i < 16384; i++) {
            vidMem[i] = 0;
        }
        for (int i = 0; i < 960; i++) {
            dirtyVidMem[i] = -1;
        }
    }
    void updateScreen()
    {
    }
    void atualizaTudo()
    {
        int i    = 0;
        int i_0_ = screenAtual == 0 ? 40 : 32;
        int i_1_ = screenAtual == 0 ? 6 : 8;
        int i_2_ = screenAtual == 0 ? 960 : 768;
        int i_3_ = -1;
        int i_4_;
        int i_5_;
        int i_6_;
        int i_7_;
        int i_8_;
        int i_9_;
        int i_10_;

        for (i_4_ = tabNome; i_4_ < tabNome + i_2_; i_4_++) {
            if (screenAtual == 2)
                i = floor((i_4_ - tabNome) / 256);
            i_5_ = floor((i_4_ - tabNome) / i_0_);          // row
            i_6_ = tabCar + vidMem[i_4_] * 8 + 2048 * i;    // glyph
            for (i_7_ = i_6_; i_7_ < i_6_ + 8; i_7_++) {    // glyph rows
                for (i_8_ = 0; i_8_ < 8; i_8_++) {          // glyph row pixels
                    i_9_  = (((i_4_ - tabNome) % i_0_) * i_1_ + (i_7_ - i_6_) * 256 + i_8_ + 2048 * i_5_);
                    i_10_ = 0;
                    if ((vidMem[i_7_] & (1 << (7 - i_8_))) > 0) {
                        uint8_t valu = 0;

                        switch (screenAtual) {
                            case 0:
                                valu  = (registros[7] & 0xf0);
                                i_10_ = valu >> 4;
                                break;
                            case 1: {
                                uint16_t idx = tabCor + floor((i_7_ - tabCar) / 64);
                                valu         = (vidMem[idx] & 0xf0);
                                i_10_        = valu >> 4;
                            } break;
                            case 2:
                                valu  = (vidMem[tabCor + i_7_ - tabCar] & 0xf0);
                                i_10_ = (valu >> 4);
                                break;
                        }
                    } else {
                        switch (screenAtual) {
                            case 0:
                                i_10_ = registros[7] & 0xf;
                                break;
                            case 1: {
                                uint16_t idx = tabCor + floor((i_7_ - tabCar) / 64);
                                i_10_        = vidMem[idx] & 0xf;
                            } break;
                            case 2:
                                i_10_ = vidMem[tabCor + i_7_ - tabCar] & 0xf;
                                break;
                        }
                    }
                    imagemTela[i_9_] = i_10_;

                    // imagedata.data[i_9_ * 4 + 0] = palette[i_10_][0];    // r
                    // imagedata.data[i_9_ * 4 + 1] = palette[i_10_][1];    // g
                    // imagedata.data[i_9_ * 4 + 2] = palette[i_10_][2];    // b
                }
            }
        }
        updateWholeScreen = false;
        for (i_4_ = 0; i_4_ < 960; i_4_++)
            dirtyVidMem[i_4_] = -1;
        primeiro = -1;
    }
    void desenhaOtimizado()
    {
        int i     = 0;
        int i_13_ = screenAtual == 0 ? 40 : 32;
        int i_14_ = screenAtual == 0 ? 6 : 8;
        int i_15_ = screenAtual == 0 ? 960 : 768;
        int i_16_ = -1;
        int i_17_ = 0;
        int i_18_;
        int i_19_;
        int i_20_;
        int i_21_;
        int i_22_;
        int i_23_;
        int i_24_;
        int i_25_;

        for (; primeiro > -1; primeiro = i_17_) {
            if (primeiro < i_15_) {
                i_18_ = primeiro % i_13_;           // column
                i_18_ *= i_14_;                     // in pixels
                i_19_ = floor(primeiro / i_13_);    // row
                i_19_ *= 8;                         // in pixels
                if (screenAtual == 2)
                    i = floor(primeiro / 256);
                i_20_ = floor(primeiro / i_13_);                               // row
                i_21_ = tabCar + vidMem[primeiro + tabNome] * 8 + 2048 * i;    // glyph data
                for (i_22_ = i_21_; i_22_ < i_21_ + 8; i_22_++) {              // glyph rows
                    for (i_23_ = 0; i_23_ < 8; i_23_++) {                      // glyph row pixels
                        i_24_ = (i_18_ + (i_22_ - i_21_) * 256 + i_23_ + 2048 * i_20_);
                        // if ((vidMem[i_22_] & 1 << 7 - i_23_) > 0) {
                        i_25_ = 0;
                        if ((vidMem[i_22_] & (1 << (7 - i_23_))) > 0) {
                            switch (screenAtual) {
                                case 0: {
                                    uint16_t valu = (registros[7] & 0xf0);
                                    i_25_         = valu >> 4;
                                } break;
                                case 1: {
                                    uint16_t idx  = tabCor + floor((i_22_ - tabCar) / 64);
                                    uint8_t  valu = (vidMem[idx] & 0xf0);
                                    i_25_         = valu >> 4;
                                } break;
                                case 2: {
                                    uint8_t valu = (vidMem[tabCor + i_22_ - tabCar] & 0xf0);
                                    i_25_        = valu >> 4;
                                } break;
                            }
                        } else {
                            switch (screenAtual) {
                                case 0:
                                    i_25_ = registros[7] & 0xf;
                                    break;
                                case 1: {
                                    uint16_t idx = tabCor + floor((i_22_ - tabCar) / 64);
                                    i_25_        = (vidMem[idx] & 0xf);
                                } break;
                                case 2:
                                    i_25_ = vidMem[tabCor + i_22_ - tabCar] & 0xf;
                                    break;
                            }
                        }
                        imagemTela[i_24_] = i_25_;

                        // imagedata.data[i_24_ * 4 + 0] = palette[i_25_][0];    // r
                        // imagedata.data[i_24_ * 4 + 1] = palette[i_25_][1];    // g
                        // imagedata.data[i_24_ * 4 + 2] = palette[i_25_][2];    // b
                    }
                }
            }
            i_17_                 = dirtyVidMem[primeiro];
            dirtyVidMem[primeiro] = -1;
        }
    }
    void escrevePortaComandos(uint16_t i)
    {
        int i_27_ = tabCor;
        int i_28_ = tabNome;
        int i_29_ = tabCar;
        int i_30_ = screenAtual;
        int i_31_ = registros[7];

        if (!lidoByte) {
            byteLido = i;
            lidoByte ^= true;
        } else {
            switch ((i & 0xc0) >> 6) {
                case 0:
                    regEnd       = (i & 0x3f) * 256 + byteLido;
                    ByteReadBuff = vidMem[regEnd++];
                    regEnd %= 16384;
                    break;
                case 1:
                    regEnd = (i & 0x3f) * 256 + byteLido;
                    break;
                case 2:
                    registros[i & 0x7] = byteLido;
                    switch (i & 0x7) {
                        case 0:
                            if ((byteLido & 0x2) == 2) {
                                if ((registros[1] & 0x18) >> 3 == 0)
                                    screenAtual = 2;
                            } else {
                                switch ((registros[1] & 0x18) >> 3) {
                                    case 0:
                                        screenAtual = 1;
                                        break;
                                    case 1:
                                        screenAtual = 3;
                                        break;
                                    case 2:
                                        screenAtual = 0;
                                        break;
                                }
                            }
                            if (screenAtual == 2) {
                                tabCor = (registros[3] & 0x80) << 6;
                                tabCar = (registros[4] & 0x4) << 11;
                            } else {
                                tabCor = registros[3] << 6;
                                tabCar = (registros[4] & 0x7) << 11;
                            }
                            tabNome   = (registros[2] & 0xf) << 10;
                            tabAtrSpt = (registros[5] & 0x7f) << 7;
                            tabImgSpt = (registros[6] & 0x7) << 11;
                            break;
                        case 1:
                            if ((registros[0] & 0x2) == 2) {
                                if ((byteLido & 0x18) >> 3 == 0)
                                    screenAtual = 2;
                            } else {
                                switch ((byteLido & 0x18) >> 3) {
                                    case 0:
                                        screenAtual = 1;
                                        break;
                                    case 1:
                                        screenAtual = 3;
                                        break;
                                    case 2:
                                        screenAtual = 0;
                                        break;
                                }
                            }
                            if (screenAtual == 2) {
                                tabCor = (registros[3] & 0x80) << 6;
                                tabCar = (registros[4] & 0x4) << 11;
                            } else {
                                tabCor = registros[3] << 6;
                                tabCar = (registros[4] & 0x7) << 11;
                            }
                            tabNome   = (registros[2] & 0xf) << 10;
                            tabAtrSpt = (registros[5] & 0x7f) << 7;
                            tabImgSpt = (registros[6] & 0x7) << 11;
                            break;
                        case 2:
                            tabNome = (registros[2] & 0xf) << 10;
                            break;
                        case 3:
                            if (screenAtual == 2)
                                tabCor = (registros[3] & 0x80) << 6;
                            else
                                tabCor = registros[3] << 6;
                            break;
                        case 4:
                            if (screenAtual == 2)
                                tabCar = (registros[4] & 0x4) << 11;
                            else
                                tabCar = (registros[4] & 0x7) << 11;
                            break;
                        case 5:
                            tabAtrSpt = (registros[5] & 0x7f) << 7;
                            break;
                        case 6:
                            tabImgSpt = (registros[6] & 0x7) << 11;
                            break;
                    }
                    break;
            }
            lidoByte ^= true;
        }
        if (i_27_ != tabCor || i_28_ != tabNome || i_29_ != tabCar || screenAtual != i_30_ || i_31_ != registros[7])
            updateWholeScreen = true;
    }
    void escrevePortaDados(uint16_t i)
    {
        bool flg = vidMem[regEnd] != i;
        int  i_32_;
        int  i_33_;
        int  i_34_;
        int  i_37_;
        int  i_38_;

        vidMem[regEnd++] = i;
        regEnd %= 16384;
        if (flg) {
            i_32_ = screenAtual == 2 ? 6144 : 2048;
            i_33_ = screenAtual == 0 ? 960 : 768;
            i_34_ = screenAtual == 2 ? 6144 : 32;
            if (regEnd > tabCar && regEnd <= tabCar + i_32_) {
                int val = floor((regEnd - tabCar - 1) / 8);
                i_37_   = val % 256;

                for (i_38_ = tabNome; i_38_ < tabNome + i_33_; i_38_++) {
                    if (vidMem[i_38_] == i_37_) {
                        if (primeiro == -1)
                            primeiro = ultimo = i_38_ - tabNome;
                        else if (dirtyVidMem[i_38_ - tabNome] == -1) {
                            dirtyVidMem[ultimo] = i_38_ - tabNome;
                            ultimo              = i_38_ - tabNome;
                        }
                    }
                }
            }
            if (regEnd > tabNome && regEnd <= tabNome + i_33_) {
                if (primeiro == -1)
                    primeiro = ultimo = regEnd - tabNome - 1;
                else if (dirtyVidMem[regEnd - tabNome - 1] == -1) {
                    dirtyVidMem[ultimo] = regEnd - tabNome - 1;
                    ultimo              = regEnd - tabNome - 1;
                }
            }
            if (regEnd > tabCor && regEnd <= tabCor + i_34_ && screenAtual != 0) {
                switch (screenAtual) {
                    case 1:
                        i_37_ = (regEnd - tabCor - 1) * 8;
                        for (i_38_ = tabNome; i_38_ < tabNome + 768; i_38_++) {
                            if (vidMem[i_38_] >= i_37_ && vidMem[i_38_] < i_37_ + 8) {
                                if (primeiro == -1) {
                                    primeiro = ultimo = i_38_ - tabNome;
                                } else if (dirtyVidMem[i_38_ - tabNome] == -1) {
                                    dirtyVidMem[ultimo] = i_38_ - tabNome;
                                    ultimo              = i_38_ - tabNome;
                                }
                            }
                        }
                        break;
                    case 2:
                        i_32_ = floor((regEnd - tabCor - 1) / 8);
                        i_33_ = floor(i_32_ / 256);
                        i_32_ %= 256;
                        i_34_ = tabNome + i_33_ * 256;
                        for (i_38_ = i_34_; i_38_ < i_34_ + 256; i_38_++) {
                            if (vidMem[i_38_] == i_32_) {
                                if (primeiro == -1) {
                                    primeiro = ultimo = i_38_ - tabNome;
                                } else if (dirtyVidMem[i_38_ - tabNome] == -1) {
                                    dirtyVidMem[ultimo] = i_38_ - tabNome;
                                    ultimo              = i_38_ - tabNome;
                                }
                            }
                        }
                        break;
                }
            }
        }
    }
    int lePortaComandos()
    {
        int i     = regStatus;
        regStatus = 0;
        return i;
    }
    int lePortaDados()
    {
        int i        = ByteReadBuff;
        ByteReadBuff = vidMem[regEnd++];
        regEnd %= 16384;
        return i;
    }
    void montaSprites()
    {
        int i     = (registros[1] & 0x2) > 0 ? 4 : 1;
        int i_45_ = (registros[1] & 0x2) > 0 ? 16 : 8;
        int i_46_ = 0;
        int i_47_;
        int i_48_;
        int i_49_;
        int i_50_;
        int i_51_;
        int i_52_;
        int i_53_;

        for (i_46_ = tabAtrSpt; vidMem[i_46_] != 208 && i_46_ < tabAtrSpt + 128; i_46_ += 4) {
        }
        for (i_46_ -= 4; i_46_ >= tabAtrSpt; i_46_ -= 4) {
            i_47_ = (vidMem[i_46_] + 1) >> 3;
            i_47_ = i_47_ * 32 + (vidMem[i_46_ + 1] >> 3);
            if (i_47_ < 768) {
                if (primeiro == -1) {
                    primeiro = ultimo = i_47_;
                    if (dirtyVidMem[i_47_] == -1) {
                        dirtyVidMem[ultimo] = i_47_;
                        ultimo              = i_47_;
                    }
                    if (dirtyVidMem[i_47_ + 1] == -1 && i_47_ + 1 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 1;
                        ultimo              = i_47_ + 1;
                    }
                    if (dirtyVidMem[i_47_ + 32] == -1 && i_47_ + 32 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 32;
                        ultimo              = i_47_ + 32;
                    }
                    if (dirtyVidMem[i_47_ + 33] == -1 && i_47_ + 33 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 33;
                        ultimo              = i_47_ + 33;
                    }
                    if (dirtyVidMem[i_47_ + 2] == -1 && i_47_ + 2 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 2;
                        ultimo              = i_47_ + 2;
                    }
                    if (dirtyVidMem[i_47_ + 34] == -1 && i_47_ + 34 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 34;
                        ultimo              = i_47_ + 34;
                    }
                    if (dirtyVidMem[i_47_ + 64] == -1 && i_47_ + 64 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 64;
                        ultimo              = i_47_ + 64;
                    }
                    if (dirtyVidMem[i_47_ + 65] == -1 && i_47_ + 65 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 65;
                        ultimo              = i_47_ + 65;
                    }
                    if (dirtyVidMem[i_47_ + 66] == -1 && i_47_ + 66 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 66;
                        ultimo              = i_47_ + 66;
                    }
                } else {
                    if (dirtyVidMem[i_47_] == -1) {
                        dirtyVidMem[ultimo] = i_47_;
                        ultimo              = i_47_;
                    }
                    if (dirtyVidMem[i_47_ + 1] == -1 && i_47_ + 1 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 1;
                        ultimo              = i_47_ + 1;
                    }
                    if (dirtyVidMem[i_47_ + 32] == -1 && i_47_ + 32 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 32;
                        ultimo              = i_47_ + 32;
                    }
                    if (dirtyVidMem[i_47_ + 33] == -1 && i_47_ + 33 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 33;
                        ultimo              = i_47_ + 33;
                    }
                    if (dirtyVidMem[i_47_ + 2] == -1 && i_47_ + 2 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 2;
                        ultimo              = i_47_ + 2;
                    }
                    if (dirtyVidMem[i_47_ + 34] == -1 && i_47_ + 34 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 34;
                        ultimo              = i_47_ + 34;
                    }
                    if (dirtyVidMem[i_47_ + 64] == -1 && i_47_ + 64 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 64;
                        ultimo              = i_47_ + 64;
                    }
                    if (dirtyVidMem[i_47_ + 65] == -1 && i_47_ + 65 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 65;
                        ultimo              = i_47_ + 65;
                    }
                    if (dirtyVidMem[i_47_ + 66] == -1 && i_47_ + 66 < 768) {
                        dirtyVidMem[ultimo] = i_47_ + 66;
                        ultimo              = i_47_ + 66;
                    }
                }
            }
            for (i_48_ = 0; i_48_ < i; i_48_++) {
                i_49_ = tabImgSpt + vidMem[i_46_ + 2] * 8 + 8 * i_48_;
                i_50_ = 0;
                switch (i_48_) {
                    case 0:
                        i_50_ = vidMem[i_46_ + 1] + (vidMem[i_46_] + 1) * 256;
                        break;
                    case 1:
                        i_50_ = vidMem[i_46_ + 1] + (vidMem[i_46_] + 9) * 256;
                        break;
                    case 2:
                        i_50_ = vidMem[i_46_ + 1] + 8 + (vidMem[i_46_] + 1) * 256;
                        break;
                    case 3:
                        i_50_ = vidMem[i_46_ + 1] + 8 + (vidMem[i_46_] + 9) * 256;
                        break;
                }
                if (i_50_ >= 0 && i_50_ < 47104) {
                    i_51_ = vidMem[i_46_ + 3] & 0xf;
                    for (i_52_ = i_49_; i_52_ < i_49_ + 8; i_52_++) {
                        for (i_53_ = 0; i_53_ < 8; i_53_++) {
                            if ((vidMem[i_52_] & (1 << (7 - i_53_))) > 0) {
                                imagemTela[i_50_ + i_53_ + ((i_52_ - i_49_) << 8)] = i_51_;

                                // imagedata.data[(i_50_ + i_53_ + (i_52_ - i_49_ << 8)) * 4 + 0] = palette[i_51_][0];
                                // // r imagedata.data[(i_50_ + i_53_ + (i_52_ - i_49_ << 8)) * 4 + 1] =
                                // palette[i_51_][1];
                                //
                                // g imagedata.data[(i_50_ + i_53_ + (i_52_ - i_49_ << 8)) * 4 + 2] = palette[i_51_][2];
                                //
                                // b
                            }
                        }
                    }
                }
            }
        }
    }
    void montaUsandoMemoria()
    {
        if ((registros[1] & 0x40) > 0) {
            if (updateWholeScreen)
                atualizaTudo();
            else
                desenhaOtimizado();
            if (screenAtual != 0)
                montaSprites();
            updateWholeScreen = false;
        }
        regStatus |= 0x80;
        updateScreen();
    }
};
#endif