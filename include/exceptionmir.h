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

/** @brief Exceção base de conexão
* estrutura usada para indicar que houve uma exceção de conexão
*/
struct BaseException : virtual std::exception, virtual boost::exception{};

/** @brief Estrutura para informar o código do erro */
typedef boost::error_info<struct tag_errno_code,int> errno_code;

/** @brief Enumerodor com as exceções dos processos MIR (AudioMonitor) */
enum MIR_EXCEPTION
{
    // CAPTURA
    MIR_ERR_STREAM_CONNECTION       = -10001,
    MIR_ERR_BADALLOC_CONTEXT        = -10002,
    MIR_ERR_CONNECTION_CLOSED       = -10003,
    MIR_ERR_MEDIA_TYPE_NO_AUDIO     = -10004,
    MIR_ERR_CODEC_NOT_SUPPORTED     = -11001,
    MIR_ERR_FRAME_READ              = -11002,
    MIR_ERR_DECODE                  = -11003

    // IDENTIFICACAO

};


#endif // EXCEPTIONMIR_INCLUDED
