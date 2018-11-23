#pragma once

#include "Parser.h"

namespace Parsers
{
    class DBT : public NmeaParser
    {
        public:
            DBT ();

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);
    };
}