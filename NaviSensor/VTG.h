#pragma once

#include "Parser.h"

namespace Parsers
{
    class VTG : public NmeaParser
    {
        public:
            VTG ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}