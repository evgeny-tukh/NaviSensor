#pragma once

#pragma pack(1)

namespace Data
{
    enum DaemonState
    {
        Inactive = 0,
        Stopped  = 1,
        Running  = 2
    };

    struct GenericData {};

    struct Time : GenericData
    {
        unsigned char hour, min, sec;
    };

    struct Pos : GenericData
    {
        Pos (const double lat = 0.0, const double lon = 0.0)
        {
            this->lat = lat;
            this->lon = lon;
        }

        double lat, lon;
    };
}

#pragma pack()
