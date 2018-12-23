#pragma once

#include "UDPReader.h"

namespace Readers
{
    class TCPReader : public UDPReader
    {
        public:
            TCPReader (Sensors::TcpParams *);
            virtual ~TCPReader ();

            virtual size_t read ();
            virtual bool open ();
            virtual void close ();
    };
}