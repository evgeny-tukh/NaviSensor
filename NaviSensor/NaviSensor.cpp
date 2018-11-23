#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "Sensor.h"
#include "DataServer.h"
#include "DataStorage.h"
#include "Socket.h"
#include "../NaviSensorUI/tools.h"

void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *param)
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
}

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

int main (int argCount, char *args [])
{
    Comm::DataServer          *dataServer;
    Data::GlobalDataStorage    globalData (15);
    Sensors::SensorConfigArray configs (true);
    Sensors::SensorArray       sensors (& configs);
    bool                       run             = true,
                               sockInitialized = false;
    std::thread                sender (senderProc, & globalData, & sensors, & run, & sockInitialized);

    printf ("NaviSensor v1.0\n");

    Comm::DataServer::init ();

    sockInitialized = true;
    
    dataServer = new Comm::DataServer (logicProc, & sensors);
    
    dataServer->create ();

    sensors.createAll ();
    sensors.startAll ();
    sensors.wait ();

    delete dataServer;

    return 0;
}
