#include "HDT.h"

Parsers::HDT::HDT () : NmeaParser ("HDT")
{
}

bool Parsers::HDT::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields();
    float           heading;
    char            type;

    if (Parsers::extractFloat (fields, 1, heading) && Parsers::extractChar (fields, 2, type) && type == 'T' && fabs ((double) heading) < 180.0)
        sensor->updateData (Data::DataType::TrueHeading, & heading, sizeof (heading));

    return true;
}