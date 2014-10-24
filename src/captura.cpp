#include "captura.h"

using namespace std;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

Captura::~Captura()
{

}

Captura::Captura()
{
    printf("\033[0m \033[2J\033[1;1H");
    cout <<  "Captura Version: " << prgVersion << endl << endl;

    try
    {
        init();

        objThreadPool = new ThreadPool(config->mrOn, config->svFP, config->mrIP, config->mrPort,
                                       config->ConnectionStringMySQL, config->cutFolder, &Filters);
    }
    catch(SignalException& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, 0, "captura (contructor) : Erro de segmentacao");
        return;
    }
    catch(ExceptionClass& err)
    {
        objLog->mr_printf(MR_LOG_ERROR, 0, err.what());
        return;
    }
    catch(...)
    {
        objLog->mr_printf(MR_LOG_ERROR, 0, "captura (contructor) : Erro desconhecido");
        return;
    }
}

void Captura::initLoop()
{
    // laço principal da aplicação.
    // dentro deste laço :
    //  - carrrega lista de rádios;
    //  - mantém arquivo de rádios;
    //  - instancia threads para rádio ou mata a mesma.
    do
    {
        // carrega a lista de rádios
//        int ret = loadStream();
int ret = readFileStream();

        try
        {
            if (ret < 0)
            {
                objLog->mr_printf(MR_LOG_ERROR, 0, "Falha ao carregar lista de streams. Sistema tentará novamente em %d minutos.\n", config->UpdateTimer);
            }
            else
            {
                for (unsigned int idxRadio = 0; idxRadio < urlStream.size(); idxRadio++)
                {
                    try
                    {
                        // cria as threads
                        objLog->mr_printf(MR_LOG_MESSAGE, urlStream[idxRadio]->radio, MR_LOG_BOLDYELLOW "Conectando em %s" MR_LOG_RESET "\n",
                               urlStream[idxRadio]->url.c_str());

                        string urlRadio = objThreadPool->getUrlRadio(urlStream[idxRadio]->radio);
                        if (urlRadio == "")
                          objThreadPool->addThreads(urlStream[idxRadio]->url, urlStream[idxRadio]->radio);
                        else if (urlRadio != urlStream[idxRadio]->url)
                        {
                            // se a url da rádio for diferente da url atual na thread
                            objThreadPool->stopThread(urlStream[idxRadio]->radio);
                            boost::this_thread::sleep(boost::posix_time::seconds(1));
                            objThreadPool->addThreads(urlStream[idxRadio]->url, urlStream[idxRadio]->radio);
                        }

                         objLog->mr_printf(MR_LOG_DEBUG, 0, "Quantidade de Threads em funcionamento %d\n", objThreadPool->getActiveThread().size());
                    }
                    catch (SignalException& err)
                    {
                        objLog->mr_printf(MR_LOG_ERROR, urlStream[idxRadio]->radio,
                                          "Erro de memoria : %s\n", urlStream[idxRadio]->url.c_str());
                    }
                    catch(...)
                    {
                        objLog->mr_printf(MR_LOG_ERROR, urlStream[idxRadio]->radio,
                                          "Erro ao capturar a rádio %s\n", urlStream[idxRadio]->url.c_str());
                    }
                }
            }

            objLog->mr_printf(MR_LOG_DEBUG, 0, "Iniciando sleep do loop principal.\n");

            // sempre haverá um sleep para verificar novas rádios
            boost::this_thread::sleep(boost::posix_time::minutes(config->UpdateTimer));
        }
        catch (SignalException& err)
        {
            objLog->mr_printf(MR_LOG_DEBUG, 0, "SignalException: %s\n", err.what());
        }
    }
    while (true);
}

/** \brief Armazena, em arquivo, as informações dos streams utilizados pelo listener atual.
*/
void Captura::writeFileStream()
{
    fs::path p (config->StreamList);

    ofstream file(config->StreamList);
    file << config->Listener << endl;
    file << pt::to_simple_string(pt::second_clock::local_time()) << endl;

    for(unsigned int index = 0; index < urlStream.size(); index++)
    {
        file << urlStream[index]->id << "\t";
        file << urlStream[index]->radio << "\t";
        file << urlStream[index]->url << endl;
    }
    file << "Total: " << urlStream.size() <<endl;
    file.flush();
    file.close();
}

