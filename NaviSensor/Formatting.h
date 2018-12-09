#pragma once

#include "Parameters.h"

namespace Formatting
{
    char *formatCoordinate(const double coordinate, const int degPosNum, const char *worldSides, char *buffer, const size_t size);
    char *formatLat (const double lat, char *buffer, const size_t size);
    char *formatLon (const double lon, char *buffer, const size_t size);
    char *formatPosition(Data::Pos *position, char *buffer, const size_t size);
    char *formatUTC(Data::Time *utc, char *buffer, const size_t size);

    char *getStringFormatValue (Data::DataType type, void *data, char *buffer, const size_t size);
}
