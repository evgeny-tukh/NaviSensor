#include "VDM.h"

Parsers::VDO::VDO (AIS::AISTargetTable *targets) : VDM (targets)
{
    memcpy (this->type, "VDO", 4);
}

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

            if (groupCompleted || sentenceNumber == 1)
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
                    groupCompleted = sentenceNumber == numOfSentences;
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
            result         =
            groupCompleted = true;
            lastProcessed  = 1;

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
    AIS::AISTarget       *target;
    const AIS::Filtering *filtering = targets->getFiltering ();

    unsigned char messageID = data.getByte (6);
    unsigned char repeatInd = data.getByte (2);
    unsigned int  mmsi      = data.getInt (30);

    targets->lock ();

    if (filtering->limitAmount || filtering->limitRange)
    {
        target = targets->findTarget (mmsi);

        if (target)
        {
            // Target already exists, just update it
            aisParsers.parseAIS (target, messageID, data);
        }
        else
        {
            double             mostDistantRange;
            AIS::AISTargetRec *mostDistant = targets->findMostDistantTarget (& mostDistantRange);
            AIS::AISTarget     tempTarget (mmsi);
            const Data::Pos   *curPosition = targets->getCurPosition ();
            double             curRange    = Tools::calcDistanceRaftly (curPosition->lat, curPosition->lon,
                                                                        tempTarget.dynamicData.lat, tempTarget.dynamicData.lon);

            if (fabs(curPosition->lat) <= 90.0 && fabs(curPosition->lon) <= 180.0)
            {
                mostDistant = targets->findMostDistantTarget(&mostDistantRange);
                curRange    = Tools::calcDistanceRaftly(curPosition->lat, curPosition->lon, tempTarget.dynamicData.lat, tempTarget.dynamicData.lon);
            }
            else if (filtering->limitAmount && targets->size () >= (unsigned) filtering->maxAmount)
            {
                targets->unlock (); return;
            }
            else
            {
                mostDistant = 0;
                curRange    = 0;
            }

            data.saveState ();
            aisParsers.parseAIS (& tempTarget, messageID, data);
            data.restoreState ();

            if (filtering->limitRange && curRange > filtering->maxRange)
            {
                // Target is too faraway, ignore it
                targets->unlock (); return;
            }

            if (filtering->limitAmount && targets->size () >= (unsigned) filtering->maxAmount)
            {
                if (curRange >= mostDistantRange)
                {
                    // Target is more distant than most distant one from existing, ignore it
                    targets->unlock(); return;
                }
            }

            if (targets->size() >= (unsigned) filtering->maxAmount)
            {
                // Replace most distant target
                mostDistant->first        = time (0);
                mostDistant->second->mmsi = mmsi;

                aisParsers.parseAIS (mostDistant->second, messageID, data);
            }
            else
            {
                // Simple add new target
                target = targets->checkAddTarget(mmsi);

                if (target)
                    aisParsers.parseAIS (target, messageID, data);
            }
        }
    }
    else
    {
        target = targets->checkAddTarget (mmsi);

        if (target)
            aisParsers.parseAIS (target, messageID, data);
    }

    targets->unlock ();
}

void Parsers::VDM::AISParsers::parseAIS (AIS::AISTarget *target, unsigned char msgType, AIS::SixBitStorage& storage)
{
    auto pos = find (msgType);

    if (pos != end ())
        pos->second (target, storage);
}

void Parsers::VDM::AISParsers::registerParser (unsigned char msgType, VDM::AISParser parser)
{
    insert (end (), std::pair <unsigned char, AISParser&> (msgType, parser));
}
