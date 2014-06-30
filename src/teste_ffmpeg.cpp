#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <boost/thread/thread.hpp>
#include <fstream>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/error.h>
    #include <libswresample/swresample.h>
    #include <libavutil/avassert.h>
    #include <libavutil/audio_fifo.h>
}

#include "testes.h"

#define JEAN

#ifdef NELSON
#include "streamradio.h"
#include "parser.h"

int Testes::ffmpeg_teste(string radio1, string radio2)
{
    string arqName;
    vector <uint8_t> binOutput;

    pthread_mutex_t mutex_acesso;
    pthread_mutex_init(&mutex_acesso, NULL);
    vector<Filter> Filters;
    Filters = Filter::FilterLoad("/home/nelson/Projetos/mir/mrserver/Dados/boostextdescr.txt");

    boost::thread* objThreadRadio1;
    boost::thread* objThreadParser1;

    boost::thread* objThreadRadio2;
    boost::thread* objThreadParser2;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    Parser* objParser1 = new Parser;
    objParser1->mutex_acesso = &mutex_acesso;
    objParser1->Filters = Filters;

    StreamRadio* objRadio1 = new StreamRadio;

    objRadio1->open(radio1);

    objParser1->SetStreamRadio(objRadio1);
    objParser1->CreateContext("eu.wav", true, NULL);

    Parser* objParser2 = new Parser;
    objParser2->mutex_acesso = &mutex_acesso;
    objParser2->Filters = Filters;

    StreamRadio* objRadio2 = new StreamRadio;

    objRadio2->open(radio2);

    objParser2->SetStreamRadio(objRadio2);
    objParser2->CreateContext("eu.wav", true, NULL);

    objThreadRadio1 = new boost::thread(boost::bind(&StreamRadio::read, objRadio1));
    sleep(2);
    while (objRadio1->getFifoSize() == 0)
    {
        cout << objRadio1->getFifoSize() << endl;
        sleep(2);
    };
    objThreadParser1 = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser1));

    objThreadRadio2 = new boost::thread(boost::bind(&StreamRadio::read, objRadio2));
    sleep(2);
    while (objRadio2->getFifoSize() == 0)
    {
        cout << objRadio2->getFifoSize() << endl;
        sleep(2);
    };
    objThreadParser2 = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser2));

    while (true);
}
#endif // NELSON

#ifdef JEAN
int Testes::ffmpeg_teste2()
{
    string url;

    pthread_mutex_t mutex_acesso;
    pthread_mutex_init(&mutex_acesso, NULL);
    vector<Filter> Filters;
    Filters = Filter::FilterLoad("/home/nelson/Projetos/mir/mrserver/Dados/boostextdescr.txt");

    av_register_all();
    avcodec_register_all();
    avformat_network_init();





    {
        boost::thread* objThreadRadio;
        boost::thread* objThreadParser;

        Parser* objParser = new Parser;
        objParser->mutex_acesso = &mutex_acesso;
        objParser->Filters = Filters;

        StreamRadio* objRadio = new StreamRadio;

        objRadio->open(url);

        objParser->SetStreamRadio(objRadio);
        objParser->CreateContext("eu.wav", true, NULL);

        objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
        sleep(2);
        while (objRadio->getFifoSize() == 0)
        {
            cout << objRadio->getFifoSize() << endl;
            sleep(2);
        };
        objThreadParser = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser));
    }

    while (true);
}
#endif // JEAN
