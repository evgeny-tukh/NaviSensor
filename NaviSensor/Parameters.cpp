#include <time.h>
#include "Parameters.h"
#include "Formatting.h"

const char *Data::getDataTypeName (const Data::DataType type)
{
    const char *result;

    switch (type)
    {
        case DataType::Position:
            result = "Pos"; break;

        case DataType::Lat:
            result = "Lat"; break;

        case DataType::Lon:
            result = "Lon"; break;

        case DataType::UTC:
            result = "UTC"; break;

        case DataType::HDOP:
            result = "HDOP"; break;

        case DataType::TrueHeading:
            result = "HDG"; break;

        case DataType::Course:
            result = "COG"; break;

        case DataType::SpeedTW:
            result = "STW"; break;

        case DataType::SpeedOG:
            result = "SOG"; break;

        case DataType::RateOfTurn:
            result = "ROT"; break;

        case DataType::GPSQual:
            result = "Quality"; break;

        case DataType::PosSysMode:
            result = "ModeInd"; break;

        case DataType::DepthBK:
            result = "Depth BK"; break;

        case DataType::DepthBS:
            result = "Depth BS"; break;

        case DataType::DepthBT:
            result = "Depth BT"; break;

        default:
            result = "";
    }

    return result;
}

const char *Data::getDataQualityName (const Data::Quality quality)
{
    const char *result;

    switch (quality)
    {
        case Quality::Good:
            result = "Good"; break;

        case Quality::Poor:
            result = "Poor"; break;

        case Quality::Suspicious:
            result = "Susp"; break;

        default:
            result = "";
    }

    return result;
}

const bool Data::alwaysSelected (const Data::DataType type)
{
    bool result;

    switch (type)
    {
        case Data::DataType::GPSQual:
        case Data::DataType::HDOP:
        case Data::DataType::Position:
        case Data::DataType::PosSysMode:
            result = true; break;

        default:
            result = false;
    }

    return result;
}

const size_t Data::getDataSize (const Data::DataType type)
{
    size_t size;

    switch (type)
    {
        case DataType::Position:
            size = sizeof (Data::Pos); break;

        case DataType::UTC:
            size = sizeof (Data::Time); break;

        case DataType::HDOP:
        case DataType::TrueHeading:
        case DataType::Course:
        case DataType::SpeedTW:
        case DataType::SpeedOG:
        case DataType::RateOfTurn:
        case DataType::DepthBK:
        case DataType::DepthBS:
        case DataType::DepthBT:
            size = sizeof (float); break;

        case DataType::GPSQual:
        case DataType::PosSysMode:
            size = sizeof (byte); break;

        default:
            size = 0;
    }

    return size;
}

const char *Data::getGPSQualityName (const GPSQuality quality)
{
    const char *result;

    switch (quality)
    {
        case GPSQuality::InvalidFix:
            result = "Fix Invalid"; break;

        case GPSQuality::SPS:
            result = "GPS SPS mode"; break;

        case GPSQuality::Differential:
            result = "Differential"; break;

        case GPSQuality::PPS:
            result = "GPS PPS mode"; break;

        case GPSQuality::RTK:
            result = "RTK"; break;

        case GPSQuality::FloatRTK:
            result = "Float RTK"; break;

        case GPSQuality::Estimated:
            result = "Estimated"; break;

        case GPSQuality::Manual:
            result = "Manual"; break;

        case GPSQuality::Simulator:
            result = "Simulator"; break;

        default:
            result = "";
    }

    return result;
}

const char *Data::getPosSysModeName (const PosSystemMode mode)
{
    const char *result;

    switch (mode)
    {
        case PosSystemMode::Autonomous:
            result = "Autonomous"; break;

        case PosSystemMode::Diff:
            result = "Differential"; break;

        case PosSystemMode::Estim:
            result = "Estimated"; break;

        case PosSystemMode::Invalid:
            result = "Invalid"; break;

        case PosSystemMode::Man:
            result = "Manual"; break;

        case PosSystemMode::Simul:
            result = "Simulator"; break;

        default:
            result = "";
    }

    return result;
}

char *Data::formatDataValueShort (const Data::Parameter& param, char *buffer, const size_t size)
{
    switch (param.type)
    {
        case DataType::Position:
            Formatting::formatPosition ((Data::Pos *) param.data, buffer, size); break;

        case DataType::UTC:
            Formatting::formatUTC ((Data::Time *) param.data, buffer, size); break;

        case DataType::HDOP:
        case DataType::SpeedTW:
        case DataType::SpeedOG:
        case DataType::RateOfTurn:
            snprintf (buffer, size, "%.1f", *((float *) param.data)); break;

        case DataType::TrueHeading:
        case DataType::Course:
            snprintf (buffer, size, "%05.1f", *((float *) param.data)); break;

        case DataType::GPSQual:
            strncpy (buffer, getGPSQualityName ((Data::GPSQuality) ((byte *) param.data) [0]), size); break;

        case DataType::PosSysMode:
            strncpy (buffer, getPosSysModeName ((Data::PosSystemMode) ((char *) param.data) [0]), size); break;

        default:
            memset (buffer, 0, size);
    }

    return buffer;
}

