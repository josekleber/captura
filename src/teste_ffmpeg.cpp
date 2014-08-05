#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <boost/thread/thread.hpp>
#include <fstream>
#include <configuration.h>

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

#define NELSON

#ifdef NELSON
#include "threadpool.h"
#include "database.h"

int Testes::ffmpeg_teste(string radio1, string radio2)
{
    vector<Filter> Filters;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    Configuration* config = new Configuration();

    Filter::FilterLoad(&Filters, config->FilterArqName);

    //Database* data = new Database(config->ConnectionString);
    //vector<UrlStream*> urls = data->getRadiosActive(config->Listener);

    ThreadPool* objThreadPool = new ThreadPool;
    objThreadPool->Filters = &Filters;
    objThreadPool->ipRecognition = config->mrIP;
    objThreadPool->portRecognition = config->mrPort;
    objThreadPool->sqlConnString = config->ConnectionString;
    objThreadPool->cutFolder = config->cutFolder;

//    for(int i = 0; i > urls.size(); i++)
//        objThreadPool->addThreads(urls[i]->url, urls[i]->radio);
    objThreadPool->addThreads(radio1, 1);

    while (true);

    return 0;
}
#endif // NELSON

#ifdef JEAN
int Testes::ffmpeg_teste(string teste1, string teste2)
{
    string url;

    pthread_mutex_t mutex_acesso;
    pthread_mutex_init(&mutex_acesso, NULL);
    vector<Filter> Filters;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    Configuration* config = new Configuration();

    Filter::FilterLoad(&Filters, config->FilterArqName);

    Database* data = new Database(config->ConnectionString);
    vector<UrlStream*> urls =data->getRadiosActive(config->Listener);

    for(int i = 0; i > url.size(); i++)
    {

        cout << urls[i]->url << endl;
//        boost::thread* objThreadRadio;
//        boost::thread* objThreadParser;
//
//        Parser* objParser = new Parser;
//        objParser->mutex_acesso = &mutex_acesso;
//        objParser->Filters = Filters;
//
//        StreamRadio* objRadio = new StreamRadio;
//
//        objRadio->open(url);
//
//        objParser->SetStreamRadio(objRadio);
//        objParser->CreateContext("eu.wav", true, NULL);
//
//        objThreadRadio = new boost::thread(boost::bind(&StreamRadio::read, objRadio));
//        sleep(2);
//        while (objRadio->getFifoSize() == 0)
//        {
//            cout << objRadio->getFifoSize() << endl;
//            sleep(2);
//        };
//        objThreadParser = new boost::thread(boost::bind(&Parser::ProcessFrames, objParser));
    }

    while (true);
}
#endif // JEAN
