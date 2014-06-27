#include "testes.h"

int Testes::Threadpool()
{
    ThreadPool *objThread = new ThreadPool;

    objThread->TesteThread(5);

    return 0;
}

