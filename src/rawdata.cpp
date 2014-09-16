#include "rawdata.h"

RAWData::RAWData(string fileName, uint64_t channelLayoutIn, int sampleRateIn, int bitRateIn,
                 AVSampleFormat sampleFormatIn, int nbSamplesIn, int nbChannelIn,
                 vector<Filter> *Filters, string ipRecognition, string portRecognition,
                 int32_t idRadio, int32_t idSlice) :
                     Parser (fileName, channelLayoutIn, sampleRateIn, bitRateIn,
                             sampleFormatIn, nbSamplesIn, nbChannelIn)
{
    this->Filters = Filters;
    this->ipRecognition = ipRecognition;
    this->portRecognition = portRecognition;
    this->freq = 11025;
    this->idRadio = idRadio;
    this->idSlice = idSlice;

    this->initObject();

    this->Config();
}

RAWData::~RAWData()
{
    //dtor
}

unsigned int* RAWData::CreateFingerPrint(vector <uint8_t> Data, unsigned int* FingerPrintSize, bool mltFFT)
{
    unsigned int len = Data.size();

    uint8_t* convArray = (uint8_t*)calloc(1, len);
    for (unsigned int i = 0; i < len; i += 2)
    {
        convArray[i] = Data[i + 1] & 0xff;
        convArray[i + 1] = Data[i] & 0xff;
    }

    try
    {
        unsigned int *bits;
        FingerPrint* objFingerPrint = new FingerPrint(Filters, (short int*)convArray, len / 2,
                cdc_ctx_out->sample_rate, mltFFT,
                &bits, FingerPrintSize);
        objFingerPrint->Generate();

        free(convArray);

        return bits;
    }
    catch(FingerPrintException& err)
    {
        cout << "FingerPrint object create error : " << *boost::get_error_info<errmsg_info>(err) << endl;

        return NULL;
    }
}

void RAWData::Execute()
{
clock_t start;
start = clock();
    Resample();

    // gerando fingerprints
    unsigned int nbits;
    unsigned int* bits = CreateFingerPrint(binOutput, &nbits, true);
    binOutput.clear();

    // preparando vetor com dados para envio via socket
    uint8_t* conv;
    int pos = 0;

    int16_t arqNameSize = fileName.size();
    uint8_t* buff  = new uint8_t[4 * nbits + arqNameSize + 18];

    conv = (uint8_t*)&this->idRadio;
    for (int i = 0; i < 4; buff[pos++] = conv[i++]);

    conv = (uint8_t*)&idSlice;
    for (int i = 0; i < 4; buff[pos++] = conv[i++]);

    conv = (uint8_t*)&freq;
    for (int i = 0; i < 4; buff[pos++] = conv[i++]);

    conv = (uint8_t*)&nbits;
    for (int i = 0; i < 4; buff[pos++] = conv[i++]);

    conv = (uint8_t*)&arqNameSize;
    for (int i = 0; i < 2; buff[pos++] = conv[i++]);

    for (int i = 0; i < arqNameSize; buff[pos++] = (fileName.c_str())[i++]);

    for (int j = 0; j < nbits; j++)
    {
        conv = (uint8_t*)&bits[j];
        for (int i = 0; i < 4; buff[pos++] = conv[i++]);
    }

    // enviando dados
    try
    {
        boost::asio::io_service IO_Service;
        tcp::resolver Resolver(IO_Service);
        tcp::resolver::query Query(ipRecognition, portRecognition);
        tcp::resolver::iterator EndPointIterator = Resolver.resolve(Query);

        TCPClient* objClient = new TCPClient(IO_Service, EndPointIterator, buff, pos);

        boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &IO_Service));

        while (objClient->strResp == "")
            boost::this_thread::sleep(boost::posix_time::milliseconds(10));

        if (objClient->strResp != "Received")
            cout << "Erro de envio : " << objClient->strResp << endl;
        objClient->Close();
    }
    catch(ConvertException& e)
    {
        cerr << e.what() << endl;
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    delete[] buff;
cout << "Radio : " << this->idRadio << "    Recorte : " << idSlice << "    Tempo de processamento : " << (float)(clock() - start)/CLOCKS_PER_SEC << endl;
start = clock();
}

void RAWData::EndResample()
{
    for (int i = 0; i < pkt_out.size; i++)
        binOutput.push_back(pkt_out.data[i]);
}

void RAWData::initObject()
{
    this->audioFormat = RAWData::raw;

    this->setBitRate(64000);
    this->setChannels(1);
    this->setSampleRate(11025);
}
