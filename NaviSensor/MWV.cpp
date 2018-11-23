#include "MWV.h"

Parsers::MWV::MWV () : NmeaParser ("MWV")
{
}

bool Parsers::MWV::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}