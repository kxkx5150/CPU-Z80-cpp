#ifndef _H_KBD
#define _H_KBD

class KBD {

  public:
    int state[9] = {};

    KBD()
    {
        for (int i = 0; i < 9; i++) {
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