#pragma once

#include "Sensor.h"

namespace Sensors
{
    class TextSensor : public Sensor
    {
        public:
            TextSensor (SensorConfig *config);

        protected:
            char sentenceBuffer [100];

            virtual size_t extractData ();

            void cleanUpSentenceBuffer ();
    };
}