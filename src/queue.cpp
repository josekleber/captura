#include "queue.h"

Queue::Queue(AVCodecContext* codecContext, AVFormatContext* formatContext)
{
    this->nbChannel = codecContext->channels;
    this->FrameSize = codecContext->frame_size;
    this->nbBuffers = av_sample_fmt_is_planar(codecContext->sample_fmt) ? this->nbChannel : 1;
    av_samples_get_buffer_size(&this->bufSize, this->nbChannel, 1, codecContext->sample_fmt, 1);
    this->SampleSize = bufSize;
}

Queue::~Queue()
{
    //dtor
}

int Queue::getQueueSize()
{
    int ret;
    mtx_.lock();
    ret = queueData.size() * FrameSize;
    mtx_.unlock();
    return ret;
}

void Queue::addQueueData(uint8_t **inputSamples, const int frameSize)
{
    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    mtx_.lock();

    vector<vector <uint8_t>> buf1;
    for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
    {
        vector <uint8_t> buf2;
        for (int data = 0; data < frameSize; data++)
            for (int qntByte = 0; qntByte < bufSize; qntByte++)
                buf2.push_back(inputSamples[numBuf][data * bufSize + qntByte]);
        buf1.push_back(buf2);
    }

    queueData.push(buf1);

    for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
        buf1[numBuf].clear();
    buf1.clear();

    mtx_.unlock();
}

//int Queue::getQueueData(uint8_t **dest, int nbSamples)
vector<vector<uint8_t>> Queue::getQueueData()
{
    vector<vector<uint8_t>> ret;
    mtx_.lock();
    if (queueData.size() > 0)
    {
        ret = queueData.front();
//        vector <vector <uint8_t>> buf1 = queueData.front();
        queueData.pop();
//        ret = buf1.size();
//        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
//            memcpy(dest[numBuf], buf1[numBuf].data(), nbSamples * bufSize);
//        for (int numBuf = 0; numBuf < nbBuffers; numBuf++)
//            buf1[numBuf].clear();
//        buf1.clear();
    }
    mtx_.unlock();

    return ret;
}

int Queue::getChannelSize()
{
    return FrameSize * bufSize;
}