/** \brief Lê os dados, em arquivo, com as informações dos streams.
*/
int Captura::readFileStream()
{
    ifstream file(config->StreamList);

    std::string line;
    int lineCount = 0;

    for (int i = 0; i < (int)urlStream.size(); i++)
        delete urlStream[i];
    urlStream.clear();

    while(!file.eof())
    {
        getline(file,line);
        if (lineCount == 0)
        {
            // id do listener
            if (config->Listener != line)
            {
                //TODO implementar log
                objLog->mr_printf(MR_LOG_ERROR, 0, "ID do listener no arquivo com lista de streams diverge da configuração atual do sistema.\n");
                file.close();
                return -1;
            }

        }
        else if (lineCount == 1)
            objLog->mr_printf(MR_LOG_MESSAGE, 0, "ultima atualização do arquivo com a lista de streams: %s\n", line.c_str());
        else
        {
            // verifica se é a linha de total
            if (line.find("Total",0) != std::string::npos)
                break;

            if (line.substr(0, 1) != "#")
            {
                UrlStream* us = new UrlStream();
                std::string lineTAB;
                std::istringstream linestrm(line);
                getline(linestrm,lineTAB,'\t');
                us->id = atoi(lineTAB.c_str());
                getline(linestrm,lineTAB,'\t');
                us->radio = atoi(lineTAB.c_str());
                getline(linestrm,lineTAB,'\t');
                us->url = lineTAB;

                urlStream.push_back(us);
            }
        }

        lineCount++;
    }

    file.close();

    return 0;
}

/** \brief Carrega a lista de streams.
    Iniciamente tenta carregar a lista de rádios apartir do banco de dados,
    se não obtiver sucesso, procura pelo arquivo com as urls salvo localmente.
*/
int Captura::loadStream()
{

    try
    {
        for (int i = 0; i < (int)urlStream.size(); i++)
            delete urlStream[i];
        urlStream.clear();
        // carrega do banco de dados a lista de streams
        urlStream = db->getRadiosActive(config->Listener);

        // atualiza a lista de rádios
        writeFileStream();
    }
    catch(...)
    {
        //TODO implementar log
        //TODO implementar tratamento para diferentes exceções
        objLog->mr_printf(MR_LOG_ERROR, 0, "Erro ao conectar a base de dados.\n");

        // tenta carregar a lista de streams do arquivo local
        fs::path p (config->StreamList);

        if (!fs::exists(p))
        {
            //TODO implementar log
            objLog->mr_printf(MR_LOG_CRITICAL, 0, "Arquivo com lista de streams local não foi localizado.\n");
            return -1;
        }
        else
        {
            readFileStream();
        }
    }

    return 0;
}

/* Catch Signal Handler functio */
void Captura::signal_callback_handler(int signum)
{
    char aux[50];
    sprintf(aux, "Caught signal %d", signum);
    throw SignalException(aux);
}

void Captura::init()
{
    signal(SIGSEGV, Captura::signal_callback_handler);
    signal(SIGPIPE, Captura::signal_callback_handler);

    // carrega configurações da captura
    config = new Configuration();

    // criando objeto para log
    objLog = new LogClass(config->toFile, config->toScreen, config->onMsg, config->onDebug);

    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Filters.FileName: %s\n", config->FilterArqName.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Database.ConnectionStringSQL: %s\n", config->ConnectionStringSQLProducao.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Database.ConnectionStringMYSQL: %s\n", config->ConnectionStringMySQL.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Settings.Listener: %s\n", config->Listener.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Settings.FileStream: %s\n", config->StreamList.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Settings.UpdateTimer: %d\n", config->UpdateTimer);
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "FingerPrint.Save: %s\n", (config->svFP ? "On" : "Off"));
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "MRServer: %s\n", ((config->mrOn == 1) ? "On" : "Off"));
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "MRServer.IP: %s\n", config->mrIP.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "MRServer.Port: %s\n", config->mrPort.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "AudityInfo.Folder: %s\n", config->cutFolder.c_str());
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Log.toFile: %s\n", ((config->toFile == 1) ? "On" : "Off"));
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Log.toScreen: %s\n", ((config->toScreen == 1) ? "On" : "Off"));
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Log.onMsg: %s\n", ((config->onMsg == 1) ? "On" : "Off"));
    objLog->mr_printf(MR_LOG_AUDIT, 0,
                      "Log.onDebug: %s\n\n", ((config->onDebug == 1) ? "On" : "Off"));

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::debug
    );

    // registra os componentes do FFMPEG
    av_register_all();
    avcodec_register_all();
    int ret = avformat_network_init();

    if (ret < 0)
        throw ExceptionClass("main", "init", "Falha na inicializacao do FFMPEG");
    else
        objLog->mr_printf(MR_LOG_DEBUG, 0, "carregado FFMPEG.\n");

    // define o nível de log do FFMPEG
// njn
    av_log_set_level(AV_LOG_QUIET);
//    av_log_set_level(AV_LOG_DEBUG);

    // instancia objeto do banco de dados
    db = new Database_SQL(config->ConnectionStringSQLProducao);

    // carrega filtro utilizado pelo fingerprint
    ret = Filter::FilterLoad(&Filters, config->FilterArqName);

    if (ret < 0)
    {
        //TODO criar códigos de erro
        throw ExceptionClass("main", "init", "Falha ao carregar filtro do fingerprint");
    }
}
