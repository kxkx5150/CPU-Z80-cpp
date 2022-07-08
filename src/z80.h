#ifndef _H_CPU
#define _H_CPU

#include <cstdint>
#include <vector>
#include <string>

using namespace std;
const int T_STATES_PER_INTERRUPT = 59659;

class PC;
class z80 {
  private:
    PC      *msx   = nullptr;
    uint64_t count = 0;

    uint8_t *memReadMap[4]{};
    uint8_t *rom[32]{};

    bool megarom       = false;
    int  pagMegaRom[4] = {0, 1, 2, 3};
    int  tipoMegarom   = 0;
    int  cartSlot      = 0;

    uint8_t _A = 0;
    uint8_t _B = 0;
    uint8_t _C = 0;
    uint8_t _D = 0;
    uint8_t _E = 0;
    uint8_t _H = 0;
    uint8_t _L = 0;

    uint16_t _AF_ = 0;
    uint16_t _HL_ = 0;
    uint16_t _BC_ = 0;
    uint16_t _DE_ = 0;
    uint16_t _IX  = 0;
    uint16_t _IY  = 0;
    uint16_t _ID  = 0;
    uint16_t _SP  = 0;
    uint16_t _PC  = 0;
    uint8_t  _I   = 0;
    uint8_t  _R   = 0;
    uint8_t  _R7  = 0;
    uint8_t  _IM  = 2;

    bool fS    = false;
    bool fZ    = false;
    bool f5    = false;
    bool fH    = false;
    bool f3    = false;
    bool fPV   = false;
    bool fN    = false;
    bool fC    = false;
    bool _IFF1 = true;
    bool _IFF2 = true;

    uint8_t PPIPortA = 0;
    // uint8_t PPIPortB = 255;
    uint8_t PPIPortC = 0;
    uint8_t PPIPortD = 0;

    uint8_t parity[256];
    int     portos[256];

    bool podeEscrever[4] = {false, false, false, true};
    bool showpc          = false;

    bool           logcheck        = false;
    bool           stepinfo        = false;
    string         filename        = "log0.txt";
    int            filecheck_start = 0;
    int            filecheck_end   = 1000;
    int            fileoffset      = 0;
    vector<string> lines;

  public:
    z80(PC *_pc);
    ~z80();

    void    run();
    void    reset();
    void    load(std::vector<uint8_t> &bin, int idx, int offset, int size);
    uint8_t z80_interrupt();
    void    dump(int opcode);
    int     file_read();

    void     setF(uint8_t i);
    int8_t   bytef(uint8_t i);
    void     setBC(uint16_t i);
    uint16_t HL();
    uint16_t BC();
    uint16_t add16(uint16_t i, uint16_t i_9_);
    void     setHL(uint16_t i);
    void     setDE(uint16_t i);
    uint16_t DE();
    uint8_t  inc8(uint8_t i);
    uint8_t  dec8(uint8_t i);
    void     sub_a(uint8_t i);
    void     add_a(uint8_t i);
    void     adc_a(uint8_t i);
    void     sbc_a(uint8_t i);
    void     and_a(uint8_t i);
    void     xor_a(uint8_t i);
    void     or_a(uint8_t i);
    void     cp_a(uint8_t i);

    void     poppc();
    uint16_t popw();
    void     pushpc();
    void     pushw(uint16_t i);

    uint8_t rlc(uint8_t i);
    uint8_t rrc(uint8_t i);
    uint8_t rl(uint8_t i);
    uint8_t rr(uint8_t i);
    uint8_t sla(uint8_t i);
    uint8_t sra(uint8_t i);
    uint8_t sls(uint8_t i);
    uint8_t srl(uint8_t i);
    void    bit(uint8_t i, uint8_t i_16_);
    uint8_t res(uint8_t i, uint8_t i_71_);
    uint8_t set(uint8_t i, uint8_t i_86_);

    void     outb(uint8_t i, uint8_t i_19_);
    uint8_t  inb(uint8_t i);
    uint8_t  in_bc();
    uint16_t sbc16(uint16_t i, uint16_t i_78_);
    uint16_t adc16(uint16_t i, uint16_t i_1_);

    void     setIFF1(bool flg);
    void     setIFF2(bool flg);
    void     setIDH(uint16_t i);
    void     setIDL(uint16_t i);
    uint16_t ID_d();

    uint16_t nxtpcb();

    uint8_t  readMem(uint16_t address);
    uint16_t readMemWord(uint16_t address);
    void     writeMem(uint16_t address, uint8_t value);
    void     writeMemWord(uint32_t address, uint16_t value);

    void preparaMemoriaMegarom(string address);

  private:
};
#endif
