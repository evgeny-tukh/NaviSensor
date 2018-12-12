#pragma once

#include "SixBitStorage.h"

namespace AIS
{
    enum NavStatus
    {
        UnderWayUsingEngine       = 0,
        AtAnchor                  = 1,
        NotUnderCommand           = 2,
        RestrictedManoeuvrability = 3,
        ConstrainedByDraft        = 4,
        Moored                    = 5,
        Aground                   = 6,
        EngagedInFinishing        = 7,
        UnderWaySailing           = 8,
        ReservedHSC               = 9,
        ReservedWIG               = 10,
        ReservedForFuture1        = 11,
        ReservedForFuture2        = 12,
        ReservedForFuture3        = 13,
        ReservedForFuture4        = 14,
        Undefined                 = 15
    };

    enum TargetFlags
    {
        PositionAccuracy     = 1,
        RAIM                 = 2,
        BaseStation          = 4,

        // Availability below
        RateOfTurn           = 8,
        RateOfTurnOutOfScale = 16,
        SOG                  = 32,
        SOGOutOfScale        = 64,
        HDG                  = 128,
        HDGOutOfScale        = 256,
        Lat                  = 512,
        Lon                  = 1024
    };

    #pragma pack(1)
    struct AISTarget
    {
        unsigned int    mmsi, flags;
        double          lat, lon;

        union
        {
            struct
            {
                unsigned short year;
                unsigned char month, day, hour, min, sec, psType;
            };

            struct
            {
                unsigned char   navStatus;
                float           rateOfTurn, sog, hdg;
            };
        };

        AISTarget ()
        {
            memset (this, 0, sizeof (*this));
        }

        AISTarget(const unsigned int mmsi)
        {
            memset (this, 0, sizeof (*this));

            this->mmsi = mmsi;
        }
    };
    #pragma pack()

    const char *navStatusName (const NavStatus);

    void parseBaseReport (AIS::AISTarget *target, SixBitStorage& storage);
    void parseBaseStationReport (AIS::AISTarget *target, SixBitStorage& storage);
}