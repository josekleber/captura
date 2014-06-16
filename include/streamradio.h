#ifndef STREAMRADIO_H
#define STREAMRADIO_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "exceptionmir.h"

extern "C"
{
    /** inclusão dos headers FFMPEG */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

/** \brief
* Exceção ocorre quando não for possível alocar memória
*/
struct BadAllocException : virtual ConnectException {};

/** \brief
* Exceção ocorre quando não for possível conectar na URI informada
*/
struct OpenConnectionException : virtual ConnectException {};

/** \brief
* Exceção ocorre quando se tenta acessar informações da conexão e/ou
* codec sem ter aberto a conexão.
*/
struct ConnectionClosedException : virtual ConnectException {};

/** \brief
* Exceção ocorre quando na conexão não for identificado nenhum
* stream de audio, o que nesta versão não é suportado.
*/
struct MediaTypeNoAudioException : virtual ConnectException{};


/** \brief
* Enumerador do status da conexão
*/
enum EnumStatusConnect
{
    MIR_CONNECTION_OPEN,
    MIR_CONNETION_CLOSE,
    MIR_CONNECTION_ERROR
};

/** \brief
* Estrutura que representa os streams de uma conexão
*/
struct StreamType
{
    unsigned int id;
    AVMediaType streamType;
};


/** \brief
 * Classe que representa uma conexão com um stream ou arquivo
 *
 * O propósito dessa classe é apenas estabelecer uma conexão e setar
 * um AVFormatContext que será utilizado, como ponteiro, na captura.
 *
 */
class StreamRadio
{
public:

    /** \brief Default constructor */
    StreamRadio();

    /** \brief Default destructor */
    virtual ~StreamRadio();

    /** \brief
    * abre conexão com o stream, ou arquivo, indicado pelo URI
    * se tiver sucesso devolve AVFormatConext, caso contrario,
    * devolve NULL.
    *
    * Numa conexão podem vir vários streams, como estamos usando
    * apenas audio no momento, o primeiro stream de audio da
    * sequência será utilizado por padrão.
    *
    *   \param  uri - URL do stream e/ou full path do arquivo
    *   \return retorna uma instãncia de AVFormatContext
    *   \exception BadAllocException, OpenConnectionException, ConnectionClosedException
    */
    AVFormatContext* open(string *uri);

    /** \brief
    * Fecha uma conexão.
    * É importante chamar este método para liberar a memória alocada
    * para o objeto AVFormatContext e o AVCodecContext
    */
    void close();

    /** \brief
    * Retorna o status da conexão
    *
    */
    EnumStatusConnect getStatus();

    /** \brief
    * Tempo decorrido desde que a conexão foi estabelecida
    * caso a conexão esteja fechada será devolvido "0"
    */
    double getConnectionTime();

    /** \brief
    * Retorna os streams da conexão
    * Uma conexão pode ter vários streams, como : vídeo, audio, etc.
    * Nesta versão vamos trabalhar apenas com o primeiro stream de
    * audio que identificarmos.
    *
    * Caso não seja identificado nenhum stream de audio será lançada
    * uma exceção.
    *
    * \exception ConnectionClosedException
    */
    StreamType * getStreamType();

    /** \brief
    * Retorna o AVCodecContext do stream de audio
    *
    */
    AVCodecContext * getCodecContext();

    /** \brief
    * Retorna o stream de audio
    *
    */
    AVStream * getStream();

    /** \brief
    * Adiciona um parâmetro para a configuração das opções de conexão
    *
    * \param key    - parâmetro de configuração
    * \param value  - valor do parâmetro
    */
    void addOptions(string *key, string *value);

    /** \brief
    * Devolve as opções de configuração da conexão
    * caso não tenha opções será devolvido NULL
    */
    AVDictionary * getListOptions();

protected:
private:
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVStream *stream;
    AVDictionary *dictionary;
    clock_t timer;
    EnumStatusConnect statusConnection;
    StreamType * streamType;

    /** \brief
    * Define os streams existentes numa conexão
    *
    * \exception MediaTypeNoAudioException
    */
    void setStreamType();

    /** \brief
    * Verifica se na URI foi informado
    * o protocolo rtsp, em caso afirmativo
    * será utilizado o TCP como protocolo
    * de comunicação.
    *
    *
    * \param uri - url do stream.
    */
    void rtspDetect(string *uri);
};

#endif // CONNECT_H
