#ifndef _H_KBD
#define _H_KBD

class KBD {

  public:
    int state[100] = {};

    KBD()
    {
        for (int i = 0; i < 100; i++) {
            state[i] = 255;
        }
    }

    void keydown(int i)
    {
    }

    void keyup(int i)
    {
    }
};
#endif