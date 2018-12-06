#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include "Sentence.h"
#include "SerialReader.h"
#include "Parameters.h"
#include "Socket.h"
#include "DataStorage.h"

#include "../NaviSensorUI/SensorConfig.h"

using namespace Readers;

namespace Nmea {}

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

            virtual bool readIteration ();

            inline const SensorConfig *getConfig () { return config; }

            inline std::thread& getReader () { return reader; }
            inline std::thread& getProcessor () { return processor; }

            void updateData (Data::DataType, void *data, const size_t size);

            inline void setForwardCallback (ForwardCb cb) { forwardCb = cb; }

            inline bool isAlive () { return alive; }

            void enableRawDataSend (const bool enable, const unsigned int port = 0);
            void enableProcessedDataSend (const bool enable, const unsigned int port = 0);
            void enableSentenceStateSend (const bool enable, const unsigned int port = 0);

        protected:
            ForwardCb               forwardCb;
            bool                    running, done, alive;
            Reader                 *terminal;
            SensorConfig           *config;
            std::thread             reader;
            std::thread             processor;
            std::mutex              locker;
            Comm::Socket            transmitter;
            bool                    sendRawData, sendProcessedData, sendSentenceState;
            unsigned int            rawDataPort, processedDataPort, sentenceStatePort;
            NMEA::SentenceRegistry  sentenceReg;
            Data::SensorDataStorage dataStorage;

            void readerProc ();
            void processorProc ();

            static void readerProcInternal (Sensor *);
            static void processorProcInternal (Sensor *);

            Reader *createTerminal ();

            virtual size_t extractData () { return 0; }
            virtual void processData (size_t size) {}

            void sendSentenceStateData ();
            void forwardProcessedData ();
    };

    #pragma pack(1)

    struct _SensorState
    {
        int  id;
        bool alive, running;
    };

    struct SensorState : public _SensorState
    {
        SensorState (const int id, const bool alive, const bool running)
        {
            this->id      = id;
            this->alive   = alive;
            this->running = running;
        }

        bool operator == (SensorState& another)
        {
            return id == another.id && alive == another.alive && running == another.running;
        }

        bool operator != (SensorState& another)
        {
            return id != another.id || alive != another.alive || running != another.running;
        }
    };

    class SensorStateArray : public std::vector <SensorState>
    {
        public:
            bool operator != (SensorStateArray& another)
            {
                bool result = size() != another.size();

                if (!result)
                {
                    for (int i = 0, count = size (); i < count; ++i)
                    {
                        if (at(i) != another [i])
                        {
                            result = true; break;
                        }
                    }
                }

                return result;
            }
    };

    #pragma pack()

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
            void enableProcessedDataSend (const unsigned int sensorID, const bool enable, const unsigned int port = 0);
            void enableSentenceStateSend (const unsigned int sensorID, const bool enable, const unsigned int port = 0);

            inline const bool isRunning () { return running; }

            SensorStateArray& populateSensorStateArray (SensorStateArray&);

        protected:
            bool running;
    };
}