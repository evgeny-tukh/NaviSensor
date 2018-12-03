#pragma once

#include <mutex>
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
        typedef std::function <void()> Callback;

        SensorInfoWnd (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg, Callback onDestroy);
        virtual ~SensorInfoWnd ();

        inline bool isActive () { return active; }

    protected:
        Callback                  onDestroy;
        Sensors::SensorConfig    *sensorCfg;
        CListCtrlWrapper          sentences;
        CEditWrapper              terminal;
        CButtonWrapper            pause;
        Comm::DataNode            dataNode;
        bool                      threadEnabled, listeningEnabled, active;
        std::thread              *rawDataReceiver, *sentenceStateReceiver;
        std::mutex                statusLock;
        unsigned int              sentenceUpdateTimer;
        NMEA::SentenceStatusArray prevStatuses,
                                  curStatuses;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnTimer (UINT uiTimerID);
        virtual LRESULT OnDestroy ();

        //void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb);
        //static void logicProcInternal (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *self);
        void onMessage (Comm::MsgType msgType, const char *data, const int size);
        static void onMessageInternal (Comm::MsgType msgType, const char *data, const int size, void *self);

        void requestRawData (const bool send = true);
        void requestSentenceState (const bool send = true);

        void rawDataReceiverProc ();
        static void rawDataReceiverProcInternal (SensorInfoWnd *);

        void sentenceStateReceiverProc();
        static void sentenceStateReceiverProcInternal (SensorInfoWnd *);

        void stopThreads ();

        void redectectData ();
};
