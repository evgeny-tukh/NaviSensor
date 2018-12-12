#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "Sensor.h"
#include "DataStorage.h"
#include "Network.h"
#include "Socket.h"
#include "AIS.h"
#include "AISTargetTable.h"
#include "Parser.h"
#include "../NaviSensorUI/tools.h"

void processedDataSenderProc (Data::GlobalDataStorage *storage, Sensors::SensorArray *sensors, bool *run, bool *sockInitialized)
{
    Comm::Socket transmitter;
    bool         created = false;

    while (*run)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create (0, true);
        }

        if (created)
        {
            Data::DataBuffer       buffer;
            Data::GlobalParamArray globalParams;

            // Polling sensor storage
            for (auto & sensor : *sensors)
            {
                Data::ParamArray       params;
                Sensors::SensorConfig *cfg      = (Sensors::SensorConfig *) sensor->getConfig ();
                const int              sensorID = cfg->id ();

                sensor->extractParameters (params);

                for (auto & param : params)
                    storage->update (sensorID, *param);
            }

            // Prepare to send out
            storage->extractAll (globalParams);

            for (auto & param : globalParams)
            {
                const char          *sensorName = sensors->sensorNameByID (param->sensorID);
                Data::LANParamHeader dest (*param, param->sensorID, param->master, sensorName);

                buffer.addData (& dest, sizeof (dest));
                buffer.addData (param->data, param->size);
            }

            size_t size = buffer.size ();

            if (size > 0)
                transmitter.sendTo ((const char *) buffer.data (), size, Comm::Ports::ProcessedDataPort);
        }

        Tools::sleepFor (1000);
    }

    transmitter.close();
}

void sensorStateSenderProc (Sensors::SensorArray *sensors, bool *run, bool *sockInitialized)
{
    Comm::Socket              transmitter;
    bool                      created = false;
    Sensors::SensorStateArray sensorsState;

    #pragma pack(1)

    union
    {
        Comm::HeartbeatData heartbeat;
        byte                buffer [2000];
    }
    message;

    #pragma pack()

    while (*run)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create (0, true);
        }

        if (created)
        {
            sensors->populateSensorStateArray (sensorsState);

            message.heartbeat.msgType      = Comm::MsgType::Heartbeat;
            message.heartbeat.daemonState  = (byte) sensors->isRunning() ? Data::DaemonState::Running : Data::DaemonState::Stopped;
            message.heartbeat.numOfSensors = (byte) sensors->size ();
            message.heartbeat.size         = sizeof (message.heartbeat) + sizeof (Sensors::_SensorState) * (message.heartbeat.numOfSensors - 1);
            
            Sensors::SensorStateArray::iterator state;
            int                                 i;

            for (i = 0, state = sensorsState.begin (); state != sensorsState.end (); ++ i, ++ state)
                message.heartbeat.sensorState [i] = *state;
                
            transmitter.sendTo ((const char *) message.buffer, message.heartbeat.size, Comm::Ports::SensorPort);
        }

        Tools::sleepFor (1000);
    }

    transmitter.close ();
}

void parseCommandParameter (unsigned int argument, unsigned char& sensorID, unsigned char& enable, unsigned short& port)
{
    sensorID = argument >> 24,
    enable   = (argument & 0xFF0000) >> 16;
    port     = argument & 0xFFFF;
}

void parseCommandParameter (unsigned int argument, unsigned short& sensorID, unsigned short& paramType)
{
    sensorID  = argument >> 16,
    paramType = argument & 0xFFFF;
}

void onCommand (Comm::Command *command, Sensors::SensorArray *sensors, Data::GlobalDataStorage *storage)
{
    switch (command->command)
    {
        case Comm::CmdType::Start:
        {
            sensors->startAll (); break;
        }

        case Comm::CmdType::Stop:
        {
            sensors->stopAll (); break;
        }

        case Comm::CmdType::RawDataCtl:
        {
            unsigned char  sensorID, enable;
            unsigned short port;

            parseCommandParameter (command->argument, sensorID, enable, port);

            sensors->enableRawDataSend (sensorID, enable, port); break;
        }

        case Comm::CmdType::ProcessedDataCtl:
        {
            unsigned char  sensorID, enable;
            unsigned short port;

            parseCommandParameter (command->argument, sensorID, enable, port);

            sensors->enableProcessedDataSend (sensorID, enable, port); break;
        }

        case Comm::CmdType::SentenceListCtl:
        {
            unsigned char  sensorID, enable;
            unsigned short port;

            parseCommandParameter (command->argument, sensorID, enable, port);

            sensors->enableSentenceStateSend (sensorID, enable, port); break;
        }

        case Comm::CmdType::SelectMasterParam:
        {
            unsigned short sensorID, paramType;

            parseCommandParameter (command->argument, sensorID, paramType);
            
            storage->assignMasterSource (sensorID, (Data::DataType) paramType); break;
        }
    }
}

void onMessage (const unsigned int msgType, const char *data, const int size, void *param, void *param2)
{
    Sensors::SensorArray    *sensors = (Sensors::SensorArray *) param;
    Data::GlobalDataStorage *storage = (Data::GlobalDataStorage *) param2;

    switch (msgType)
    {
        case Comm::MsgType::Cmd:
            onCommand ((Comm::Command *) (data - sizeof (Comm::GenericMsg)), sensors, storage); break;
    }
}

int main (int argCount, char *args [])
{
    //Comm::DataServer          *dataServer;
    AIS::AISTargetTable        aisTargets (60);
    Parsers::NmeaParsers       parsers (& aisTargets);
    Comm::DataNode            *dataNode;
    Data::GlobalDataStorage    globalData (15);
    Sensors::SensorConfigArray configs (true);
    Sensors::SensorArray       sensors (& configs);
    bool                       run             = true,
                               sockInitialized = false;
    std::thread                sensorStateSender (sensorStateSenderProc, & sensors, & run, & sockInitialized);
    std::thread                processedDataSender (processedDataSenderProc, & globalData, & sensors, & run, & sockInitialized);
    WSADATA                    wsaData;

    printf ("NaviSensor v1.0\n");

    //Comm::DataServer::init ();

    WSAStartup (0x0101, & wsaData);

    sockInitialized = true;
    
    dataNode = new Comm::DataNode ((const unsigned int) Comm::Ports::CmdPort, onMessage, & sensors, & globalData);

    //dataServer = new Comm::DataServer (logicProc, & sensors);
    
    //dataServer->create ();

    sensors.createAll ();
    sensors.startAll ();
    sensors.wait ();

    sensorStateSender.join ();
    processedDataSender.join ();

    delete dataNode;

    return 0;
}
