#include "Parser.h"
#include "GLL.h"
#include "GGA.h"
#include "VTG.h"
#include "VBW.h"
#include "VHW.h"
#include "HDT.h"
#include "RMC.h"
#include "DBK.h"
#include "DBS.h"
#include "DBT.h"
#include "MWV.h"
#include "VDM.h"

Parsers::NmeaParsers *Parsers::parsers = 0;

Parsers::NmeaParser::NmeaParser (const char *type)
{
    memcpy (this->type, type, 4);
}

bool Parsers::NmeaParser::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    return false;
}

Parsers::NmeaParsers::NmeaParsers (AIS::AISTargetTable *aisTargets)
{
    parsers = this;

    this->aisTargets = aisTargets;

    addParser (new GLL ());
    addParser (new GGA ());
    addParser (new VTG ());
    addParser (new VBW ());
    addParser (new VHW ());
    addParser (new HDT ());
    addParser (new RMC ());
    addParser (new DBK ());
    addParser (new DBS ());
    addParser (new DBT ());
    addParser (new MWV ());
    addParser (new VDM (aisTargets));
}

Parsers::NmeaParsers::~NmeaParsers ()
{
    for (auto & def : *this)
        delete def.second;
}

void Parsers::NmeaParsers::addParser (NmeaParser *parser)
{
    push_back (std::pair <NMEA::SentenceType, NmeaParser *> (NMEA::SentenceType (parser->getType ()), parser));
}

bool Parsers::NmeaParsers::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    bool result = false;

    for (auto & def : *this)
    {
        if (sentence.getType() == def.first.type)
        {
            result = def.second->parse (sentence, sensor); break;
        }
    }

    return result;
}

bool Parsers::extractPosition (Tools::Strings& fields, const size_t start, Data::Pos& position)
{
    bool result = false;

    if (fields.size() > (start + 3))
    {
        std::string& field = fields[start];

        if (field.length() >= 6)
        {
            position.lat = Parsers::deformatCoordinate (fields [start].c_str (), 2, fields [start+1][0]);
            position.lon = Parsers::deformatCoordinate (fields [start+2].c_str (), 3, fields [start+3][0]);

            result = fabs(position.lat) <= 90.0 && fabs(position.lon) < 180.0;
        }
    }

    return result;
}

bool Parsers::extractFloat (Tools::Strings& fields, const size_t start, float& value)
{
    bool result = false;

    if (fields.size () > start)
    {
        std::string& field = fields [start];

        if (field.length () > 0)
        {
            value  = (float) atof (field.c_str ());
            result = true;
        }
    }

    return result;
}

bool Parsers::extractByte (Tools::Strings& fields, const size_t start, byte& value)
{
    int  intValue;
    bool result = extractInteger (fields, start, intValue);

    value = (byte) (intValue & 255);

    return result;
}

bool Parsers::extractChar (Tools::Strings& fields, const size_t start, char& value)
{
    bool result = false;

    if (fields.size () > start)
    {
        std::string& field = fields [start];

        if (field.length() > 0)
        {
            value = field [0];
            result = true;
        }
    }

    return result;
}

bool Parsers::extractInteger (Tools::Strings& fields, const size_t start, int& value)
{
    bool result = false;

    if (fields.size() > start)
    {
        std::string& field = fields[start];

        if (field.length() > 0)
        {
            value  = atoi (field.c_str());
            result = true;
        }
    }

    return result;
}

bool Parsers::extractUTC (Tools::Strings& fields, const size_t start, Data::Time& utc)
{
    bool result = false;

    if (fields.size () > start)
    {
        std::string& field = fields [start];

        if (field.length () >= 6)
        {
            const char *time = field.c_str ();

            utc.hour = Tools::twoDecCharToInt (time);
            utc.min  = Tools::twoDecCharToInt (time + 2);
            utc.sec  = Tools::twoDecCharToInt (time + 4);

            result = utc.hour >= 0 && utc.hour < 24 && utc.min >= 0 && utc.min < 60 && utc.sec >= 0 && utc.sec < 60;
        }
    }

    return result;
}

double Parsers::deformatCoordinate (const char *source, const int degreeFieldSize, const char hemisphereChar)
{
    double coordinate;

    switch (degreeFieldSize)
    {
        case 2:
            coordinate = (double) Tools::twoDecCharToInt (source); break;

        case 3:
            coordinate = Tools::threeDecCharToInt (source); break;

        default:
            coordinate = 10000.0;
    }

    if (fabs (coordinate) <= 180.0)
    {
        coordinate += atof (source + degreeFieldSize) / 60.0;

        if (hemisphereChar == 'S' || hemisphereChar == 'W')
            coordinate = - coordinate;
    }

    return coordinate;
}