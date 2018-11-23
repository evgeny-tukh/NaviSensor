#pragma once

#include "Parser.h"

namespace Parsers
{
    class HDT : public NmeaParser
    {
        public:
            HDT ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}