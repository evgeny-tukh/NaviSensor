#pragma once

#include <thread>
#include "../NaviSensor/Socket.h"
#include "../NaviSensor/Network.h"
#include "tools.h"

namespace Comm
{
    class LocalListener : public std::thread
    {
        public:
            LocalListener (unsigned int port, bool *active, Comm::Socket::ReadCb callback);

        protected:
            unsigned int         port;
            bool                *active;
            Comm::Socket::ReadCb callback;

            static void procInternal (LocalListener *);
            void proc ();
    };
}