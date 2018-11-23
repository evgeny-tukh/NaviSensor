#include "GLL.h"

Parsers::GLL::GLL () : NmeaParser ("GLL")
{
}

bool Parsers::GLL::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields();
    Data::Time      utc;
    Data::Pos       position;
    char            validity,
                    modeIndicator;
    bool            dataValid = Parsers::extractChar (fields, 6, validity) && validity == 'A';

    if (dataValid)
    {
        if (Parsers::extractChar (fields, 7, modeIndicator))
            sensor->updateData (Data::DataType::PosSysMode, & modeIndicator, sizeof (modeIndicator));

        if (Parsers::extractUTC (fields, 5, utc))
            sensor->updateData(Data::DataType::UTC, & utc, sizeof (utc));

        if (Parsers::extractPosition(fields, 1, position))
            sensor->updateData(Data::DataType::Position, & position, sizeof (position));
    }

    return dataValid;
}