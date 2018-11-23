#pragma once

#include <time.h>

namespace Comm
{
    enum MsgType
    {
        Heartbeat     = 1,
        Command       = 2,
        Ack           = 3,
        Nak           = 4,
        Sensors       = 5,
        Command       = 6,
        RawData       = 7,
        ProcessedData = 8
    };

    #pragma pack(1)

    struct GenericMsg
    {
        unsigned int  size;
        unsigned char msgType;
    };

    struct Heartbeat : GenericMsg
    {
        unsigned char daemonState;
    };

    struct SensorsState : GenericMsg
    {
        struct SensorState
        {
            unsigned char id;
            time_t        lastUpdate;
        }
        sensorStates [1];
    };

    struct Command : GenericMsg
    {
        unsigned char command;
    };

    struct RawData : GenericMsg
    {
        char data [1];
    };

    struct ProcessedData : GenericMsg
    {
        struct ParsedParam
        {
            unsigned int  size;
            unsigned char type, quality, data [1];
        };
    };

    #pragma pack()
}