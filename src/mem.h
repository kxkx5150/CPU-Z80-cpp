#ifndef _H_MEM_
#define _H_MEM_
#include <string>
#include <vector>

class PC;
class Mem {
  public:
    PC *pc = nullptr;

    int *memReadMap[4]{};
    int *rom[32]{};

    bool megarom       = false;
    int  pagMegaRom[4] = {0, 1, 2, 3};
    int  tipoMegarom   = 0;
    int  cartSlot      = 0;

    int PPIPortA = 0;
    int PPIPortC = 0;
    int PPIPortD = 0;

    bool podeEscrever[4] = {false, false, false, true};

  public:
    Mem(PC *pc);
    ~Mem();

    void load(std::vector<int> &bin, int idx, int offset, int size);

    void reset();
    int  readMem(int address);
    int  readMemWord(int address);
    void writeMem(int address, int value);
    void writeMemWord(int address, int value);
    void preparaMemoriaMegarom(std::string address);
};

#endif