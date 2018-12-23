#pragma once

#include <map>
#include <mutex>
#include "../NaviSensorUI//tools.h"
#include "DataDef.h"

namespace Data
{
    enum DataType
    {
        Unknown     = 0,
        UTC         = 1,
        Position    = 2,
        Lat         = 3,
        Lon         = 4,
        HDOP        = 5,
        GPSQual     = 6,
        TrueHeading = 7,
        Course      = 8,
        SpeedOG     = 9,
        SpeedTW     = 10,
        RateOfTurn  = 11,
        PosSysMode  = 12,
        DepthBK     = 13,
        DepthBT     = 14,
        DepthBS     = 15,

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

    enum PosSystemMode
    {
        Autonomous   = 'A',
        Diff         = 'D',
        Estim        = 'E',
        Man          = 'M',
        Simul        = 'S',
        Invalid      = 'N'
    };

    #pragma pack(1)

    struct ParamHeader
    {
        size_t   size;
        DataType type;
        Quality  quality;

        ParamHeader ()
        {
            size    = 0;
            type    = DataType::Unknown;
            quality = Quality::Poor;
        }

        ParamHeader (ParamHeader& source)
        {
            *this = source;
        }

        ParamHeader (ParamHeader *source)
        {
            *this = *source;
        }

        ParamHeader (size_t size, DataType type, Quality quality)
        {
            this->size    = size;
            this->type    = type;
            this->quality = quality;
        }

        ParamHeader& operator = (ParamHeader& source)
        {
            this->size    = source.size;
            this->quality = source.quality;
            this->type    = source.type;

            return *this;
        }

        struct Parameter;
    };

    struct Parameter : ParamHeader
    {
        time_t       updateTime;
        GenericData *data;

        Parameter ();
        Parameter (DataType);
        Parameter (ParamHeader&);
        virtual ~Parameter ();

        void update (GenericData *, Data::Quality sourceQualily = Data::Quality::Good);
        void update (GenericData&, Data::Quality sourceQualily = Data::Quality::Good);

        void assign (Parameter& source);

        const ParamHeader& getHeader ()
        {
            return *((ParamHeader *) this);
        }
    };

    struct LANParamHeader : ParamHeader
    {
        unsigned char sensorID, master;
        char          sensorName [20];

        LANParamHeader () : ParamHeader ()
        {
            sensorID = master = 0;
            
            memset (sensorName, 0, sizeof (sensorName));
        }

        LANParamHeader (ParamHeader& source, const int sensorID = 0, const bool master = true, const char *sensorName = 0) : ParamHeader (source)
        {
            this->sensorID = sensorID;
            this->master   = master ? 1 : 0;

            if (sensorName)
                strncpy (this->sensorName, sensorName, sizeof (this->sensorName));
            else
                memset (this->sensorName, 0, sizeof (this->sensorName));
        }

        LANParamHeader (LANParamHeader& source) : ParamHeader()
        {
            *this = source;
        }
    };

    struct GlobalParameter : Parameter
    {
        int  sensorID;
        bool master;

        GlobalParameter ();
        GlobalParameter (const int, const bool);
        GlobalParameter (const int, const bool, DataType);
        GlobalParameter (const int, const bool, Parameter&);

        void init (const int sensorID = 0, const bool master = true);
    };

    struct ReceivedParameter : GlobalParameter
    {
        char sensorName [20];
    };

    struct DisplParam : Parameter
    {
        DisplParam (Parameter& param)
        {
            assign (param);

            item = -1;
        }

        int item;
    };

    #pragma pack()

    class DisplayedParams : public std::map <DataType, DisplParam>
    {
        public:
            void checkAdd (Parameter&);

            inline void lock () { locker.lock (); }
            inline void unlock () { locker.unlock (); }

        protected:
            std::mutex locker;
    };


    typedef std::pair <DataType, Parameter *> DataItem;

    class DataBuffer : public std::vector <unsigned char>
    {
        public:
            void addData (void *data, const size_t size)
            {
                unsigned char *byteData = (unsigned char *) data;

                for (size_t i = 0; i < size; ++ i)
                    push_back (byteData [i]);
            }

            void addString (const char *data, const size_t size)
            {
                size_t i;
                bool   eolPassed;

                for (i = 0, eolPassed = false; i < size; ++ i)
                {
                    if (!eolPassed && !(data [i]))
                        eolPassed = true;

                    push_back (eolPassed ? '\0' : data [i]);
                }
            }
    };

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

    #pragma pack(1)
    
    #define NSSP_SIGNATURE      "NSSP"
    #define NSSP_MAJ_VER        1
    #define NSSP_MIN_VER        0

    struct SimpleProtoItem
    {
        unsigned short dataType;
        double         value;
    };

    struct SimpleProtoPacket
    {
        char            signature [4];                      // Always NSSP_SIGNATURE
        unsigned char   protoVerMajor, protoVerMinor;
        unsigned short  numOfItems;
        SimpleProtoItem items [1];

        static SimpleProtoPacket *init (void *buffer, const unsigned short numOfItems = 0);

        std::string formatData (const size_t index);

        void addData (const DataType, const GenericData *);
        void addData (const DataType, const double);
    };

    class SimpleProtoBuffer
    {
        public:
            SimpleProtoBuffer ();
            virtual ~SimpleProtoBuffer ();

            inline char *getBuffer () { return buffer; }
            const size_t getBufferSize ();
            const size_t getNumOfItems ();

            void addData (const DataType, const GenericData *);
            void addData (const DataType, const double);
            void addData (const GlobalParameter *);

            SimpleProtoItem *getData (const size_t index);

            inline std::string formatData (const size_t index) { return packet ? packet->formatData (index) : ""; }

            void fromBuffer (const char *data, const size_t size);

    protected:
            std::vector <DataType> processedTypes;
            SimpleProtoPacket     *packet;
            char                  *buffer;
    };

    #pragma pack()

    const size_t getDataSize (const DataType);
    const char *getDataTypeName (const Data::DataType);
    const char *getDataQualityName (const Data::Quality);

    const char *getGPSQualityName (const GPSQuality quality);
    const char *getPosSysModeName (const PosSystemMode mode);

    const bool alwaysSelected (const DataType);

    char *formatDataValueShort (const Data::Parameter& param, char *buffer, const size_t size);
}