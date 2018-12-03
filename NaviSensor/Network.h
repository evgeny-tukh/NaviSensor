#pragma once

#include <time.h>
#include <thread>
#include "Socket.h"
#include "Sensor.h"

namespace Comm
{
    enum Ports
    {
        CmdPort                = 8080,
        SensorPort             = 9080,
        RawDataFirstPort       = 7080,
        SentenceStateFirstPort = 6080,
        ProcessedDataPort      = 8001
    };

    enum MsgType
    {
        Heartbeat     = 1,
        Cmd           = 2,
        Ack           = 3,
        Nak           = 4,
        Sensors       = 5,
        RawData       = 6,
        ProcessedData = 7,
        SentenceList  = 8
    };

    enum CmdType
    {
        Start           = 1,
        Stop            = 2,
        RawDataCtl      = 3,
        SentenceListCtl = 4
    };

    #pragma pack(1)

    struct GenericMsg
    {
        unsigned int  size;
        unsigned char msgType;
    };

    struct HeartbeatData : GenericMsg
    {
        unsigned char         daemonState, numOfSensors;
        Sensors::_SensorState sensorState [1];
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
        unsigned int  seqNumber, argument;
    };

    struct Acknowledge : GenericMsg
    {
        unsigned char command;
        unsigned int  seqNumber;
    };

    struct Raw : GenericMsg
    {
        char data [1];
    };

    struct SentenceList : GenericMsg
    {
        int                  numOfSentences;
        NMEA::SentenceStatus sentenceStatus [1];
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

    class DataNode : public Socket
    {
        public:
            typedef std::function <void (MsgType msgType, const char *data, const int size, void *param)> MsgReadCb;

            DataNode (const unsigned int port, MsgReadCb readCb, void *param = 0);
            virtual ~DataNode ();

            void sendMessage (MsgType msgType, byte *data, const int dataSize, const unsigned int port, const char *destAddr = 0);
            unsigned int sendCommand (CmdType cmd, const unsigned int arg = 0, const unsigned int port = Ports::CmdPort, const char *destAddr = 0);
            unsigned int sendCommand (CmdType cmd, const unsigned short arg1, const unsigned short arg2, const unsigned int port = Ports::CmdPort, const char *destAddr = 0);
            unsigned int sendCommand (CmdType cmd, const unsigned char arg1, const unsigned char arg2, const unsigned short arg3, const unsigned int port = Ports::CmdPort, const char *destAddr = 0);

        protected:
            unsigned int port;
            std::thread  worker;
            bool         active;

            void workerProc (MsgReadCb readCb, void *param);
            static void workerProcInternal (MsgReadCb readCb, void *param, DataNode *self);
    };
}