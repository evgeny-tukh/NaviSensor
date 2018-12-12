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
    unsigned char  navStatus  = data.getByte (4),
                   rateOfTurn = data.getByte (8);
    unsigned short sog        = data.getShort (10);
    bool           accuracy   = data.getByte (1) != 0;
    int            lon        = (signed) data.getInt (28, true),
                   lat        = (signed) data.getInt (27, true);
    unsigned short cog        = data.getShort (12),
                   hdg        = data.getShort (9);
    unsigned char  timestamp  = data.getByte (6),
                   regional   = data.getByte (4),
                   spare      = data.getByte (1);
    bool           raimFlag   = data.getByte (1) != 0;
    unsigned int   commState  = data.getInt (19);

    if (accuracy)
        target->flags |= TargetFlags::PositionAccuracy;
    else
        target->flags &= ~TargetFlags::PositionAccuracy;

    if (raimFlag)
        target->flags |= TargetFlags::RAIM;
    else
        target->flags &= ~TargetFlags::RAIM;

    if (rateOfTurn != 0x80)
    {
        target->flags |= TargetFlags::RateOfTurn;

        if (rateOfTurn = 0x7F)
        {
            target->rateOfTurn = 720.0f;
            target->flags     |= TargetFlags::RateOfTurnOutOfScale;
        }
        else if (rateOfTurn = 0x81)
        {
            target->rateOfTurn = -720.0f;
            target->flags     |= TargetFlags::RateOfTurnOutOfScale;
        }
        else
        {
            target->rateOfTurn = (float) (4.733 * sqrt ((double) rateOfTurn));
            target->flags     &= ~TargetFlags::RateOfTurnOutOfScale;
        }
    }
    else
    {
        target->flags &= ~TargetFlags::RateOfTurn;
    }

    if (sog != 1023)
    {
        target->flags |= TargetFlags::SOG;

        if (sog == 1022)
        {
            target->sog    = 102.2f;
            target->flags |= SOGOutOfScale;
        }
        else
        {
            target->sog    = (float) ((double) sog * 0.1);
            target->flags &= ~SOGOutOfScale;
        }
    }
    else
    {
        target->flags &= ~TargetFlags::SOG;
    }

    if (hdg != 511)
    {
        target->flags |= TargetFlags::HDG;
        target->flags &= ~HDGOutOfScale;
        target->hdg    = (float) hdg;
    }
    else
    {
        target->flags &= ~TargetFlags::HDG;
    }

    if (lon != 0x6791AC0)
    {
        target->flags |= TargetFlags::Lon;
        target->lon = (double) lon / 600000.0;

    }
    else
    {
        target->flags &= ~TargetFlags::Lon;
    }

    if (lat != 0x3412140)
    {
        target->flags |= TargetFlags::Lat;
        target->lat = (double) lat / 600000.0;
    }
    else
    {
        target->flags &= ~TargetFlags::Lat;
    }
}

void AIS::parseBaseStationReport (AIS::AISTarget *target, SixBitStorage& data)
{
    target->flags |= TargetFlags::BaseStation;

    unsigned short year      = data.getShort (14);
    unsigned char  month     = data.getByte (4),
                   day       = data.getByte (5),
                   hour      = data.getByte (5),
                   min       = data.getByte (6),
                   sec       = data.getByte (6);
    bool           accuracy  = data.getByte (1) != 0;
    int            lon       = (signed) data.getInt (28, true),
                   lat       = (signed) data.getInt (27, true);
    unsigned char  psType    = data.getByte (4);
    unsigned short spare     = data.getShort (10);
    bool           raimFlag  = data.getByte (1) != 0;
    unsigned int   commState = data.getInt (19);

    target->year   = year;
    target->month  = month;
    target->day    = day;
    target->hour   = hour;
    target->min    = min;
    target->sec    = sec;
    target->psType = psType;

    if (accuracy)
        target->flags |= TargetFlags::PositionAccuracy;
    else
        target->flags &= ~TargetFlags::PositionAccuracy;

    if (raimFlag)
        target->flags |= TargetFlags::RAIM;
    else
        target->flags &= ~TargetFlags::RAIM;

    if (lon != 0x6791AC0)
    {
        target->flags |= TargetFlags::Lon;
        target->lon = (double)lon / 600000.0;
    }
    else
    {
        target->flags &= ~TargetFlags::Lon;
    }

    if (lat != 0x3412140)
    {
        target->flags |= TargetFlags::Lat;
        target->lat = (double)lat / 600000.0;
    }
    else
    {
        target->flags &= ~TargetFlags::Lat;
    }
}
