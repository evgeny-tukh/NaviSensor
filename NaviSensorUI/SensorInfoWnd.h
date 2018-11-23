#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "ListCtrlWrapper.h"
#include "EditWrapper.h"
#include "ButtonWrapper.h"
#include "../NaviSensor/Sensor.h"
#include "../NaviSensor/DataServer.h"

class SensorInfoWnd : public CDialogWrapper
{
    public:
        SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg);

    protected:
        Sensors::SensorConfig *sensorCfg;
        CListCtrlWrapper       sentences;
        CEditWrapper           terminal;
        CButtonWrapper         pause;
        Comm::DataClient       dataClient;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);

        void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb);
        static void logicProcInternal (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *self);
};
