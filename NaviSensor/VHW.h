#pragma once

#include "Parser.h"

namespace Parsers
{
    class VHW : public NmeaParser
    {
        public:
            VHW ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}