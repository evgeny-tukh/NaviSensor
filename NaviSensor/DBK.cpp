#include "DBK.h"

Parsers::DBK::DBK () : NmeaParser ("DBK")
{
}

bool Parsers::DBK::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}