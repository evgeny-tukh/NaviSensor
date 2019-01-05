#pragma once

#include "Parser.h"

namespace Parsers
{
    class THS : public NmeaParser
    {
        public:
            THS ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}