#include "VDM.h"

Parsers::VDM::VDM (AIS::AISTargetTable *targets) : NmeaParser ("VDM")
{
    this->targets  = targets;
    groupCompleted = true;
    lastProcessed  =
    curSeqNumber   = 0;

    aisParsers.registerParser (1, AIS::parseBaseReport);
    aisParsers.registerParser (2, AIS::parseBaseReport);
    aisParsers.registerParser (3, AIS::parseBaseReport);
    aisParsers.registerParser (4, AIS::parseBaseStationReport);
    aisParsers.registerParser (5, AIS::parseStaticVoyageReport);
    aisParsers.registerParser (18, AIS::parseStandardClassBPosReport);
    aisParsers.registerParser (19, AIS::parseExtendedClassBPosReport);
    aisParsers.registerParser (21, AIS::parseAidsToNavigationReport);
}

bool Parsers::VDM::parse (NMEA::Sentence& sentence, Sensors::Sensor *sensor)
{
    Tools::Strings& fields = sentence.getFields ();
    bool            result = false;

    if (fields.size () >= 7 && !fields.omitted (1) && !fields.omitted (2) && !fields.omitted (5) && !fields.omitted (6))
    {
        int sentenceNumber = fields.getAsIntAt (2);
        int numOfSentences = fields.getAsIntAt (1);

        // Process multi-sentence group
        if (numOfSentences > 1)
        {
            int seqNumber = fields.getAsIntAt (3);

            if (groupCompleted)
            {
                // Start the new group; should be from 1
                if (sentenceNumber == 1)
                {
                    // Normal start from the group begin
                    groupCompleted = true;
                    lastProcessed  = 1;
                    groupCompleted = false;
                    curSeqNumber   = seqNumber;

                    data.clear ();
                }
                else
                {
                    // A part of group is missing, ignore this sentence
                    return false;
                }
            }
            else
            {
                // The same group, next sentences?
                if (seqNumber == curSeqNumber && lastProcessed == (sentenceNumber - 1))
                {
                    lastProcessed  = sentenceNumber;
                    groupCompleted = sentenceNumber < numOfSentences;
                }
                else
                {
                    // Something is wrong, ignore
                    return false;
                }
            }
        }
        else
        {
            groupCompleted = true;
            lastProcessed  = 1;
            groupCompleted = true;

            data.clear ();
        }

        data.add (fields.getAt (5));

        if (groupCompleted)
            parseData ();
    }

    return result;
}

void Parsers::VDM::parseData ()
{
    AIS::AISTarget *target;

    unsigned char messageID = data.getByte (6);
    unsigned char repeatInd = data.getByte (2);
    unsigned int  mmsi      = data.getInt (30);

    targets->lock ();

    target = targets->checkAddTarget (mmsi);

    if (target)
        aisParsers.parseAIS (target, messageID, data);

    targets->unlock ();
}

void Parsers::VDM::AISParsers::parseAIS (AIS::AISTarget *target, unsigned char msgType, AIS::SixBitStorage& storage)
{
char a[100];
sprintf(a,"**msg %d\n",msgType);
OutputDebugString(a);

    auto pos = find (msgType);

    if (pos != end ())
        pos->second (target, storage);
}

void Parsers::VDM::AISParsers::registerParser (unsigned char msgType, VDM::AISParser parser)
{
    insert (end (), std::pair <unsigned char, AISParser&> (msgType, parser));
}
