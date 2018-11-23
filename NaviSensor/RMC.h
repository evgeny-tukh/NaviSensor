#pragma once

#include "Parser.h"

namespace Parsers
{
    class RMC : public NmeaParser
    {
        public:
            RMC ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}