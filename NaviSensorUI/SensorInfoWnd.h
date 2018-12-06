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

struct DisplParam : Data::Parameter
{
    DisplParam (Data::Parameter& param)
    {
        assign (param);

        item = -1;
    }

    int item;
};

class DisplayedParams : public std::map <Data::DataType, DisplParam>
{
    public:
        void checkAdd (Data::Parameter&);

        inline void lock () { locker.lock (); }
        inline void unlock() { locker.unlock (); }

    protected:
        std::mutex locker;
};

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
        CListCtrlWrapper          sentences, parameters;
        DisplayedParams           params;
        CEditWrapper              terminal;
        CButtonWrapper            pause;
        Comm::DataNode            dataNode;
        bool                      threadEnabled, listeningEnabled, active;
        std::thread              *rawDataReceiver, *processedDataReceiver, *sentenceStateReceiver;
        std::mutex                statusLock;
        unsigned int              updateTimer;
        NMEA::SentenceStatusArray prevStatuses,
                                  curStatuses;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnTimer (UINT uiTimerID);
        virtual LRESULT OnDestroy ();

        //void logicProc (Comm::CommCallback sendCb, Comm::CommCallback rcvCb);
        //static void logicProcInternal (Comm::CommCallback sendCb, Comm::CommCallback rcvCb, void *self);
        void onMessage (Comm::MsgType msgType, const char *data, const int size);
        static void onMessageInternal (Comm::MsgType msgType, const char *data, const int size, void *self);

        void requestRawData (const bool send = true);
        void requestProcessedData (const bool send = true);
        void requestSentenceState (const bool send = true);

        void rawDataReceiverProc ();
        static void rawDataReceiverProcInternal (SensorInfoWnd *);

        void processedDataReceiverProc ();
        static void processedDataReceiverProcInternal (SensorInfoWnd *);

        void sentenceStateReceiverProc();
        static void sentenceStateReceiverProcInternal (SensorInfoWnd *);

        void stopThreads ();

        void redectectData ();

        void updateSentences ();
        void updateParameters ();
};
