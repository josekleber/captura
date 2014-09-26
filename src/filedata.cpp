#include "filedata.h"

FileData::FileData() : Parser()
{
    this->audioFormat = AUDIOFORMAT::arq;

    this->setBitRate(24000);
    this->setChannels(1);
    this->setSampleRate(11025);
}

FileData::FileData(string fileName, uint64_t channelLayoutIn, int sampleRateIn,
                   int bitRateIn, AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn)
    : Parser(fileName, channelLayoutIn, sampleRateIn, bitRateIn, sampleFormatIn, nbSamplesIn, nbChannelIn)
{
    this->audioFormat = AUDIOFORMAT::arq;

    this->setBitRate(24000);
    this->setChannels(1);
    this->setSampleRate(11025);

    try
    {
        this->Config();
    }
    catch(...)
    {
        throw;
    }
}

FileData::~FileData()
{
    //dtor
}

void FileData::Execute()
{
    try
    {
        Resample();
    }
    catch(BadAllocException& err)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Error code: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
    }
    catch(FFMpegException& err)
    {
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Error code: " << *boost::get_error_info<errno_code>(err) << ANSI_COLOR_RESET;
    }

    // grava o recorte em disco
    av_write_trailer(fmt_ctx_out);
}

void FileData::EndResample()
{
    // arruma os pacotes antes de gravar
    av_interleaved_write_frame(fmt_ctx_out,&pkt_out);
}


