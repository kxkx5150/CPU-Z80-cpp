#ifndef _H_CPU
#define _H_CPU

#include <cstdint>
#include <vector>
#include <string>
#include "mem.h"

const int T_STATES_PERrINTERRUPT = 59659;

class PC;
class z80 {
  private:
    PC  *msx = nullptr;
    Mem *mem = nullptr;

    uint64_t count = 0;

    int rA = 0;
    int rB = 0;
    int rC = 0;
    int rD = 0;
    int rE = 0;
    int rH = 0;
    int rL = 0;

    int rAF = 0;
    int rHL = 0;
    int rBC = 0;
    int rDE = 0;
    int rIX = 0;
    int rIY = 0;
    int rID = 0;
    int rSP = 0;
    int rPC = 0;
    int rI  = 0;
    int rR  = 0;
    int rR7 = 0;
    int rIM = 2;

    bool fS   = false;
    bool fZ   = false;
    bool f5   = false;
    bool fH   = false;
    bool f3   = false;
    bool fPV  = false;
    bool fN   = false;
    bool fC   = false;
    bool IFF1 = true;
    bool IFF2 = true;

    int parity[256];
    int portos[256];

    bool showpc = false;

    bool                     logcheck        = false;
    bool                     stepinfo        = false;
    std::string              filename        = "log0.txt";
    int                      filecheck_start = 0;
    int                      filecheck_end   = 1000;
    int                      fileoffset      = 0;
    std::vector<std::string> lines;

  public:
    z80(PC *_pc, Mem *_mem);
    ~z80();

    void run();
    void reset();
    int  z80_interrupt();
    void dump(int opcode, int i);
    int  file_read();

    void   setF(int i);
    int8_t bytef(int i);
    void   setBC(int i);
    int    HL();
    int    BC();
    int    add16(int i, int i_9_);
    void   setHL(int i);
    void   setDE(int i);
    int    DE();
    int    inc8(int i);
    int    dec8(int i);
    void   sub_a(int i);
    void   add_a(int i);
    void   adc_a(int i);
    void   sbc_a(int i);
    void   and_a(int i);
    void   xor_a(int i);
    void   or_a(int i);
    void   cp_a(int i);

    void poppc();
    int  popw();
    void pushpc();
    void pushw(int i);

    int  rlc(int i);
    int  rrc(int i);
    int  rl(int i);
    int  rr(int i);
    int  sla(int i);
    int  sra(int i);
    int  sls(int i);
    int  srl(int i);
    void bit(int i, int i_16_);
    int  res(int i, int i_71_);
    int  set(int i, int i_86_);

    void outb(int i, int i_19_);
    int  inb(int i);
    int  in_bc();
    int  sbc16(int i, int i_78_);
    int  adc16(int i, int i_1_);

    void setIFF1(bool flg);
    void setIFF2(bool flg);
    void setIDH(int i);
    void setIDL(int i);
    int  ID_d();

    int nxtpcb();

  private:
};
#endif
