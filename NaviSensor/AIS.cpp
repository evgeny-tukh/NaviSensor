#include <time.h>
#include "AIS.h"

const char *AIS::navStatusName (const NavStatus navStatus)
{
    const char *result;

    switch (navStatus)
    {
        case UnderWayUsingEngine:
            result = "Under way using engine"; break;

        case AtAnchor:
            result = "At anchor"; break;

        case NotUnderCommand:
            result = "Not under command"; break;

        case RestrictedManoeuvrability:
            result = "Restricted manoeuvrability"; break;

        case ConstrainedByDraft:
            result = "Constrained by her draft"; break;

        case Moored:
            result = "Moored"; break;

        case Aground:
            result = "Aground"; break;

        case EngagedInFinishing:
            result = "Engaged in finishing"; break;

        case UnderWaySailing:
            result = "Under way sailing"; break;

        case ReservedHSC:
            result = "reserved for future amendment of navigational status for ships carrying DG, HS, or MP, or IMO hazard "
                     "or pollutant category C (HSC)";

            break;

        case ReservedWIG:
            result = "reserved for future amendment of navigational status for ships carrying DG, HS or MP, or IMO hazard "
                     "or pollutant category A (WIG)";

            break;

        case ReservedForFuture1:
        case ReservedForFuture2:
        case ReservedForFuture3:
        case ReservedForFuture4:
            result = "Reserved for future use"; break;

        default:
            result = "Undefined";
    }

    return result;
}

void AIS::parseBaseReport (AIS::AISTarget *target, SixBitStorage& data)
{
    AISDynamic    *dynData    = & target->dynamicData;
    unsigned char  navStatus  = data.getByte (4),
                   rateOfTurn = data.getByte (8);
    unsigned short sog        = data.getShort (10);
    bool           accuracy   = data.getFlag ();
    int            lon        = (signed) data.getInt (28, true),
                   lat        = (signed) data.getInt (27, true);
    unsigned short cog        = data.getShort (12),
                   hdg        = data.getShort (9);
    unsigned char  timestamp  = data.getByte (6),
                   regional   = data.getByte (4),
                   spare      = data.getByte (1);
    bool           raimFlag   = data.getFlag ();
    unsigned int   commState  = data.getInt (19);

    if (accuracy)
        target->setFlag (TargetFlags::PositionAccuracy);
    else
        target->clearFlag (TargetFlags::PositionAccuracy);

    if (raimFlag)
        target->setFlag (TargetFlags::RAIM);
    else
        target->clearFlag (TargetFlags::RAIM);

    if (rateOfTurn != 0x80)
    {
        target->setFlag (TargetFlags::RateOfTurn);

        if (rateOfTurn = 0x7F)
        {
            dynData->rateOfTurn = 720.0f;
            
            target->setFlag (TargetFlags::RateOfTurnOutOfScale);
        }
        else if (rateOfTurn = 0x81)
        {
            dynData->rateOfTurn = -720.0f;
            
            target->setFlag (TargetFlags::RateOfTurnOutOfScale);
        }
        else
        {
            dynData->rateOfTurn = (float) (4.733 * sqrt ((double) rateOfTurn));
            
            target->clearFlag (TargetFlags::RateOfTurnOutOfScale);
        }
    }
    else
    {
        target->clearFlag (TargetFlags::RateOfTurn);
    }

    if (sog != 1023)
    {
        target->setFlag (TargetFlags::SOG);

        if (sog == 1022)
        {
            dynData->sog = 102.2f;

            target->setFlag (SOGOutOfScale);
        }
        else
        {
            dynData->sog = (float) ((double) sog * 0.1);

            target->clearFlag (SOGOutOfScale);
        }
    }
    else
    {
        target->clearFlag (TargetFlags::SOG);
    }

    if (hdg != 511)
    {
        target->setFlag (TargetFlags::HDG);
        target->clearFlag (TargetFlags::HDGOutOfScale);
        
        dynData->hdg = (float) hdg;
    }
    else
    {
        target->clearFlag (TargetFlags::HDG);
    }

    if (lon != 0x6791AC0)
    {
        target->setFlag (TargetFlags::Lon);

        dynData->lon = (double) lon / 600000.0;

    }
    else
    {
        target->clearFlag (TargetFlags::Lon);
    }

    if (lat != 0x3412140)
    {
        target->setFlag (TargetFlags::Lat);

        dynData->lat = (double) lat / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lat);
    }
}

