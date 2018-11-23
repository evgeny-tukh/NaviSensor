#include "VBW.h"

Parsers::VBW::VBW () : NmeaParser ("VBW")
{
}

bool Parsers::VBW::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}