#ifndef _H_PSG
#define _H_PSG

class PSG8910 {
    int enderecoAtual = 0;
    int registros[16] = {};

  public:
    void escrevePortaDados(int i)
    {
        registros[enderecoAtual] = i;
        if (enderecoAtual == 7)
            registros[enderecoAtual] |= 0x80;
    }

    void escrevePortaEndereco(int i)
    {
        if (i < 17)
            enderecoAtual = i;
    }

    int lePortaDados()
    {
        if (enderecoAtual != 14)
            return registros[enderecoAtual];
        return 255;
    }
};
#endif
