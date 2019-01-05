#include "THS.h"

Parsers::THS::THS () : NmeaParser ("THS")
{
}

bool Parsers::THS::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields ();
    float           heading;
    char            mode;

    if (Parsers::extractFloat (fields, 1, heading) && Parsers::extractChar (fields, 2, mode) && fabs ((double) heading) < 180.0)
    {
        sensor->updateData (Data::DataType::GyroMode, & mode, sizeof (mode));
        sensor->updateData (Data::DataType::TrueHeading, & heading, sizeof (heading));
    }

    return true;
}