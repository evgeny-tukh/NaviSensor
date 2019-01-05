#include "MWV.h"

Parsers::MWV::MWV () : NmeaParser ("MWV")
{
}

bool Parsers::MWV::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields();
    float           dir, speed;
    char            dirType, speedType, validity;

    if (Parsers::extractFloat (fields, 1, dir) && Parsers::extractChar (fields, 2, dirType) &&
        Parsers::extractFloat (fields, 3, speed) && Parsers::extractChar (fields, 4, speedType) &&
        /*Parsers::extractChar (fields, 5, validity) && validity == 'A' &&*/ fabs (dir) < 360.0)
    {
        switch (speedType)
        {
            case 'K':
                speed /= 3.6f; break;

            case 'N':
                speed *= (float) (1852.0 / 3600.0); break;
        }

        if (dirType == 'T')
        {
            sensor->updateData (Data::DataType::WindDirT, & dir, sizeof (dir));
            sensor->updateData (Data::DataType::WindSpeedT, & speed, sizeof (speed));
        }
        else
        {
            sensor->updateData (Data::DataType::WindDirR, & dir, sizeof (dir));
            sensor->updateData (Data::DataType::WindSpeedR, & speed, sizeof (speed));
        }
    }

    return true;
}