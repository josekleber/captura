#ifndef EXCEPTIONMIR_INCLUDED
#define EXCEPTIONMIR_INCLUDED

/** @brief  Estruturas usadas para o controle das exceções do AudioMonitor
*
* Essas estruturas serão utilizadas pelos módulos de captura e identificação
*/

/*
* boost Header
*/
#include <boost/exception/all.hpp>

/** @brief Estrutura para informar o código do erro */
typedef boost::error_info<struct tag_errno_code,int> errno_code;

/** @brief Estrutura para informar a mensagem do erro */
typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;

/** @brief Exceção base de conexão
* estrutura usada para indicar que houve uma exceção de conexão
*/
struct BaseException : virtual std::exception, virtual boost::exception{};

/** \brief
* Exceção ocorre quando não for possível alocar memória
*/
struct BadAllocException : virtual BaseException {};

/** \brief
* Exceção ocorre quando não for possível conectar na URI informada
*/
struct OpenConnectionException : virtual BaseException {};

/** \brief
* Exceção ocorre quando se tenta acessar informações da conexão e/ou
* codec sem ter aberto a conexão.
*/
struct ConnectionClosedException : virtual BaseException {};

/** \brief
* Exceção ocorre quando na conexão não for identificado nenhum
* stream de audio, o que nesta versão não é suportado.
*/
struct MediaTypeNoAudioException : virtual BaseException {};

/** \brief
* Exceção ocorre quando for usado um codec não suportado.
*/
struct CodecNotSupportedException : virtual BaseException {};

/** \brief
* Ocorre quando um não é possível adicionar uma stream a um output
*/
struct StreamException : virtual BaseException {};

/** \brief
* Erro na leitura dos Frames
*/
struct FrameReadException : virtual BaseException {};

/** \brief
* Erro de decodificação dos Frames
*/
struct DecoderException : virtual BaseException {};

/** \brief
* Erro de decodificação dos Frames
*/
struct FFMpegException : virtual BaseException {};

/** \brief
* Erro de abertura de arquivo
*/
struct OpenFileException : virtual BaseException{};





/** \brief
* Exceção geral, usado no caso de erro pego por catch(...)
*/
struct GeneralException : virtual BaseException {};

/** \brief
* Erro de abertura de arquivo
*/
struct ResampleException : virtual BaseException{};

/** \brief
* Erro de abertura de arquivo
*/
struct StreamRadioException : virtual BaseException{};

/** \brief
* Erro de abertura de arquivo
*/
struct FifoException : virtual BaseException{};

/** @brief Enumerodor com as exceções dos processos MIR (AudioMonitor) */
enum MIR_EXCEPTION
{
    // CAPTURA
    MIR_ERR_STREAM_CONNECTION       = -10001,
    MIR_ERR_BADALLOC_CONTEXT        = -10002,
    MIR_ERR_CONNECTION_CLOSED       = -10003,
    MIR_ERR_MEDIA_TYPE_NO_AUDIO     = -10004,
    MIR_ERR_BADALLOC_FIFO           = -11005,
    MIR_ERR_CODEC_NOT_SUPPORTED     = -11001,
    MIR_ERR_FRAME_READ              = -11002,
    MIR_ERR_DECODE                  = -11003,

    // PROCESSO
    MIR_ERR_CREATE_FORMAT_CONTEXT   = -12001,
    MIR_ERR_OPEN_STREAM             = -12002,
    MIR_ERR_OUTPUT_FORMAT           = -12003,
    MIR_ERR_OPEN_CODEC              = -12004,
    MIR_ERR_ALLOC_SWR_CONTEXT       = -12005,
    MIR_ERR_INIT_INPUT_FRAME        = -12006,
    MIR_ERR_READ_INPUT_FRAME        = -12007,
    MIR_ERR_ALLOC_CONVERTED         = -12008,
    MIR_ERR_CONVERTER               = -12009,
    MIR_ERR_FRAME_ALLOC             = -12010,
    MIR_ERR_ENCONDING               = -12011,
    MIR_ERR_WRITE_FRAME             = -12012,
    MIR_ERR_OPEN_OUTPUT_FILE        = -12013,
    MIR_ERR_OPEN_FORMAT_CONTEXT     = -12014,
    MIR_ERR_OPEN_OUTPUT_FORMAT      = -12015,
    MIR_ERR_OPEN_CODEC_CONTEXT      = -12016,
    MIR_ERR_INIT_SWR_CONTEXT        = -12017,
    MIR_ERR_BUFFER_ALLOC            = -12018,
    MIR_ERR_RESAMPLE                = -12019,
    MIR_ERR_ENCODE                  = -12020,
    MIR_ERR_RAW_VECTOR_ALLOC        = -12021,
    MIR_ERR_CREATE_PATH             = -12022,

    // FIFO
    MIR_ERR_NOT_HAVE_FIFO_OBJECT    = -13001,
    MIR_ERR_FIFO_SIZE               = -13002,
    MIR_ERR_FIFO_ADD                = -13003,
    MIR_ERR_FIFO_BUFFER_SIZE        = -13004,
    MIR_ERR_FIFO_ADD_FORMAT         = -13005,
    MIR_ERR_FIFO_ADD_CHANNELS       = -13006,
    MIR_ERR_FIFO_GET                = -13007,
    MIR_ERR_FIFO_DATA1              = -13008,
    MIR_ERR_FIFO_DATA2              = -13009,


    // MIR_ERR_

    // IDENTIFICACAO


    //BANCO DE DADOS
    MIR_DB_ALLOC_ENV       = 15000,
    MIR_DB_SET_ENV         = 15001,
    MIR_DB_ALLOC_HANDLE    = 15002,
    MIR_DB_ALLOC_STMT      = 15003,
    MIR_DB_DRIVER_CONNECT  = 15004,
    MIR_DB_EXECUTE_QUERY   = 15005,
    MIR_DB_UNKOWN          = 15010,

    // usado para rastrear erros (pode ser dados temporarios, somente criados em modo de desenvolvimento)
    MIR_DES_STMRADIO_1     = -30000,
    MIR_DES_SIGNAL         = -30001,
    MIR_DES_PIPE           = -30002,
};
#endif // EXCEPTIONMIR_INCLUDED
