#pragma once

#include <vector>
#include <thread>
#include "Parameters.h"

namespace Data
{
    typedef std::map <int, Parameter *> ParamMap;

    typedef std::pair <int, Parameter *> ParamInfo;

    typedef std::vector <Parameter *> ParamArray;
    typedef std::vector <GlobalParameter *> GlobalParamArray;

    class SensorDataStorage : public std::map <DataType, Parameter *>
    {
        public:
            SensorDataStorage (const int paramTimeout);
            virtual ~SensorDataStorage ();

            void update (Parameter& param);
            Parameter *findParam (DataType type);

            void extractAll (ParamArray& params);

        protected:
            int         paramTimeout;
            bool        active;
            std::thread watchdog;
            std::mutex  locker;

            void watchdogProc ();
            static void watchdogProcInternal (SensorDataStorage *);
    };

    class GlobalDataStorage : public std::map <DataType, ParamMap *>
    {
        public:
            GlobalDataStorage (const int paramTimeout);
            virtual ~GlobalDataStorage ();

            void update (const int sensorID, Parameter& param);
            Parameter *findFirst (DataType type, int& sensorID, const bool goodOnly = true);

            void extractAll (GlobalParamArray& params, DataType type = DataType::All, const bool goodOnly = true);

        protected:
            int         paramTimeout;
            bool        active;
            std::thread watchdog;
            std::mutex  locker;

            void watchdogProc ();
            static void watchdogProcInternal (GlobalDataStorage *);
    };
}