Data::Parameter::Parameter ()
{
    size       = 0;
    type       = Data::DataType::Unknown;
    quality    = Quality::Poor;
    updateTime = 0;
    data       = 0;
}

Data::Parameter::Parameter (DataType paramType)
{
    size       = Data::getDataSize (paramType);
    type       = paramType;
    quality    = Quality::Poor;
    updateTime = 0;
    data       = (GenericData *) malloc (size);
}

Data::Parameter::Parameter (Data::ParamHeader& source)
{
    *((ParamHeader *) this) = source;

    updateTime = 0;
    data       = (GenericData *) malloc (size);
}

Data::Parameter::~Parameter ()
{
    if (data)
        free (data);
}

void Data::Parameter::update (GenericData *source, Data::Quality sourceQuality)
{
    if (source && data)
    {
        updateTime = time (0);
        quality    = sourceQuality;

        memcpy (data, source, size);
    }
}

void Data::Parameter::assign (Parameter& source)
{
    if (size != source.size)
    {
        size = source.size;

        if (data)
            data = (GenericData *) realloc (data, size);
        else
            data = (GenericData *) malloc (size);
    }

    type       = source.type;
    quality    = source.quality;
    updateTime = source.updateTime;

    memcpy (data, source.data, size);
}

void Data::Parameter::update (GenericData& source, Data::Quality sourceQuality)
{
    updateTime = time(0);
    quality    = sourceQuality;

    memcpy (data, & source, size);
}

void Data::GlobalParameter::init (const int sensorID, const bool master)
{
    this->sensorID = sensorID;
    this->master   = master;
}

Data::GlobalParameter::GlobalParameter () : Data::Parameter ()
{
    init ();
}

Data::GlobalParameter::GlobalParameter (const int sensorID, const bool master) : Data::Parameter ()
{
    init (sensorID, master);
}

Data::GlobalParameter::GlobalParameter (const int sensorID, const bool master, DataType type) : Data::Parameter (type)
{
    init (sensorID, master);
}

Data::GlobalParameter::GlobalParameter (const int sensorID, const bool master, Parameter& source) : Data::Parameter (source)
{
    init (sensorID, master);
}

Data::DataStorage::DataStorage (const time_t timeout)
{
    this->timeout = timeout;
}

Data::DataStorage::~DataStorage ()
{
    for (iterator iter = begin (); iter != end (); ++ iter)
    {
        if (iter->second)
            delete iter->second;
    }
}

void Data::DataStorage::update (const DataType dataType, GenericData *data, Data::Quality sourceQuality)
{
    Data::Parameter *param = find (dataType);

    if (!param)
        param = new Parameter (dataType);

    param->update (data, sourceQuality);
}

void Data::DataStorage::update (const DataType dataType, GenericData& data, Data::Quality sourceQuality)
{
    update (dataType, & data, sourceQuality);
}

Data::Parameter *Data::DataStorage::find (const DataType dataType)
{
    Data::Parameter *result = 0;

    for (iterator iter = begin(); iter != end(); ++iter)
    {
        if (iter->second && iter->second->type == dataType)
        {
            result = iter->second; break;
        }
    }

    return result;
}

void Data::DataStorage::checkExipired ()
{
    time_t now = time (0);

    for (iterator iter = begin (); iter != end (); ++iter)
    {
        if (iter->second && (now - iter->second->updateTime) > timeout)
            iter->second->quality = Data::Quality::Poor;
    }
}

void Data::DisplayedParams::checkAdd (Data::Parameter& param)
{
    iterator pos = find (param.type);

    if (pos == end())
        emplace (param.type, param);
    else
        pos->second.update (param.data);
}

Data::SimpleProtoPacket *Data::SimpleProtoPacket::init (void *buffer, const unsigned short numOfItems)
{
    SimpleProtoPacket *packet = (SimpleProtoPacket *)buffer;

    memcpy(packet->signature, NSSP_SIGNATURE, 4);

    packet->numOfItems    = numOfItems;
    packet->protoVerMajor = NSSP_MAJ_VER;
    packet->protoVerMinor = NSSP_MIN_VER;

    return packet;
}

