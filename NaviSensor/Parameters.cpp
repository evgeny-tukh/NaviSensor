#include <time.h>
#include "Parameters.h"

const size_t Data::getDataSize (const Data::DataType type)
{
    size_t size;

    switch (type)
    {
        case Data::Position:
            size = sizeof (Data::Position); break;

        case DataType::UTC:
            size = sizeof (Data::Time); break;

        case DataType::HDOP:
            size = sizeof (float); break;

        case DataType::GPSQual:
            size = sizeof (byte); break;

        default:
            size = 0;
    }

    return size;
}

Data::Parameter::Parameter ()
{
    size       = 0;
    type       = Data::DataType::Unknown;
    quality    = Quality::Poor;
    updateTime = 0;
    data       = 0;
}

Data::Parameter::Parameter (DataType type)
{
    size       = Data::getDataSize (type);
    type       = type;
    quality    = Quality::Poor;
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
    size       = source.size;
    type       = source.type;
    quality    = source.quality;
    updateTime = source.updateTime;
    data       = (GenericData *) malloc (size);

    memcpy (data, source.data, size);
}

void Data::Parameter::update (GenericData& source, Data::Quality sourceQuality)
{
    updateTime = time(0);
    quality    = sourceQuality;

    memcpy (data, & source, size);
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
