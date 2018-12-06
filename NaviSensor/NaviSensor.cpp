#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "Sensor.h"
//#include "DataServer.h"
#include "DataStorage.h"
#include "Network.h"
#include "Socket.h"
#include "../NaviSensorUI/tools.h"

/*void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *param)
{
    Sensors::SensorArray *sensors = (Sensors::SensorArray *) param;
    Tools::SimpleCall     sendCmd = [sendCb] (const byte cmd) -> int
                                    {
                                        return sendCb ((byte *) & cmd, (size_t) sizeof (cmd));
                                    };

    while (true)
    {
        byte command;

        // Wait for command
        while (rcvCb (& command, sizeof (command)) != sizeof (command))
            Tools::sleepFor (10);

        switch (command)
        {
            case Comm::Command::StartAll:
                sendCmd (Comm::Command::Ack); break;

            case Comm::Command::StopAll:
                sendCmd (Comm::Command::Ack); break;

            case Comm::Command::StartInputFw:
            case Comm::Command::StopInputFw:
                sendCmd (Comm::Command::Ack); break;
        }
    }
}*/

void senderProc (Data::GlobalDataStorage *storage, Sensors::SensorArray *sensors, bool *run, bool *sockInitialized)
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
                
            transmitter.sendTo ((const char *) message.buffer, message.heartbeat.size, 8001);
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

void onCommand (Comm::Command *command, Sensors::SensorArray *sensors)
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

            parseCommandParameter(command->argument, sensorID, enable, port);

            sensors->enableSentenceStateSend (sensorID, enable, port); break;
        }
    }
}

void onMessage (const unsigned int msgType, const char *data, const int size, void *param)
{
    Sensors::SensorArray *sensors = (Sensors::SensorArray *) param;

    switch (msgType)
    {
        case Comm::MsgType::Cmd:
            onCommand ((Comm::Command *) (data - sizeof (Comm::GenericMsg)), sensors); break;
    }
}

int main (int argCount, char *args [])
{
    //Comm::DataServer          *dataServer;
    Comm::DataNode            *dataNode;
    Data::GlobalDataStorage    globalData (15);
    Sensors::SensorConfigArray configs (true);
    Sensors::SensorArray       sensors (& configs);
    bool                       run             = true,
                               sockInitialized = false;
    std::thread                sender (senderProc, & globalData, & sensors, & run, & sockInitialized);
    WSADATA                    wsaData;

    printf ("NaviSensor v1.0\n");

    //Comm::DataServer::init ();

    WSAStartup (0x0101, & wsaData);

    sockInitialized = true;
    
    dataNode = new Comm::DataNode ((const unsigned int) Comm::Ports::CmdPort, onMessage, & sensors);

    //dataServer = new Comm::DataServer (logicProc, & sensors);
    
    //dataServer->create ();

    sensors.createAll ();
    sensors.startAll ();
    sensors.wait ();

    //delete dataServer;
    delete dataNode;

    return 0;
}
