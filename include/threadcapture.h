#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include "/home/nelson/Projetos/mir/fingerprint/include/filter.h"


#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>


using namespace std;

/** \brief
*  Classe utilizado para criar a thread principal, onde conecta com a radio, captura um recorte,
*  transforma em fingerprint, envia para o servidor de reconhecimento e atualiza a base de dados.
 */
class ThreadCapture
{
    public:
        ThreadCapture();
        virtual ~ThreadCapture();

        string uriRadio;
        string slqConnString;
        string ipRecognition;
        string portRecognition;

        int status;
        int idThread;

        vector<Filter>* Filters;

        /** \brief Thread de um canal. cria o objeto MRServer e acerta os parametros
         *
         * \param tmp void*
         * \return void*
         *
         */
        void thrRun();
        void CloseThread();
    protected:
    private:

};

#endif // CAPTURE_H
