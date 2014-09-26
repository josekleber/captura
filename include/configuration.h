#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "util.h"

using namespace std;

class Configuration
{
    public:
        Configuration();

        std::string FilterArqName;
        std::string ConnectionStringSQLProducao;
        std::string ConnectionStringMySQL;
        std::string Listener;
        std::string StreamList;
        /** \brief Tempo, em minutos, em que serão verificadas alterações nos streams */
        int UpdateTimer;

        int mrOn;
        bool svFP;
        std::string mrIP;
        std::string mrPort;

        std::string cutFolder;

        virtual ~Configuration();

    protected:

    private:

};

#endif // CONFIGURATION_H
