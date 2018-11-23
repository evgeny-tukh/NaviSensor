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
    Comm::Socket transmitter;
    bool         created    = false;
    byte         heartbreak = 0xFF;

    while (*run)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create (0, true);
        }

        if (created)
        {
            heartbreak = (byte) sensors->isRunning () ? Data::DaemonState::Running : Data::DaemonState::Stopped;

            transmitter.sendTo ((const char *) & heartbreak, sizeof (heartbreak), 8001);
        }

        Tools::sleepFor (1000);
    }

    transmitter.close ();
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
            unsigned char  sensorID = command->argument >> 24,
                           enable   = (command->argument && 0xFF0000) >> 16;
            unsigned short port     = command->argument && 0xFFFF;

            sensors->enableRawDataSend (sensorID, enable, port); break;
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
