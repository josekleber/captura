#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <iostream>

using namespace std;

class Configuration
{
    public:
        Configuration();

        std::string FilterArqName;
        std::string ConnectionStringSQL;
        std::string ConnectionStringMySQL;
        std::string Listener;
        std::string StreamList;
        /** \brief Tempo, em minutos, em que serão verificadas alterações nos streams */
        int UpdateTimer;

        std::string mrIP;
        std::string mrPort;

        std::string cutFolder;

        virtual ~Configuration();

    protected:

    private:

};

#endif // CONFIGURATION_H
