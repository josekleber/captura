#include "filedata.h"

FileData::FileData() : Parser()
{
    this->audioFormat = AUDIOFORMAT::arq;

    this->setBitRate(24000);
    this->setChannels(1);
    this->setSampleRate(11025);
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
    catch(ResampleException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(FifoException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(BadAllocException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }
    catch(FFMpegException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): %d\n", *boost::get_error_info<errno_code>(err));
        return;
    }

    // grava o recorte em disco
    av_write_trailer(fmt_ctx_out);
}

void FileData::EndResample()
{
    // arruma os pacotes antes de gravar
    av_interleaved_write_frame(fmt_ctx_out,&pkt_out);
}
