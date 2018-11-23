#pragma once

#include <map>
#include "../NaviSensorUI//tools.h"
#include "DataDef.h"

namespace Data
{
    enum DataType
    {
        Unknown     = 0,
        UTC         = 1,
        Position    = 2,
        HDOP        = 3,
        GPSQual     = 4,
        TrueHeading = 5,
        Course      = 6,
        SpeedOG     = 7,
        SpeedTW     = 8,
        RateOfTurn  = 9,
        PosSysMode  = 10,

        All         = -1
    };

    enum Quality
    {
        Good       = 1,
        Suspicious = 2,
        Poor       = 0
    };

    enum GPSQuality
    {
        InvalidFix   = 0,
        SPS          = 1,
        Differential = 2,
        PPS          = 3,
        RTK          = 4,
        FloatRTK     = 5,
        Estimated    = 6,
        Manual       = 7,
        Simulator    = 8
    };

    struct Parameter
    {
        size_t       size;
        DataType     type;
        Quality      quality;
        time_t       updateTime;
        GenericData *data;

        Parameter ();
        Parameter (DataType);
        ~Parameter ();

        void update (GenericData *, Data::Quality sourceQualily = Data::Quality::Good);
        void update (GenericData&, Data::Quality sourceQualily = Data::Quality::Good);

        void assign (Parameter& source);
    };

    typedef std::pair <DataType, Parameter *> DataItem;

    class DataStorage : public std::map <DataType, Parameter *>
    {
        public:
            DataStorage (const time_t timeout);
            ~DataStorage ();

            void update (const DataType, GenericData *, Data::Quality sourceQualily = Data::Quality::Good);
            void update (const DataType, GenericData &, Data::Quality sourceQualily = Data::Quality::Good);

            Parameter *find (const DataType);

            void checkExipired ();

        protected:
            time_t timeout;
    };

    const size_t getDataSize (const DataType);
}