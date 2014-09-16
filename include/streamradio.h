#ifndef STREAMRADIO_H
#define STREAMRADIO_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "util.h"
#include "exceptionmir.h"
#include <boost/thread.hpp>

extern "C"
{
    /** inclusão dos headers FFMPEG */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
}

#include <boost/thread.hpp>

using namespace std;
namespace pt = boost::posix_time;

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
    void addOptions(string key, string value);

    /** \brief
    * Devolve as opções de configuração da conexão
    * caso não tenha opções será devolvido NULL
    */
    AVDictionary * getListOptions();

    /** \brief
    * Retorna um ponteiro para a FIFO de captura
    */
    AVAudioFifo**getFIFO();

    int getFifoSize();
    int getFifoData(void **data, int nb_samples);
    int getNumFrames(double sec);

protected:
private:
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVStream *stream;
    AVCodec *codec;
    AVFrame *frame = NULL;
    AVDictionary *dictionary;
    AVAudioFifo *fifo = NULL;
    clock_t timer;
    EnumStatusConnect statusConnection;
    StreamType * streamType;
    double duration;
    bool isExit;
    int bitRate; // contém o valor do bitrate de entrada
    bool isVBR; // true se for Variable BitRate
    boost::mutex mtx_;  // controle de lock da thread


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
    void rtspDetect(string uri);

    /** \brief
    * Aloca memória para as filas de entrada.
    *
    * \see AVAudioFifo
    */
    void initFIFO();

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

    /** \brief
    * Adiciona frame decodificado para o FIFO de entrada.
    *
    * \param inputSamples   - dados decodificados que serão inseridos no FIFO ( do Kininho )
    * \param frameSize      - tamanho dos dados.
    */
    void addSamplesFIFO(uint8_t **inputSamples, const int frameSize);
};

#endif // CONNECT_H
