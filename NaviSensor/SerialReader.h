#pragma once

#include <Windows.h>
#include "Reader.h"

#pragma once

namespace Readers
{
    class SerialReader : public Reader
    {
        public:
            SerialReader (Sensors::SerialParams *config);
            ~SerialReader ();

            virtual size_t read ();
            virtual bool open ();
            virtual void close ();

        protected:
            HANDLE portHandle;
    };
}