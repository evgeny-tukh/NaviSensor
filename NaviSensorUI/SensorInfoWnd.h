#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "ListCtrlWrapper.h"
#include "EditWrapper.h"
#include "ButtonWrapper.h"
#include "../NaviSensor/Sensor.h"
#include "../NaviSensor/Network.h"
#include "../NaviSensor/Socket.h"

class SensorInfoWnd : public CDialogWrapper
{
    public:
        SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg);
        virtual ~SensorInfoWnd ();

    protected:
        Sensors::SensorConfig *sensorCfg;
        CListCtrlWrapper       sentences;
        CEditWrapper           terminal;
        CButtonWrapper         pause;
        Comm::DataNode         dataNode;
        bool                   threadEnabled, listeningEnabled;
        std::thread           *rawDataReceiver;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);

        //void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb);
        //static void logicProcInternal (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *self);
        void onMessage (Comm::MsgType msgType, const char *data, const int size);
        static void onMessageInternal (Comm::MsgType msgType, const char *data, const int size, void *self);

        void requestRawData (const bool send = true);

        void receiverProc ();
        static void receiverProcInternal (SensorInfoWnd *);
};
