#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <functional>
#include "Sensor.h"
#include "DataStorage.h"
#include "Network.h"
#include "Socket.h"
#include "AIS.h"
#include "AISConfig.h"
#include "AISTargetTable.h"
#include "Parser.h"
#include "../NaviSensorUI/tools.h"
#include "Service.h"

#define SERVICE_NAME    "NaviSensorDaemon"

bool globalRun = true;

void mainProc ();

class NaviSensorService : public Service
{
    public:
        NaviSensorService ();
        virtual ~NaviSensorService ();

    protected:
        std::thread *naviSensorThread;

        virtual void onStart (unsigned int numOgArgs = 0, const char **args = 0);
        virtual void onStop ();
};

NaviSensorService::NaviSensorService () : Service ("NaviSensor", "NaviSensor service")
{
    naviSensorThread = 0;
}

NaviSensorService::~NaviSensorService ()
{
    if (naviSensorThread)
    {
        if (naviSensorThread->joinable ())
            naviSensorThread->join ();

        delete naviSensorThread;
    }
}

void NaviSensorService::onStart (unsigned int numOgArgs, const char **args)
{
    globalRun        = true;
    naviSensorThread = new std::thread (mainProc);
}

void NaviSensorService::onStop ()
{
    if (naviSensorThread && naviSensorThread->joinable ())
    {
        globalRun = false;

        naviSensorThread->detach ();
    }
}

void aisDynTargetSenderProc (AIS::AISTargetTable *targetTable, bool *run, bool *sockInitialized)
{
    Comm::Socket transmitter;
    bool         created = false;

    while (*run && globalRun)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create (0, true);
        }

        if (created)
        {
            AIS::AISDynamicDataArray targets;
            unsigned char            buffer [1400];
            AIS::AISDynamicData     *allTargets;
            Comm::AISDynData        *data = (Comm::AISDynData *) buffer;

            data->msgType = Comm::AISDynamic;

            targetTable->extractDynamicData (targets);

            allTargets = targets.data ();

            for (size_t start = 0, maxTrgInPacket = 1400 / sizeof (AIS::AISDynamicData), count = targets.size (); start < count; start += maxTrgInPacket)
            {
                size_t targetsToSend = (start + maxTrgInPacket) <= count ? maxTrgInPacket : count - start;

                data->size = targetsToSend * sizeof (AIS::AISDynamicData);

                memcpy (data->targets, allTargets + start, data->size);

                if (targetsToSend > 0)
                    transmitter.sendTo ((const char *) buffer,  data->size + sizeof (Comm::AISDynData) - sizeof (AIS::AISDynamicData),
                                         Comm::Ports::AISTargetPort);

                Tools::sleepFor (5);
            }
        }

        Tools::sleepFor (5000);
    }

    transmitter.close();
}

void aisStaticTargetSenderProc (AIS::AISTargetTable *targetTable, bool *run, bool *sockInitialized)
{
    Comm::Socket transmitter;
    bool         created = false;

    while (*run && globalRun)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create(0, true);
        }

        if (created)
        {
            AIS::AISStaticDataArray targets;
            unsigned char           buffer[1400];
            AIS::AISStaticData     *allTargets;
            Comm::AISStaticData    *data = (Comm::AISStaticData *) buffer;

            data->msgType = Comm::AISStatic;

            targetTable->extractStaticData (targets);

            allTargets = targets.data();

            for (size_t start = 0, maxTrgInPacket = 1400 / sizeof (AIS::AISStaticData), count = targets.size (); start < count; start += maxTrgInPacket)
            {
                size_t targetsToSend = (start + maxTrgInPacket) <= count ? maxTrgInPacket : count - start;

                data->size = targetsToSend * sizeof (AIS::AISStaticData);

                memcpy (data->targets, allTargets + start, data->size);

                if (targetsToSend > 0)
                    transmitter.sendTo ((const char *)buffer, data->size + sizeof(Comm::AISStaticData) - sizeof(AIS::AISStaticData),
                                        Comm::Ports::AISTargetPort);

                Tools::sleepFor (5);
            }
        }

        Tools::sleepFor (60000);
    }

    transmitter.close ();
}

