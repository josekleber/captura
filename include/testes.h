#ifndef TESTES_H
#define TESTES_H

#include "streamradio.h"
#include "threadpool.h"

class Testes
{
    public:
        Testes();
        virtual ~Testes();

        int Threadpool();
        int ffmpeg_teste(string arqNameIn, string arqNameOut);

    protected:
    private:
};

#endif // TESTES_H
