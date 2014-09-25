#ifndef STREAMRADIO_H
#define STREAMRADIO_H

#include <stdio.h>
#include <time.h>
#include <string.h>

extern "C"
{
    /** inclusão dos headers FFMPEG */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <boost/thread.hpp>

#include "util.h"
#include "exceptionmir.h"
#include "queue.h"

using namespace std;
namespace pt = boost::posix_time;

/** \brief
* Enumerador do status da conexão
*/
enum EnumStatusConnect
{
    MIR_CONNECTION_OPEN,
    MIR_CONNETION_CLOSE,
    MIR_CONNETION_CLOSING,
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
 * Classe que responsável por conectar com um stream ou arquivo de áudio
 * e gerar o FIFO de entrada com os frames que serão capturados.
 *
 * Os dados salvos no FIFO serão dados decodificados.
 *
 * \see https://www.ffmpeg.org/doxygen/trunk/transcode__aac_8c.html
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
    AVFormatContext* open(string uri);

    /** \brief
    * Fecha uma conexão.
    * É importante chamar este método para liberar a memória alocada
    * para o objeto AVFormatContext e o AVCodecContext
    */
    void close();

    /** \brief
    * Lê os dados de um stream.
    */
    void read();

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
    __attribute__((deprecated)) StreamType * getStreamType();

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
    void addOptions(string key, string value);

    /** \brief
    * Devolve as opções de configuração da conexão
    * caso não tenha opções será devolvido NULL
    */
    AVDictionary * getListOptions();

    /** \brief
    * Devido a um BUG no decode da entrada AAC temos de manter o ChannelLayout da entrada antes de chamar o decode.
    */
    int getChannelLayout();

    int getQueueSize();
    int getChannelSize();
    vector<vector<uint8_t>> getQueueData();

protected:
private:
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVStream *stream;
    AVCodec *codec;
    AVFrame *frame = NULL;
    AVDictionary *dictionary;

    clock_t timer;
    EnumStatusConnect statusConnection;
    __attribute__((deprecated)) StreamType * streamType;
    bool isExit;
    int bitRate; // contém o valor do bitrate de entrada
    bool isVBR; // true se for Variable BitRate
    Queue* objQueue;
    int streamIndex; // contém o indice do stream de aúdio localizado av_find_best_stream.
    string uri; // URL ou arquivo de entrada de dados.
    int channelLayout; // fix bug de entrada AAC, onde ao chamar o decode ele modifica o layout do channel de entrada.

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
    */
    void rtspDetect();

    /**\brief
    * Lê os frames da conexão e armazena no FIFO.
    * A captura será constante
    */
    void readFrame();

    /** \brief
    * Decodifica um frame do stream de áudio.
    *
    * \param frame          - ponteiro para o frame do stream.
    * \param fmt_ctx_in     - ponteiro para o contexto I/O de entrada.
    * \param cdc_ctx_in     - ponteiro para o contexto do codec de entrada.
    * \param data           - buffer utilizado na decodificação.
    * \param finished       - controle para verificar se chegou a processar todos os dados do frame.
    *
    */
    void decodeAudioFrame(int *data, int *finished, AVPacket *inputPacket);
};

#endif // CONNECT_H
