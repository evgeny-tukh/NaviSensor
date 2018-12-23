#pragma once

#include <string>

namespace AIS
{
    class SixBitStorage;

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
        RateOfTurnOutOfScale = 0x10,
        SOG                  = 0x20,
        SOGOutOfScale        = 0x40,
        HDG                  = 0x80,
        HDGOutOfScale        = 0x100,
        Lat                  = 0x200,
        Lon                  = 0x400,
        CallSign             = 0x800,
        IMONumber            = 0x1000,
        Name                 = 0x2000,
        Destination          = 0x4000,
        ShipCargoType        = 0x8000,
        ETA                  = 0x10000,
        DTE                  = 0x20000,
        AtoN                 = 0x40000,
        OffPosition          = 0x80000,
        AtoNType             = 0x100000,
        ClassB               = 0x200000
    };

    #pragma pack(1)
    struct AISDynamic
    {
        double lat, lon;

        union
        {
            struct
            {
                unsigned short year;
                unsigned char  month, day, hour, min, sec;
            };

            struct
            {
                unsigned char   navStatus;
                float           rateOfTurn, sog, hdg;
            };
        };
    };

    struct AISDynamicData
    {
        unsigned int mmsi, flags;
        AISDynamic   data;

        AISDynamicData ()
        {
            memset (this, 0, sizeof (*this));
        }

        AISDynamicData (unsigned int mmsi, unsigned int flags, AISDynamic& source)
        {
            this->mmsi  = mmsi;
            this->flags = flags;

            memcpy (& data, & source, sizeof (data));
        }
    };

    struct AISStatic
    {
        unsigned int   imoNumber;
        char           callSign [8], name [21], destination [21];
        unsigned char  shipCargoType, dimStbd, dimPort, psType;
        unsigned short dimAhead, dimAstern;

        union
        {
            time_t        eta;
            unsigned char aToNType;
        };
    };

    struct AISStaticData
    {
        unsigned int mmsi, flags;
        AISStatic    data;

        AISStaticData ()
        {
            memset (this, 0, sizeof (*this));
        }

        AISStaticData (unsigned int mmsi, unsigned int flags, AISStatic& source)
        {
            this->mmsi  = mmsi;
            this->flags = flags;

            memcpy (& data, & source, sizeof (data));
        }
    };

    struct AISTarget
    {
        unsigned int mmsi, flags;

        AISDynamic dynamicData;
        AISStatic  staticData;

        AISTarget ()
        {
            memset (this, 0, sizeof (*this));
        }

        AISTarget (const unsigned int mmsi)
        {
            memset (this, 0, sizeof (*this));

            this->mmsi = mmsi;
        }

        inline void setFlag (const TargetFlags flag)
        {
            flags |= flag;
        }

        inline void clearFlag (const TargetFlags flag)
        {
            flags &= ~flag;
        }

        void assignString (char *addr, const size_t size, const char *source, const TargetFlags flag);
        void assignString (char *addr, const size_t size, std::string& source , const TargetFlags flag);
    };
    #pragma pack()

    const char *navStatusName (const NavStatus);

    void parseBaseReport (AIS::AISTarget *target, SixBitStorage& data);
    void parseBaseStationReport (AIS::AISTarget *target, SixBitStorage& data);
    void parseStaticVoyageReport (AIS::AISTarget *target, SixBitStorage& data);
    void parseStandardClassBPosReport (AIS::AISTarget *target, SixBitStorage& data);
    void parseExtendedClassBPosReport (AIS::AISTarget *target, SixBitStorage& data);
    void parseAidsToNavigationReport (AIS::AISTarget *target, SixBitStorage& data);
}