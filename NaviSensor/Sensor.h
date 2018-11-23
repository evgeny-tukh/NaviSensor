#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include "SerialReader.h"
#include "Parameters.h"
#include "Socket.h"

#include "../NaviSensorUI/SensorConfig.h"

using namespace Readers;

namespace Sensors
{
    typedef std::function <void (const char *, const size_t)> ForwardCb;

    class Sensor
    {
        public:
            Sensor (SensorConfig *config);
            virtual ~Sensor ();

            void start ();
            void stop ();

            inline const bool isRunning () { return running; }

            void terminate ();

            virtual void readIteration ();

            inline const SensorConfig *getConfig () { return config; }

            inline std::thread& getReader () { return reader; }
            inline std::thread& getProcessor () { return processor; }

            void updateData (Data::DataType, void *data, const size_t size);

            inline void setForwardCallback (ForwardCb cb) { forwardCb = cb; }

            void enableRawDataSend (const bool enable, const unsigned int port = 0);

        protected:
            ForwardCb     forwardCb;
            bool          running, done;
            Reader       *terminal;
            SensorConfig *config;
            std::thread   reader;
            std::thread   processor;
            std::mutex    locker;
            Comm::Socket  transmitter;

            void readerProc ();
            void processorProc ();

            static void readerProcInternal (Sensor *);
            static void processorProcInternal (Sensor *);

            Reader *createTerminal ();

            virtual size_t extractData () { return 0; }
            virtual void processData (size_t size) {}
    };

    class SensorArray : public std::vector <Sensor *>
    {
        public:
            SensorArray (SensorConfigArray *sensorConfigs);
            virtual ~SensorArray ();

            void createAll ();
            void startAll ();
            void stopAll ();

            void deleteBySensorID (const int id, const bool removeCfgFile = false);
            void deleteByIndex (const size_t index, const bool removeCfgFile = false);

            void setForwardCallback (const int sensorID, const ForwardCb cb);

            SensorConfigArray *sensorConfigs;

            void wait ();
            bool allStopped ();

            void enableRawDataSend (const unsigned int sensorID, const bool enable, const unsigned int port = 0);

            inline const bool isRunning () { return running; }

        protected:
            bool running;
    };
}