void processedDataSenderProc (Data::GlobalDataStorage *storage, Sensors::SensorArray *sensors, bool *run, bool *sockInitialized)
{
    Comm::Socket transmitter;
    bool         created = false;

    while (*run && globalRun)
    {
        if (!created && *sockInitialized)
        {
            created = true;

            created = transmitter.create (0, true);
        }

        if (created)
        {
            Data::DataBuffer        buffer;
            Data::GlobalParamArray  globalParams;
            Data::SimpleProtoBuffer simpleProtoBuffer;

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

                simpleProtoBuffer.addData (param);
            }

            size_t size = buffer.size ();

            if (size > 0)
                transmitter.sendTo ((const char *) buffer.data (), size, Comm::Ports::ProcessedDataPort);

            // always to be sent even there is no data
            transmitter.sendTo (simpleProtoBuffer.getBuffer (), simpleProtoBuffer.getBufferSize (), Comm::Ports::SimpleProtocolPort);
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

    while (*run && globalRun)
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

void onCommand (Comm::Command *command, Sensors::SensorArray *sensors, Data::GlobalDataStorage *storage, AIS::AISTargetTable *targets)
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

        case Comm::CmdType::ReapplyAISFilter:
        {
            targets->loadFiltering (); break;
        }
    }
}

void onMessage (const unsigned int msgType, const char *data, const int size, void *param, void *param2, void *param3)
{
    Sensors::SensorArray    *sensors = (Sensors::SensorArray *) param;
    Data::GlobalDataStorage *storage = (Data::GlobalDataStorage *) param2;
    AIS::AISTargetTable     *targets = (AIS::AISTargetTable *) param3;

    switch (msgType)
    {
        case Comm::MsgType::Cmd:
            onCommand ((Comm::Command *) (data - sizeof (Comm::GenericMsg)), sensors, storage, targets); break;
    }
}

void mainProc ()
{
    Data::Pos                  curPosition (10000.0, 10000.0);
    AIS::AISTargetTable        aisTargets (60, & curPosition);
    Parsers::NmeaParsers       parsers (& aisTargets);
    Comm::DataNode            *dataNode;
    Data::GlobalDataStorage    globalData (15, & curPosition);
    Sensors::SensorConfigArray configs (true);
    Sensors::SensorArray       sensors (& configs);
    bool                       run = true, sockInitialized = false;
    std::thread                sensorStateSender (sensorStateSenderProc, & sensors, & run, & sockInitialized);
    std::thread                processedDataSender (processedDataSenderProc, & globalData, & sensors, & run, & sockInitialized);
    std::thread                aisDynTargetSender (aisDynTargetSenderProc, & aisTargets, & run, & sockInitialized);
    std::thread                aisStaticTargetSender (aisStaticTargetSenderProc, & aisTargets, & run, & sockInitialized);
    WSADATA                    wsaData;

    printf ("NaviSensor v1.0\n");

    WSAStartup (0x0101, & wsaData);

    sockInitialized = true;

    dataNode = new Comm::DataNode ((const unsigned int) Comm::Ports::CmdPort, onMessage, & sensors, & globalData);

    sensors.createAll ();
    sensors.startAll ();
    sensors.wait ();

    sensorStateSender.join ();
    processedDataSender.join ();
    aisDynTargetSender.join ();
    aisStaticTargetSender.join ();

    delete dataNode;
}

void installService ()
{
    NaviSensorService service;

    service.install (true);
}

void uninstallService ()
{
    NaviSensorService service;

    service.uninstall (true);
}

void startService ()
{
    NaviSensorService service;

    service.startStop (true);
}

void stopService ()
{
    NaviSensorService service;

    service.startStop (false);
}

void runService()
{
    NaviSensorService service;

    service.run ();
}

int main (int argCount, char *args [])
{
    bool                                         install = false, uninstall = false, start = false, stop = false;
    std::function <bool (const char, const int)> checkArg = [args](const char argChar, const int index) -> bool
                                                            {
                                                                return toupper (args [index][1]) == argChar &&
                                                                       (args [index][0] == '-' || args [index][0] == '/');
                                                            };

    if (argCount > 1)
    {
        for (int i = 1; i < argCount; ++i)
        {
            if (checkArg ('D', i))
            {
                mainProc ();
                exit (0);
            }
            else if (checkArg ('I', i))
            {
                install = true;
            }
            else if (checkArg ('U', i))
            {
                uninstall = true;
            }
            else if (checkArg ('R', i))
            {
                start = true;
            }
            else if (checkArg ('S', i))
            {
                stop = true;
            }
        }
    }

    if (install)
        installService ();
    else if (uninstall)
        uninstallService ();
    else if (start)
        startService ();
    else if (stop)
        stopService ();
    else
        runService ();

    return 0;
}
