#pragma once

#include "Sensor.h"
#include "Sentence.h"
#include "Parameters.h"
#include "DataDef.h"
#include "AISTargetTable.h"

namespace Sensors
{
    class Sensor;
}

namespace Parsers
{
    class NmeaParser;

    typedef std::pair <NMEA::SentenceType, NmeaParser *> SentenceParseDef;

    typedef std::vector <SentenceParseDef> SentenceParsers;

    class NmeaParser
    {
        public:
            NmeaParser (const char *type);

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);

            inline const char *getType () { return type; }

        protected:
            char type [4];
    };

    class NmeaParsers : public SentenceParsers
    {
        public:
            NmeaParsers (AIS::AISTargetTable *aisTargets);
            virtual ~NmeaParsers ();

            void addParser (NmeaParser *);

            bool parse (NMEA::Sentence&, Sensors::Sensor *);

        protected:
            AIS::AISTargetTable *aisTargets;
    };

    bool extractFloat (Tools::Strings& fields, const size_t start, float& value);
    bool extractInteger (Tools::Strings& fields, const size_t start, int& value);
    bool extractByte (Tools::Strings& fields, const size_t start, byte& value);
    bool extractChar (Tools::Strings& fields, const size_t start, char& value);
    bool extractUTC (Tools::Strings& fields, const size_t start, Data::Time& utc);
    bool extractPosition (Tools::Strings& fields, const size_t start, Data::Pos& position);

    double deformatCoordinate (const char *source, const int degreeFieldSize, const char hemisphereChar);

    extern NmeaParsers *parsers;
}