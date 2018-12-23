#include <math.h>
#include "Formatting.h"

namespace Formatting
{
    char *formatCoordinate (const double coordinate, const int degPosNum, const char *worldSides, char *buffer, const size_t size)
    {
        const char *format   = degPosNum == 2 ? "%02d %6.3f%c" : "%03d %6.3f%c";
        double      absValue = fabs (coordinate);
        int         deg      = (int) floor (absValue);
        double      min      = (absValue - (double) deg) * 60.0;
        int         index    = coordinate >= 0 ? 0 : 1;

        snprintf (buffer, size, format, deg, min, worldSides [index]);

        return buffer;
    }

    char *formatLat (const double lat, char *buffer, const size_t size)
    {
        return formatCoordinate (lat, 2, "NS", buffer, size);
    }

    char *formatLon (const double lon, char *buffer, const size_t size)
    {
        return formatCoordinate (lon, 3, "EW", buffer, size);
    }

    char *formatPosition (Data::Pos *position, char *buffer, const size_t size)
    {
        char lat [100], lon [100];

        snprintf (buffer, size, "%s %s", formatLat (position->lat, lat, size), formatLon (position->lon, lon, size));

        return buffer;
    }

    char *formatUTC (Data::Time *utc, char *buffer, const size_t size)
    {
        snprintf (buffer, size, "%02d:%02d:%02d", utc->hour, utc->min, utc->sec);

        return buffer;
    }

    char *formatUTC (const time_t timestamp, char *buffer, const size_t size)
    {
        Data::Time utc;
        tm        *dateTime = gmtime (& timestamp);

        utc.hour = dateTime->tm_hour;
        utc.min  = dateTime->tm_min;
        utc.sec  = dateTime->tm_sec;

        return formatUTC (& utc, buffer, size);
    }

    char *getStringFormatValue (Data::DataType type, void *data, char *buffer, const size_t size)
    {
        switch (type)
        {
            case Data::DataType::Course:
            case Data::DataType::TrueHeading:
                snprintf (buffer, size, "%05.1f°", *((float *) data)); break;

            case Data::DataType::SpeedOG:
            case Data::DataType::SpeedTW:
                snprintf (buffer, size, "%.1fkn", *((float *) data)); break;

            case Data::DataType::HDOP:
                snprintf (buffer, size, "%.1f", *((float *) data)); break;

            case Data::DataType::Position:
                formatPosition ((Data::Pos *) data, buffer, size); break;

            case Data::DataType::PosSysMode:
                strncpy (buffer, Data::getPosSysModeName (*((Data::PosSystemMode *) data)), size);  break;

            case Data::DataType::RateOfTurn:
                snprintf (buffer, size, "%.1f°/sec", *((float *) data)); break;

            case Data::DataType::UTC:
                formatUTC ((Data::Time *) data, buffer, size); break;

            case Data::DataType::DepthBK:
            case Data::DataType::DepthBS:
            case Data::DataType::DepthBT:
                snprintf (buffer, size, "%.1fm", *((float *) data)); break;

            default:
                memset (buffer, 0, size);
        }

        return buffer;
    }
}