void AIS::parseBaseStationReport (AIS::AISTarget *target, SixBitStorage& data)
{
    target->flags |= TargetFlags::BaseStation;

    unsigned short year       = data.getShort (14);
    unsigned char  month      = data.getByte (4),
                   day        = data.getByte (5),
                   hour       = data.getByte (5),
                   min        = data.getByte (6),
                   sec        = data.getByte (6);
    bool           accuracy   = data.getFlag ();
    int            lon        = (signed) data.getInt (28, true),
                   lat        = (signed) data.getInt (27, true);
    unsigned char  psType     = data.getByte (4);
    unsigned short spare      = data.getShort (10);
    bool           raimFlag   = data.getFlag ();
    unsigned int   commState  = data.getInt (19);
    AISDynamic    *dynData    = & target->dynamicData;
    AISStatic     *staticData = & target->staticData;

    dynData->year  = year;
    dynData->month = month;
    dynData->day   = day;
    dynData->hour  = hour;
    dynData->min   = min;
    dynData->sec   = sec;

    staticData->psType = psType;

    if (accuracy)
        target->setFlag (TargetFlags::PositionAccuracy);
    else
        target->clearFlag (TargetFlags::PositionAccuracy);

    if (raimFlag)
        target->setFlag (TargetFlags::RAIM);
    else
        target->clearFlag (TargetFlags::RAIM);

    if (lon != 0x6791AC0)
    {
        target->setFlag (TargetFlags::Lon);

        dynData->lon = (double)lon / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lon);
    }

    if (lat != 0x3412140)
    {
        target->setFlag (TargetFlags::Lat);

        dynData->lat = (double)lat / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lat);
    }
}

void AIS::parseStandardClassBPosReport (AIS::AISTarget *target, SixBitStorage& data)
{
    AISDynamic    *dynData                 = & target->dynamicData;
    unsigned char  reservedForRegionalApps = data.getByte (8);
    unsigned short sog                     = data.getShort (10);
    bool           accuracy                = data.getFlag ();
    int            lon                     = (signed)data.getInt (28, true),
                   lat                     = (signed)data.getInt (27, true);
    unsigned short cog                     = data.getShort (12),
                   hdg                     = data.getShort (9);
    unsigned char  timestamp               = data.getByte (6),
                   regional                = data.getByte (4),
                   spare                   = data.getByte (4);
    bool           raimFlag                = data.getFlag ();
    unsigned int   commState               = data.getInt (19);

    if (accuracy)
        target->setFlag (TargetFlags::PositionAccuracy);
    else
        target->clearFlag (TargetFlags::PositionAccuracy);

    if (raimFlag)
        target->setFlag (TargetFlags::RAIM);
    else
        target->clearFlag (TargetFlags::RAIM);

    if (sog != 1023)
    {
        target->setFlag (TargetFlags::SOG);

        if (sog == 1022)
        {
            dynData->sog = 102.2f;

            target->setFlag (SOGOutOfScale);
        }
        else
        {
            dynData->sog = (float)((double)sog * 0.1);

            target->clearFlag (SOGOutOfScale);
        }
    }
    else
    {
        target->clearFlag (TargetFlags::SOG);
    }

    if (hdg != 511)
    {
        target->setFlag (TargetFlags::HDG);
        target->clearFlag (TargetFlags::HDGOutOfScale);

        dynData->hdg = (float)hdg;
    }
    else
    {
        target->clearFlag (TargetFlags::HDG);
    }

    if (lon != 0x6791AC0)
    {
        target->setFlag (TargetFlags::Lon);

        dynData->lon = (double)lon / 600000.0;

    }
    else
    {
        target->clearFlag (TargetFlags::Lon);
    }

    if (lat != 0x3412140)
    {
        target->setFlag (TargetFlags::Lat);

        dynData->lat = (double)lat / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lat);
    }
}

