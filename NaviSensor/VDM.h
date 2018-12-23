#pragma once

#include <functional>
#include <map>
#include "Parser.h"
#include "SixBitStorage.h"
#include "AISTargetTable.h"

namespace Parsers
{
    class VDM : public NmeaParser
    {
        public:
            VDM (AIS::AISTargetTable *targets);

            virtual bool parse (NMEA::Sentence&, Sensors::Sensor *);

        protected:
            typedef std::function <void (AIS::AISTarget *, AIS::SixBitStorage&)> AISParser;

            class AISParsers: public std::map <unsigned char, AISParser>
            {
                public:
                    void parseAIS (AIS::AISTarget *, unsigned char, AIS::SixBitStorage&);

                    void registerParser (unsigned char, AISParser);
            };

            AIS::AISTargetTable *targets;
            AIS::SixBitStorage   data;
            AISParsers           aisParsers;
            int                  curSeqNumber, lastProcessed;
            bool                 groupCompleted;

            void parseData ();
    };

    class VDO : public VDM
    {
        public:
            VDO (AIS::AISTargetTable *targets);
    };
}