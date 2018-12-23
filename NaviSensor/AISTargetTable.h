#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include "AIS.h"
#include "AISConfig.h"

namespace AIS
{
    typedef std::pair <time_t, AISTarget *> AISTargetRec;

    typedef std::vector <AISStaticData> AISStaticDataArray;
    typedef std::vector <AISDynamicData> AISDynamicDataArray;

    class AISTargetTable
    {
        public:
            AISTargetTable (const time_t, Data::Pos *);
            virtual ~AISTargetTable ();

            AISTargetRec *findMostDistantTarget (double *range);

            AISTarget *findTarget (const unsigned int mmsi, AISTargetRec **record = 0);
            AISTarget *checkAddTarget (const unsigned int mmsi);

            inline void lock () { locker.lock (); }
            inline void unlock () { locker.unlock (); }

            void extractStaticData (AISStaticDataArray&);
            void extractDynamicData (AISDynamicDataArray&);

            void loadFiltering (const char * = 0);

            inline const Filtering *getFiltering () { return & filtering; }

            inline const Data::Pos *getCurPosition () { return curPosition; }

            inline const size_t size () { return container.size (); }

        protected:
            typedef std::map <const unsigned int, AISTargetRec> Container;

            Data::Pos  *curPosition;
            Container   container;
            Filtering   filtering;
            bool        active;
            time_t      timeout;
            std::mutex  locker;
            std::thread watchdog;

            void watchdogProc ();
            static void watchdogProcInternal (AISTargetTable *self);
    };
}