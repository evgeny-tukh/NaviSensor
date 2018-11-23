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
        byte hour, min, sec;
    };

    struct Pos : GenericData
    {
        double lat, lon;
    };
}

#pragma pack()