void Data::SimpleProtoPacket::addData (const DataType dataType, const GenericData *data)
{
    switch (dataType)
    {
        case DataType::Position:
        {
            Data::Pos *position = (Data::Pos *) data;

            addData (DataType::Lat, position->lat);
            addData (DataType::Lon, position->lon);

            break;
        }

        case DataType::Course:
        case DataType::DepthBK:
        case DataType::DepthBS:
        case DataType::DepthBT:
        case DataType::RateOfTurn:
        case DataType::SpeedOG:
        case DataType::SpeedTW:
        case DataType::TrueHeading:
        case DataType::HDOP:
        {
            addData (dataType, *((float *) data)); break;
        }

        case DataType::GPSQual:
        case DataType::PosSysMode:
        {
            addData (dataType, *((unsigned char *) data)); break;
        }

        case Data::UTC:
        {
            Data::Time *utc      = (Data::Time *) data;
            time_t      now      = time (0);
            tm         *dateTime = gmtime (& now);

            addData (dataType, (double) mktime (dateTime)); break;
        }
    }
}

void Data::SimpleProtoPacket::addData (const DataType dataType, const double value)
{
    items [numOfItems].dataType = dataType;
    items [numOfItems].value    = value;

    ++ numOfItems;
}

Data::SimpleProtoBuffer::SimpleProtoBuffer ()
{
    buffer = (char *) malloc (1400);
    packet = SimpleProtoPacket::init (buffer);
}

Data::SimpleProtoBuffer::~SimpleProtoBuffer()
{
    if (buffer)
        free (buffer);
}

const size_t Data::SimpleProtoBuffer::getBufferSize()
{
    return packet ? sizeof (*packet) + (packet->numOfItems - 1) * sizeof (packet->items [0]) : 0;
}

const size_t Data::SimpleProtoBuffer::getNumOfItems()
{
    return packet ? (const size_t)packet->numOfItems : 0;
}

std::string Data::SimpleProtoPacket::formatData (const size_t index)
{
    char buffer [50];

    memset (buffer, 0, sizeof (buffer));

    if (index < numOfItems)
    {
        Data::SimpleProtoItem& data = items [index];

        switch (data.dataType)
        {
            case DataType::Lat:
                Formatting::formatLat (data.value, buffer, sizeof (buffer)); break;

            case DataType::Lon:
                Formatting::formatLon (data.value, buffer, sizeof (buffer)); break;

            case DataType::Course:
            case DataType::TrueHeading:
                sprintf (buffer, "%05.1fdeg", data.value); break;

            case Data::SpeedOG:
            case Data::SpeedTW:
                sprintf (buffer, "%.1fkn", data.value); break;

            case DataType::DepthBK:
            case DataType::DepthBS:
            case DataType::DepthBT:
                sprintf (buffer, "%.1fm", data.value); break;

            case DataType::RateOfTurn:
                sprintf (buffer, "%.1fdeg/min", data.value); break;

            case DataType::HDOP:
                sprintf (buffer, "%.f1", data.value); break;

            case DataType::GPSQual:
                strncpy (buffer, getGPSQualityName ((const Data::GPSQuality) (int) (data.value + 0.1)), sizeof (buffer)); break;

            case DataType::PosSysMode:
                strncpy (buffer, getPosSysModeName ((const Data::PosSystemMode) (int) (data.value + 0.1)), sizeof (buffer)); break;

            case DataType::UTC:
                Formatting::formatUTC ((time_t) (int) (data.value + 0.1), buffer, sizeof (buffer)); break;
        }
    }

    return std::string (buffer);
}

void Data::SimpleProtoBuffer::fromBuffer (const char *data, const size_t size)
{
    size_t actualSize = size < 1400 ? size : 1400;

    memcpy (packet, data, actualSize);

    packet->numOfItems = (unsigned short) (size - (sizeof (packet) - sizeof (packet->items [0]))) / sizeof (packet->items [0]);
}

Data::SimpleProtoItem *Data::SimpleProtoBuffer::getData (const size_t index)
{
    return packet && packet->numOfItems > index && index >= 0 ? packet->items + index : 0;
}

void Data::SimpleProtoBuffer::addData (const DataType dataType, const GenericData *data)
{
    if (packet)
        packet->addData (dataType, data);
}

void Data::SimpleProtoBuffer::addData (const DataType dataType, const double value)
{
    if (packet)
        packet->addData (dataType, value);
}

void Data::SimpleProtoBuffer::addData (const Data::GlobalParameter *param)
{
    if (packet)
    {
        const Data::ParamHeader& header = ((GlobalParameter *) param)->getHeader ();

        if (std::find (processedTypes.begin(), processedTypes.end (), header.type) == processedTypes.end ())
        {
            packet->addData (header.type, param->data);

            processedTypes.push_back (header.type);
        }
    }
}