void AIS::parseExtendedClassBPosReport (AIS::AISTarget *target, SixBitStorage& data)
{
    AISDynamic    *dynData       = & target->dynamicData;
    AISStatic     *staticData    = & target->staticData;
    unsigned char  regApps       = data.getByte (8);
    unsigned short sog           = data.getShort (10);
    bool           accuracy      = data.getFlag ();
    int            lon           = (signed) data.getInt (28, true),
                   lat           = (signed) data.getInt (27, true);
    unsigned short cog           = data.getShort (12),
                   hdg           = data.getShort (9);
    unsigned char  timestamp     = data.getByte (6),
                   regional      = data.getByte (4);
    std::string    name          = data.getString (20);
    unsigned char  shipCargoType = data.getByte (8);
    unsigned int   dimensions    = data.getInt (30);
    unsigned char  psType        = data.getByte (4);
    bool           raimFlag      = data.getFlag ();
    bool           dte           = data.getFlag();

    if (dte)
        target->setFlag (TargetFlags::DTE);
    else
        target->clearFlag (TargetFlags::DTE);

    if (accuracy)
        target->setFlag (TargetFlags::PositionAccuracy);
    else
        target->clearFlag (TargetFlags::PositionAccuracy);

    if (raimFlag)
        target->setFlag (TargetFlags::RAIM);
    else
        target->clearFlag (TargetFlags::RAIM);

    if (sog != 1023)
    {
        target->setFlag (TargetFlags::SOG);

        if (sog == 1022)
        {
            dynData->sog = 102.2f;

            target->setFlag (SOGOutOfScale);
        }
        else
        {
            dynData->sog = (float)((double)sog * 0.1);

            target->clearFlag (SOGOutOfScale);
        }
    }
    else
    {
        target->clearFlag (TargetFlags::SOG);
    }

    staticData->psType = psType;

    if (hdg != 511)
    {
        target->setFlag (TargetFlags::HDG);
        target->clearFlag (TargetFlags::HDGOutOfScale);

        dynData->hdg = (float)hdg;
    }
    else
    {
        target->clearFlag (TargetFlags::HDG);
    }

    if (lon != 0x6791AC0)
    {
        target->setFlag (TargetFlags::Lon);

        dynData->lon = (double) lon / 600000.0;

    }
    else
    {
        target->clearFlag (TargetFlags::Lon);
    }

    if (lat != 0x3412140)
    {
        target->setFlag (TargetFlags::Lat);

        dynData->lat = (double)lat / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lat);
    }

    target->assignString (target->staticData.name, sizeof (target->staticData.name), name, TargetFlags::Name);

    if (shipCargoType)
    {
        target->setFlag (TargetFlags::ShipCargoType);

        staticData->shipCargoType = shipCargoType;
    }
    else
    {
        target->clearFlag (TargetFlags::ShipCargoType);
    }

    target->staticData.dimAhead  = dimensions & 0x1ff;
    target->staticData.dimAstern = (dimensions >> 9) & 0x1ff;
    target->staticData.dimPort   = (dimensions >> 15) & 0x3f;
    target->staticData.dimStbd   = (dimensions >> 21) & 0x3f;
}

////
void AIS::parseAidsToNavigationReport (AIS::AISTarget *target, SixBitStorage& data)
{
    AISDynamic   *dynData    = & target->dynamicData;
    AISStatic    *staticData = & target->staticData;
    unsigned char aToNType   = data.getByte (5);
    std::string   name       = data.getString (20);
    bool          accuracy   = data.getFlag ();
    int           lon        = (signed) data.getInt (28, true),
                  lat        = (signed) data.getInt (27, true);
    unsigned int  dimensions = data.getInt (30);
    unsigned char psType     = data.getByte (4),
                  timestamp  = data.getByte (6);
    bool          offPos     = data.getFlag ();
    unsigned char regApps    = data.getByte (8);
    bool          raimFlag   = data.getFlag ();
    unsigned char spare      = data.getByte (3);

    staticData->psType = psType;

    target->setFlag (AIS::TargetFlags::AtoN);

    if (offPos)
        target->setFlag (AIS::TargetFlags::OffPosition);
    else
        target->clearFlag (AIS::TargetFlags::OffPosition);

    if (accuracy)
        target->setFlag (TargetFlags::PositionAccuracy);
    else
        target->clearFlag (TargetFlags::PositionAccuracy);

    if (raimFlag)
        target->setFlag (TargetFlags::RAIM);
    else
        target->clearFlag (TargetFlags::RAIM);

    if (aToNType)
    {
        staticData->aToNType = aToNType;

        target->setFlag (AIS::TargetFlags::AtoNType);
    }
    else
    {
        target->clearFlag(AIS::TargetFlags::AtoNType);
    }

    if (lon != 0x6791AC0)
    {
        target->setFlag (TargetFlags::Lon);

        dynData->lon = (double) lon / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lon);
    }

    if (lat != 0x3412140)
    {
        target->setFlag (TargetFlags::Lat);

        dynData->lat = (double) lat / 600000.0;
    }
    else
    {
        target->clearFlag (TargetFlags::Lat);
    }

    target->assignString (target->staticData.name, sizeof (target->staticData.name), name, TargetFlags::Name);

}
////

