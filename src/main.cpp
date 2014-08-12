#include "testes.h"
#include "configuration.h"
#include "database.h"
#include "threadpool.h"
#include <mir/filter.h>

#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

//#DEFINE TESTE

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

    for(int index = 0; index < urlStream.size(); index++)
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
                cout << "ID do listener no arquivo com lista de streams diverge da configuração atual do sistema." << endl;
                return -1;
            }

        }
        else if (lineCount == 1)
            cout << "ultima atualização do arquivo com a lista de streams: " << line << endl;
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
        cout << "Erro ao conectar a base de dados.\nNão foi possível pegar a lista atualizada de URLs." << endl;

        // tenta carregar a lista de streams do arquivo local
        fs::path p (config->StreamList);

        if (!fs::exists(p))
        {
            //TODO implementar log
            cout << "Arquivo com lista de streams local não foi localizado." << endl;
            return -1;
        }
        else
        {
            readFileStream();
        }
    }
}

/** \brief Sistema de Captura de Audios
    Captura através de streams web.
*/
int main(int argc, char **argv)
{
    // retorno de métodos
    int ret = 0;

    // filtro utilizado pelo fingerprint
    vector<Filter> Filters;

#if TESTE


    Testes* objTestes = new Testes();

    string radio1, radio2;

    radio1 = "/home/nelson/Projetos/Musicas/66.mp3";
    radio1 = "mmsh://radio.tokhost.com.br/germaniafm";  // ******* Erro
    radio1 = "http://wms5.fabricahost.com.br:8386/;stream.nsv";                          // mp3
    radio1 = "http://184-107-102-140.webnow.net.br:80/98fm.aac";                         // aac
//    radio1 = "http://livestream-f.akamaihd.net/3172111_1948081_f9882103_1_1756@103114";  // h24      ***** ERRO
//    radio1 = "rtmp://media.sgr.globo.com:80/VideoMusicais/beat98.sdp";                   // flv      ***** Erro
//    radio1 = "mmsh://divinal.dnip.com.br:1380/divinal?MSWMExt=.asf";                     // wmav2    ***** ERRO
//    radio1 = "mmsh://portalradios15.dnip.com.br:1380/radiocruzeiro?MSWMExt=.asf";        // wmapro   ***** ERRO
//    radio1 = "rtsp://servidor10.dnip.com.br/87fmagudos";                                 // *****ERRO

//    radio2 = "/home/nelson/Projetos/Musicas/66_out_mp3.aac";
    radio2 = "http://wms5.fabricahost.com.br:8386/;stream.nsv";                          // mp3

//    objTestes->ffmpeg_teste(radio1, radio2);

    return 0;

#else

    // registra os componentes do FFMPEG
    av_register_all();
    avcodec_register_all();
    ret = avformat_network_init();

    if (ret < 0)
    {
        //TODO implementar log
        cout << "falha ao iniciar componente de rede do FFMPEG." << endl;
        //TODO criar códigos de erro
        return -1;
    }

    // carrega configurações da captura
    config = new Configuration();

    // instancia objeto do banco de dados
    db = new Database(config->ConnectionStringSQL);

    // carrega filtro utilizado pelo fingerprint
    ret = Filter::FilterLoad(&Filters, config->FilterArqName);

    if (ret < 0)
    {
        //TODO implementar log
        cout << "falha ao carregar filtro do fingerprint." << endl;
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
            cout << "falha ao carregar lista de streams.\nSistema tentará novamente em " << config->UpdateTimer << " minutos." << endl;
        }
        else
        {
            for (int index = 0; index < urlStream.size(); index++)
            {
                // cria as threads
                objThreadPool->addThreads(urlStream[index]->url,urlStream[index]->radio);
            }
        }

        // sempre haverá um sleep para verificar novas rádios
        boost::this_thread::sleep(boost::posix_time::minutes(config->UpdateTimer));

    }
    while (true);



#endif // TESTE
}
