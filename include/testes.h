#ifndef TESTES_H
#define TESTES_H

#include "streamradio.h"
#include "threadpool.h"
#include "parser.h"
#include "database.h"
#include "configuration.h"

class Testes
{
    public:
        Testes();
        virtual ~Testes();

        int Threadpool();
        int ffmpeg_teste(string arqNameIn);
        int ffmpeg_teste2();

    protected:
    private:
};

#endif // TESTES_H
