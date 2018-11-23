#pragma once

#include "TextSensor.h"

namespace Sensors
{
    class NmeaSensor : public TextSensor
    {
        protected:
            virtual size_t extractData ();

    };
}