void AIS::parseStaticVoyageReport (AIS::AISTarget *target, SixBitStorage& data)
{
    unsigned char aisVersion    = data.getByte (2);
    unsigned int  imoNumber     = data.getInt (30);
    std::string   callSign      = data.getString (7),
                  name          = data.getString (20);
    unsigned char shipCargoType = data.getByte (8);
    unsigned int  dimensions    = data.getInt (30);
    unsigned char psType        = data.getByte (4);
    unsigned int  etaPacked     = data.getInt (20);
    unsigned char maxDraft      = data.getByte (8);
    std::string   dest          = data.getString (20);
    bool          dte           = data.getFlag ();
    unsigned char spare         = data.getByte (1);
    AISStatic    *staticData    = & target->staticData;
    time_t        now           = time(0);
    tm           *eta           = gmtime (& now);

    staticData->psType = psType;

    if (dte)
        target->setFlag (TargetFlags::DTE);
    else
        target->clearFlag (TargetFlags::DTE);

    if (imoNumber)
    {
        target->setFlag (TargetFlags::IMONumber);

        staticData->imoNumber = imoNumber;
    }
    else
    {
        target->clearFlag (TargetFlags::IMONumber);
    }

    target->assignString (target->staticData.callSign, sizeof (target->staticData.callSign), callSign, TargetFlags::CallSign);
    target->assignString (target->staticData.name, sizeof (target->staticData.name), name, TargetFlags::Name);
    target->assignString (target->staticData.destination, sizeof(target->staticData.destination), dest, TargetFlags::Destination);

    if (shipCargoType)
    {
        target->setFlag (TargetFlags::ShipCargoType);

        staticData->shipCargoType = shipCargoType;
    }
    else
    {
        target->clearFlag (TargetFlags::ShipCargoType);
    }

    target->staticData.dimAhead  = dimensions & 0x1ff;
    target->staticData.dimAstern = (dimensions >> 9) & 0x1ff;
    target->staticData.dimPort   = (dimensions >> 15) & 0x3f;
    target->staticData.dimStbd   = (dimensions >> 21) & 0x3f;

    eta->tm_mon  = (etaPacked >> 16) - 1;
    eta->tm_mday = (etaPacked >> 11) & 0x1f;
    eta->tm_hour = (etaPacked >> 6) & 0x1f;
    eta->tm_min  = etaPacked & 0x11f;

    target->staticData.eta = mktime (eta);


    if (callSign.compare ("@@@@@@@") != 0)
    {
        target->setFlag (TargetFlags::CallSign);

        memset (staticData->callSign, 0, sizeof (staticData->callSign));
        strncpy (staticData->callSign, callSign.c_str(), 7);
    }
    else
    {
        target->clearFlag (TargetFlags::CallSign);
    }
}

void AIS::AISTarget::assignString (char *addr, const size_t size, std::string& source, const AIS::TargetFlags flag)
{
    assignString (addr, size, source.c_str (), flag);
}

void AIS::AISTarget::assignString (char *addr, const size_t size, const char *source, const AIS::TargetFlags flag)
{
    bool   missing;
    size_t i;

    memset (addr, 0, size);

    for (i = 0, missing = true; missing && i < size && source[i]; ++i)
    {
        if (source[i] != '@')
            missing = false;
    }

    if (missing)
    {
        clearFlag (flag);
    }
    else
    {
        setFlag (flag);

        strncpy (addr, source, size - 1);
    }
}