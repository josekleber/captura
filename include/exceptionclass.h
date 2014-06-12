/**
 *
 * Author: Nelson Nagamine
 * 2013
 *
 */

#ifndef EXCEPTIONCLASS_H
#define EXCEPTIONCLASS_H

#include <string>
#include <exception>

using namespace std;

/** \brief classe para controle de excecoes. fornece informacao do arquivo, funcao e mensagem de quem gerou
 */
class ExceptionClass
{
    public:
        ExceptionClass(const string& File, const string& Function, const string& Message)
        {
            sFile = string(File);
            sFunction = string(Function);
            sMessage = string(Message);

            if (sFile != "")
                sSaida = sFile + " (" + sFunction + ") : " + sMessage;
            else
                sSaida = sMessage;
        }
        virtual ~ExceptionClass() {}

        virtual const char* what() const throw() {return (sSaida).c_str();}
    private:
        string sFile, sFunction, sMessage;
        string sSaida;
};

/** \brief classe para manipular erros do socket
 */
class SocketException
{
 public:
  SocketException ( std::string s ) : m_s ( s ) {};
  ~SocketException (){};

  std::string description() { return m_s; }

 private:

  std::string m_s;

};
#endif // EXCEPTIONCLASS_H
