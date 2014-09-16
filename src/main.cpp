#include <iostream>
#include <mir/filter.h>

#include "main.h"

using namespace std;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

// objeto do banco de dados
Database* db = NULL;
// configurações gerais da aplicação
Configuration* config = NULL;
// lista de streams
vector<UrlStream*> urlStream;


/** \brief Armazena, em arquivo, as informações dos streams utilizados pelo listener atual.
*/
void writeFileStream()
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
int readFileStream()
{
    ifstream file(config->StreamList);

    std::string line;
    int lineCount = 0;

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
                BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "ID do listener no arquivo com lista de streams diverge da configuração atual do sistema." ANSI_COLOR_RESET;
                return -1;
            }

        }
        else if (lineCount == 1)
            BOOST_LOG_TRIVIAL(info) << "ultima atualização do arquivo com a lista de streams: " << line.c_str();
        else
        {
            // verifica se é a linha de total
            if (line.find("Total",0) != std::string::npos)
                break;

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

        lineCount++;
    }

    file.close();

    return 0;
}

/** \brief Carrega a lista de streams.
    Iniciamente tenta carregar a lista de rádios apartir do banco de dados,
    se não obtiver sucesso, procura pelo arquivo com as urls salvo localmente.
*/
int loadStream()
{

    try
    {
        // carrega do banco de dados a lista de streams
        urlStream = db->getRadiosActive(config->Listener);

        // atualiza a lista de rádios
        writeFileStream();
    }
    catch(...)
    {
        //TODO implementar log
        //TODO implementar tratamento para diferentes exceções
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "Erro ao conectar a base de dados." ANSI_COLOR_RESET;

        // tenta carregar a lista de streams do arquivo local
        fs::path p (config->StreamList);

        if (!fs::exists(p))
        {
            //TODO implementar log
            BOOST_LOG_TRIVIAL(fatal) << "Arquivo com lista de streams local não foi localizado.";
            return -1;
        }
        else
        {
            readFileStream();
        }
    }
    return 0;
}


void init()
{
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::debug
    );
}


/** \brief Sistema de Captura de Audios
    Captura através de streams web.
*/
int main()
{
    printf("\033[0m \033[2J\033[1;1H");
    BOOST_LOG_TRIVIAL(info) <<  "Captura Version: " << prgVersion;

    init();

    // retorno de métodos
    int ret = 0;

    // filtro utilizado pelo fingerprint
    vector<Filter> Filters;

    // registra os componentes do FFMPEG
    av_register_all();
    avcodec_register_all();
    ret = avformat_network_init();

    if (ret < 0)
    {
        //TODO implementar log
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "falha ao iniciar componente de rede do FFMPEG." ANSI_COLOR_RESET;
        //TODO criar códigos de erro
        return -1;
    }
    else
        BOOST_LOG_TRIVIAL(debug) << "carregado FFMPEG.";

    // carrega configurações da captura
    config = new Configuration();

    // instancia objeto do banco de dados
    db = new Database(config->ConnectionStringSQL);

    // carrega filtro utilizado pelo fingerprint
    ret = Filter::FilterLoad(&Filters, config->FilterArqName);

    if (ret < 0)
    {
        //TODO implementar log
        BOOST_LOG_TRIVIAL(error) << ANSI_COLOR_RED "falha ao carregar filtro do fingerprint." ANSI_COLOR_RESET;
        //TODO criar códigos de erro
        return -1;
    }

    ThreadPool* objThreadPool = new ThreadPool;
    objThreadPool->Filters = &Filters;
    objThreadPool->ipRecognition = config->mrIP;
    objThreadPool->portRecognition = config->mrPort;
    objThreadPool->sqlConnString = config->ConnectionStringSQL;
    objThreadPool->cutFolder = config->cutFolder;

    // laço principal da aplicação.
    // dentro deste laço :
    //  - carrrega lista de rádios;
    //  - mantém arquivo de rádios;
    //  - instancia threads para rádio ou mata a mesma.
    do
    {
        // carrega a lista de rádios
        ret = loadStream();

        if (ret < 0)
        {
            //TODO implementar log
            BOOST_LOG_TRIVIAL(error) << "falha ao carregar lista de streams.\nSistema tentará novamente em " << config->UpdateTimer << " minutos.\n";
        }
        else
        {
            for (unsigned int idxRadio = 0; idxRadio < urlStream.size(); idxRadio++)
            {
                try
                {
                    // cria as threads
                    BOOST_LOG_TRIVIAL(info) << "Conectando em " << urlStream[idxRadio]->radio
                                << " - " << urlStream[idxRadio]->url.c_str();

                    string urlRadio = objThreadPool->getUrlRadio(urlStream[idxRadio]->radio);
                    if (urlRadio == "")
                        objThreadPool->addThreads(urlStream[idxRadio]->url, urlStream[idxRadio]->radio);
                    else if (urlRadio != urlStream[idxRadio]->url)
                    {
                        // se a url da rádio for diferente da url atual na thread
                        objThreadPool->stopThread(urlStream[idxRadio]->radio);
                        objThreadPool->addThreads(urlStream[idxRadio]->url, urlStream[idxRadio]->radio);
                    }

                     BOOST_LOG_TRIVIAL(info) << "Quantidade de Threads em funcionamento " << objThreadPool->getActiveThread().size();
                }
                catch(...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Erro ao capturar a rádio"
                    << urlStream[idxRadio]->radio << " : " << urlStream[idxRadio]->url.c_str();
                }
            }
        }

        BOOST_LOG_TRIVIAL(debug) << "Iniciando sleep do loop principal." << __DATE__  ":" << __TIME__;

        // sempre haverá um sleep para verificar novas rádios
        boost::this_thread::sleep(boost::posix_time::minutes(config->UpdateTimer));
    }
    while (true);
}
