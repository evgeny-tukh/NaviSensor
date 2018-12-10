#pragma once

#include "DBc.h"

namespace Parsers
{
    class DBS : public DBc
    {
        public:
            DBS () : DBc ("DBS") {}
    };
}