#include "TextSensor.h"
#include "Sentence.h"
#include "Parser.h"

Sensors::TextSensor::TextSensor(SensorConfig *config) : Sensor (config)
{
    cleanUpSentenceBuffer ();
}

void Sensors::TextSensor::cleanUpSentenceBuffer ()
{
    memset (sentenceBuffer, 0, sizeof (sentenceBuffer));
}

size_t Sensors::TextSensor::extractData ()
{
    size_t size = 0;

    if (terminal)
    {
        size = terminal->getData (sentenceBuffer, sizeof (sentenceBuffer), "\n");

        if (size > 0 && sentenceBuffer [size-2] == '\r')
            sentenceBuffer [(size--)-2] = '\0';

        if (size > 0)
        {
            NMEA::Sentence sentence (sentenceBuffer);

            if (forwardCb)
                forwardCb (sentenceBuffer, strlen (sentenceBuffer));

            if (Parsers::parsers.parse (sentence, this))
            {

            }
        }
    }

    return size;
}