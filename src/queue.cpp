#include "queue.h"

Queue::Queue(AVCodecContext* codecContext, AVFormatContext* formatContext, int idRadio)
{
    this->nbChannels = codecContext->channels;
    this->Format = codecContext->sample_fmt;
    this->isPlannar = av_sample_fmt_is_planar(codecContext->sample_fmt);
    this->szData = av_get_bytes_per_sample(codecContext->sample_fmt);
    this->szFrame = codecContext->frame_size;
    this->idRadio = idRadio;
}

Queue::~Queue()
{
    while(!queueData.empty())
        queueData.pop();
}

int Queue::getQueueSize()
{
    //boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        int ret;
        ret = queueData.size() * szFrame;
        mtx = false;
        return ret;
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("queue", "getQueueSize", "Erro de segmentacao");
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_SIZE);
    }
}

void Queue::addQueueData(AVFrame* frame)
{
//    boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;

    int nbBuffers = av_sample_fmt_is_planar((AVSampleFormat)frame->format) ? frame->channels : 1;
    int szBuffer = frame->linesize[0];

    try
    {
        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
        {
            for (int data = 0; data < szBuffer; data++)
                buf2.push_back(frame->data[numBuf][data]);
            buf1.push_back(buf2);
            if (buf2.size() > 0)
                buf2.clear();
        }

        queueData.push(buf1);

        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
            if (buf1[numBuf].size() > 0)
                buf1[numBuf].clear();

        if (buf1.size() > 0)
            buf1.clear();
        mtx = false;
    }
    catch(SignalException& err)
    {
        mtx = false;
        throw ExceptionClass("queue", "addQueueData", "Erro de segmentacao");
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_ADD);
    }
}

vector<vector<uint8_t>> Queue::getQueueData()
{
    vector<vector<uint8_t>> ret;
//    boost::lock_guard<boost::mutex> guard(mtx_);
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        if (!queueData.empty())
        {
            ret = queueData.front();

            queueData.pop();
        }

        mtx = false;
        return ret;
    }
    catch(SignalException& err)
    {
        mtx = false;
        throw ExceptionClass("queue", "getQueueData", "Erro de segmentacao");
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_GET);
    }
}

void Queue::delQueueData()
{
    while (mtx)
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    mtx = true;
    try
    {
        if (!queueData.empty())
            queueData.pop();

        mtx = false;
    }
    catch(SignalException& err)
    {
        mtx = false;
        throw ExceptionClass("queue", "delQueueData", "Erro de segmentacao");
    }
    catch(...)
    {
        mtx = false;
        throw FifoException() << errno_code(MIR_ERR_FIFO_GET);
    }
}
