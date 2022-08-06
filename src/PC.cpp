#include "PC.h"
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>

PC::PC()
{
    mem      = new Mem(this);
    cpu      = new z80(this, mem);
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
    mem->load(cbios_main_msx1_rom, 0, 0, 32768);
    mem->load(msx_game_rom, 2, 16384, 49152);
}
void PC::start()
{
    vdp->reset();
}
void PC::run_cpu()
{
    cpu->run();
    vdp->montaUsandoMemoria();
}
