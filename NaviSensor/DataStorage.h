#pragma once

#include <vector>
#include <thread>
#include "Parameters.h"

namespace Data
{
    class ParamMap : public std::map <int, Parameter *>
    {
        public:
            ParamMap ()
            {
                masterSourceID = 0;
            }

            inline void assignMasterSourceID (const int id)
            {
                masterSourceID = id;
            }

            inline const int getMasterSourceID ()
            {
                return masterSourceID;
            }

            inline bool hasMasterSource ()
            {
                return masterSourceID > 0;
            }

        protected:
            int masterSourceID;
    };

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

            void assignMasterSource (const int sensorID, Data::DataType paramType);

        protected:
            int         paramTimeout;
            bool        active;
            std::thread watchdog;
            std::mutex  locker;

            void watchdogProc ();
            static void watchdogProcInternal (GlobalDataStorage *);
    };
}
