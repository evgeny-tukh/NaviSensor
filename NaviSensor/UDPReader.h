#pragma once

#pragma once

#include <Windows.h>
#include "Reader.h"
#include "Socket.h"

namespace Readers
{
    class UDPReader : public Reader
    {
        public:
            UDPReader (Sensors::UdpParams *);
            virtual ~UDPReader ();

            virtual size_t read ();
            virtual bool open ();
            virtual void close ();

        protected:
            Comm::Socket *socket;
    };
}