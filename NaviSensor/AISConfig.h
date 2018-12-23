#pragma once

#include <stdlib.h>
#include "../NaviSensorUI/SensorConfig.h"

namespace AIS
{
    class Filtering : public Sensors::Config
    {
        public:
            bool limitAmount, limitRange;
            int  maxAmount, maxRange;

            Filtering ()
            {
                limitAmount = limitRange = false;
                maxAmount   = maxRange   = 0;
            }

            Filtering& operator = (Filtering& source)
            {
                assign (source); return *this;
            }

            virtual void assign (Filtering& source);

            virtual void save ();
            virtual void load ();

            Filtering *get (const char *cfgFileName = 0)
            {
                if (!cfgFileName)
                {
                    /*if (!*defCfgPath)
                    {
                        std::string path = Tools::getAppDataFolder("CAIM", "NaviSensor") + "\\settings.cfg";

                        strncpy (defCfgPath, path.c_str (), sizeof (defCfgPath));
                    }*/

                    cfgFileName = "settings.cfg";//defCfgPath;
                }

                setCfgFileName (cfgFileName);
                load ();

                return this;
            }

        protected:
            static char defCfgPath [1000];
    };


}