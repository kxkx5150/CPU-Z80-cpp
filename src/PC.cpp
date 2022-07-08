#include "PC.h"
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>

PC::PC()
{
    cpu      = new z80(this);
    psg      = new PSG8910();
    keyboard = new KBD();
    vdp      = new TMS9918();
}
PC::~PC()
{
    delete cpu;
    delete psg;
    delete keyboard;
    delete vdp;
}
void PC::init()
{
    printf("load file\n");
    load();
}
void PC::load()
{
    cpu->load(cbios_main_msx1_rom, 0, 0, 32768);
    cpu->load(goonies_rom, 2, 16384, 49152);
}
void PC::start()
{
    vdp->reset();
}
void PC::run_cpu()
{
    cpu->run();
}
