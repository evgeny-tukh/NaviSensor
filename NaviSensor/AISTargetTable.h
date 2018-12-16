#pragma once

#include <map>
#include <mutex>
#include <thread>
#include "AIS.h"

namespace AIS
{
    typedef std::pair <time_t, AISTarget *> AISTargetRec;

    class AISTargetTable : public std::map <const unsigned int, AISTargetRec>
    {
        public:
            AISTargetTable (const time_t);
            virtual ~AISTargetTable ();

            AISTarget *findTarget (const unsigned int mmsi, AISTargetRec **record = 0);
            AISTarget *checkAddTarget (const unsigned int mmsi);

            inline void lock () { locker.lock (); }
            inline void unlock () { locker.unlock (); }

        protected:
            bool        active;
            time_t      timeout;
            std::mutex  locker;
            std::thread watchdog;

            void watchdogProc ();
            static void watchdogProcInternal (AISTargetTable *self);
    };
}