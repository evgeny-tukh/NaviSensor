#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include "AIS.h"

namespace AIS
{
    typedef std::pair <time_t, AISTarget *> AISTargetRec;

    typedef std::vector <AISStaticData> AISStaticDataArray;
    typedef std::vector <AISDynamicData> AISDynamicDataArray;

    class AISTargetTable
    {
        public:
            AISTargetTable (const time_t);
            virtual ~AISTargetTable ();

            AISTarget *findTarget (const unsigned int mmsi, AISTargetRec **record = 0);
            AISTarget *checkAddTarget (const unsigned int mmsi);

            inline void lock () { locker.lock (); }
            inline void unlock () { locker.unlock (); }

            void extractStaticData (AISStaticDataArray&);
            void extractDynamicData (AISDynamicDataArray&);

        protected:
            typedef std::map <const unsigned int, AISTargetRec> Container;

            Container   container;
            bool        active;
            time_t      timeout;
            std::mutex  locker;
            std::thread watchdog;

            void watchdogProc ();
            static void watchdogProcInternal (AISTargetTable *self);
    };
}