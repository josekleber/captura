#include "testes.h"

int main(int argc, char **argv)
{
    Testes* objTestes = new Testes();

    string arqNameIn, arqNameOut;

    arqNameIn = argv[1];
    arqNameOut = argv[2];

    objTestes->ffmpeg_teste(arqNameIn, arqNameOut);

    return 0;
}
