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
}