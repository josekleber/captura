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
clock_t start = clock();

    if (idRadio == 0)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Radio sem id\n");
        return;
    }

    try
    {
        Resample();
    }
    catch(SignalException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): %s\n", err.what());
        return;
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
    try
    {
        av_write_trailer(fmt_ctx_out);
    }
    catch(SignalException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "Error code (File): Erro de segmentacao no fechamento do arquivo\n");
        return;
    }
    catch(...)
    {
        objLog->mr_printf(MR_LOG_ERROR, idRadio, "filedata (Execute) : Erro Desconhecido\n");
        return;
    }

objLog->mr_printf(MR_LOG_DEBUG, idRadio, "ArqData >>> Tempo de processamento : %8.4f    Name : %s\n",
                  (float)(clock() - start)/CLOCKS_PER_SEC, fileName.c_str());
}

void FileData::EndResample()
{
    // arruma os pacotes antes de gravar
    try
    {
        av_interleaved_write_frame(fmt_ctx_out, &pkt_out);
    }
    catch(SignalException& err)
    {
        throw ExceptionClass("filedata", "EndResample", "Erro de Segmentacao");
    }
    catch(...)
    {
        throw ExceptionClass("filedata", "EndResample", "Erro desconhecido");
    }
}
