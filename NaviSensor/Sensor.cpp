#include <chrono>
#include <iostream>
#include "TextSensor.h"

Sensors::Sensor::Sensor (SensorConfig *config) :
    forwardCb (0),
    alive (false),
    done (false),
    running (false),
    sendRawData (false),
    config (0),
    rawDataPort (0),
    reader (readerProcInternal, this), 
    processor (processorProcInternal, this),
    locker ()
{
    this->terminal = 0;
    this->config   = config;

    setForwardCallback ([this] (const char *data, const int size) -> void
                        {
                            if (sendRawData)
                                transmitter.sendTo (data, size, rawDataPort, "127.0.0.1");
                        });
}

Sensors::Sensor::~Sensor ()
{
    if (terminal)
        delete terminal;

    terminate ();
}

bool Sensors::Sensor::readIteration ()
{
    bool result = false;

    if (config)
    {
        if (terminal)
            result = terminal->read () > 0;
    }

    return result;
}

void Sensors::Sensor::terminate ()
{
    done = true;

    reader.join ();
    processor.join ();
}

void Sensors::Sensor::start ()
{
    createTerminal ();

    if (terminal)
        terminal->open ();

    transmitter.create (config->id () + 8080);

    running = true;
}

void Sensors::Sensor::stop ()
{
    transmitter.close ();

    running = false;

    if (terminal)
    {
        terminal->close ();

        delete terminal;

        terminal = 0;
    }
}

void Sensors::Sensor::readerProcInternal (Sensor *sensor)
{
    if (sensor)
        sensor->readerProc ();
}

void Sensors::Sensor::readerProc ()
{
    time_t lastDataAt = 0, now;

    while (!done)
    {
        if (running && locker.try_lock ())
        {
            now = time (0);

            if (readIteration ())
            {
                lastDataAt = time (0);
                alive      = true;
            }
            else if ((now - lastDataAt) > 30)
            {
                alive = false;
            }

            locker.unlock ();
        }

        Tools::sleepFor (config ? config->pauseBtwIter : 10);
    }
}

void Sensors::Sensor::processorProcInternal (Sensor *sensor)
{
    if (sensor)
        sensor->processorProc ();
}

void Sensors::Sensor::processorProc ()
{
    while (!done)
    {
        if (running)
        {
            size_t dataSize;

            while (dataSize = extractData (), dataSize > 0)
            {
                processData (dataSize);

                Tools::sleepFor (5);
            }
        }

        Tools::sleepFor (config ? config->pauseBtwIter : 10);
    }
}

Reader *Sensors::Sensor::createTerminal ()
{
    if (terminal)
        delete terminal;

    switch (config->connection)
    {
        case Connection::Serial:
            terminal = new SerialReader (& config->serialParam); break;

        default:
            terminal = 0;
    }

    return terminal;
}

void Sensors::Sensor::updateData (Data::DataType, void *data, const size_t size)
{

}

void Sensors::Sensor::enableRawDataSend (const bool enable, const unsigned int port)
{
    sendRawData = enable;
    rawDataPort = port;
}

Sensors::SensorArray::SensorArray (SensorConfigArray *sensorConfigs)
{
    this->running       = false;
    this->sensorConfigs = sensorConfigs;
}

Sensors::SensorArray::~SensorArray ()
{
    for (auto & sensor : *this)
    {
        if (sensor)
            delete sensor;
    }
}

void Sensors::SensorArray::createAll ()
{
    for (auto & config : *sensorConfigs)
    {
        Sensor *sensor;

        switch (config->type)
        {
            case NMEA:
            case AIS:
                sensor = new TextSensor (config); break;

            default:
                sensor = new Sensor (config);
        }

        push_back (sensor);
    }
}

void Sensors::SensorArray::startAll ()
{
    for (auto & sensor : *this)
    {
        if (sensor)
            sensor->start ();
    }

    running = true;

    std::cout << "Started." << std::endl;
}

void Sensors::SensorArray::stopAll ()
{
    for (auto & sensor : *this)
    {
        if (sensor)
            sensor->stop ();
    }

    running = false;

    std::cout << "Stopped." << std::endl;
}

void Sensors::SensorArray::deleteBySensorID (const int id, const bool removeCfgFile)
{
    for (iterator pos = begin (); pos != end (); ++ pos)
    {
        Sensor *sensor = *pos;

        if (sensor)
        {
            SensorConfig *config = (SensorConfig *) sensor->getConfig ();

            if (config->sensorID == id)
            {
                if (removeCfgFile)
                    config->deleteCfgFile();

                delete sensor;

                erase (pos);

                break;
            }
        }
    }
}

void Sensors::SensorArray::deleteByIndex (const size_t index, const bool removeCfgFile)
{
    if (index >= 0 && index < size ())
    {
        iterator pos    = begin();
        Sensor  *sensor = at (index);

        if (sensor)
        {
            if (removeCfgFile)
            {
                SensorConfig *config = (SensorConfig *) sensor->getConfig ();

                if (config)
                    config->deleteCfgFile ();
            }

            delete sensor;
        }

        advance (pos, index);

        erase (pos);
    }
}

bool Sensors::SensorArray::allStopped ()
{
    bool result = true;

    for (auto & sensor : *this)
    {
        if (sensor && sensor->getReader().joinable())
        {
            result = false; break;
        }
    }

    return result;
}

void Sensors::SensorArray::wait ()
{
    while (!allStopped())
        std::this_thread::sleep_for (std::chrono::seconds (1));
}

void Sensors::SensorArray::setForwardCallback (const int sensorID, const Sensors::ForwardCb cb)
{
    for (auto & sensor : *this)
    {
        if (sensor && sensor->getConfig ()->sensorID == sensorID)
        {
            sensor->setForwardCallback (cb); break;
        }
    }
}

void Sensors::SensorArray::enableRawDataSend (const unsigned int sensorID, const bool enable, const unsigned int port)
{
    for (auto & sensor : *this)
    {
        if (sensor && sensor->getConfig()->sensorID == sensorID)
        {
            sensor->enableRawDataSend (enable, port); break;
        }
    }
}

Sensors::SensorStateArray& Sensors::SensorArray::populateSensorStateArray (SensorStateArray& sensorStates)
{
    sensorStates.clear ();

    for (auto & sensor : *this)
    {
        if (sensor)
        {
            SensorConfig *config = (SensorConfig *) sensor->getConfig ();

            sensorStates.emplace_back (config->id (), sensor->isAlive (), sensor->isRunning ());
        }
    }

    return sensorStates;
}