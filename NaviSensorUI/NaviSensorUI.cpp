#include "resource.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <commctrl.h>
#include "WindowWrapper.h"
#include "ListCtrlWrapper.h"
#include "TreeCtrlWrapper.h"
#include "ListBoxWrapper.h"
#include "StaticWrapper.h"
#include "InputBox.h"
#include "SensorPropsDlg.h"
#include "SensorInfoWnd.h"
#include "SensorConfig.h"
#include <thread>
#include "tools.h"
#include "../NaviSensor/DataDef.h"
#include "../NaviSensor/Sensor.h"
#include "../NaviSensor/Network.h"
#include "../NaviSensor/Parameters.h"
#include "../NaviSensor/Formatting.h"
#include "../NaviSensor/AISTargetTable.h"
#include "../NaviSensor/Service.h"
#include "LocalListener.h"
#include "AISFilteringDlg.h"

#define IDC_SENSORS             100
#define IDC_AIS_TARGETS         101
#define IDC_PARAM_VALUE         102
#define IDC_AIS_NUM_OF_TARGETS  103
#define IDC_SIMPLE_PROTO_LIST   104

#define TIMER_STATE_UPDATE      101
#define TIMER_WATCHDOG          102

#define TITLE_ERROR             "Error"
#define TITLE_INFO              "Information"
#define SVC_NOT_INSTALLED       "Service is not installed"
#define SVC_INSTALL_REQUEST     "Service is not installed. Do you want to install it now?"
#define SVC_NAME                "NaviSensor"

#define PARAM_TIMEOUT           15
#define DEAD_ELIM_PERIOD        5

Sensors::SensorConfigArray sensors;

class Lockable
{
    public:
        inline void lock () { locker.lock (); }
        inline void unlock () { locker.unlock (); }

    protected:
        std::mutex locker;
};

class CSensorListCtrl : public CListCtrlWrapper
{
    public:
        CSensorListCtrl (HWND parent, UINT controlID) : CListCtrlWrapper (parent, controlID) {}
};

class CAISTargetListCtrl : public CListCtrlWrapper, public Lockable
{
    public:
        CAISTargetListCtrl (HWND parent, UINT controlID) : CListCtrlWrapper (parent, controlID) {}
        virtual ~CAISTargetListCtrl ();

        void updateTarget (AIS::AISDynamicData *);
        void updateTarget (AIS::AISStaticData *);
        void updateList ();

        void removeDeads ();

    protected:
        typedef std::map <unsigned int, AIS::AISTargetRec> Targets;

        Targets targets;
};

class CSimpleProtoListCtrl : public CListCtrlWrapper, public Lockable
{
    public:
        CSimpleProtoListCtrl(HWND parent, UINT controlID) : CListCtrlWrapper(parent, controlID), lastUpdate (0) {}
        virtual ~CSimpleProtoListCtrl () {}

        void updateList ();
        void updateData (const char *data, const int size);

    protected:
        time_t                  lastUpdate;
        Data::SimpleProtoBuffer buffer;
};

class CParamTreeCtrl : public CTreeCtrlWrapper, public Lockable
{
    public:
        CParamTreeCtrl (HWND parent, UINT controlID) : CTreeCtrlWrapper (parent, controlID), lastDeadsCheck (0) {}

        void updateData (Data::LANParamHeader *header, Data::GenericData *data);
        void updateTree ();
        void eliminateDeads (const bool lockData = false);

        enum Image
        {
            AlwaysSelected = 1,
            Selected       = 2,
            Unselected     = 3,
            Parameter      = 4
        };

        struct LANDisplayParamHeader : Data::LANParamHeader
        {
            Data::GenericData *data;
            HTREEITEM          item;
            time_t             updateTime;

            LANDisplayParamHeader () : Data::LANParamHeader ()
            {
                init ();
            }

            LANDisplayParamHeader (Data::LANParamHeader& source) : Data::LANParamHeader (source)
            {
                init ();
            }

            LANDisplayParamHeader (Data::LANParamHeader *source) : Data::LANParamHeader (*source)
            {
                init ();
            }

            void init ()
            {
                data       = (Data::GenericData *) malloc(size);
                item       = 0;
                updateTime = 0;
            }

            virtual ~LANDisplayParamHeader()
            {
                if (data)
                    free (data);
            }

            void update (Data::GenericData *sourceData, const bool masterState, Data::Quality sourceQuality = Data::Quality::Good)
            {
                if (sourceData && data)
                {
                    updateTime = time (0);
                    quality    = sourceQuality;
                    master     = masterState;

                    memcpy (data, sourceData, size);
                }
            }

            void update (Data::GenericData& sourceData, const bool masterState, Data::Quality sourceQuality = Data::Quality::Good)
            {
                update (& sourceData, masterState, sourceQuality);
            }
        };

    protected:
        class TypedParams //: public std::map <unsigned char, LANDisplayParamHeader *>
        {
            public:
                TypedParams (Data::DataType type)
                {
                    this->type = type;
                    this->item = 0;
                }

                virtual ~TypedParams ()
                {
                    for (auto & pos : map)
                    {
                        if (pos.second)
                            delete pos.second;
                    }
                }

                typedef std::map <unsigned char, LANDisplayParamHeader *> Map;

                Map            map;
                HTREEITEM      item;
                Data::DataType type;
        };

        class AllParams //: public std::map <Data::DataType, TypedParams *>
        {
            public:
                virtual ~AllParams ()
                {
                    for (auto & pos : map)
                    {
                        if (pos.second)
                            delete pos.second;
                    }
                }

                typedef std::map <Data::DataType, TypedParams *> Map;

                Map map;
        };

        AllParams  params;
        time_t     lastDeadsCheck;
};

class CMainWnd : public CWindowWrapper
{
    public:
        CMainWnd (HINSTANCE instance);
        virtual ~CMainWnd ();

    private:
        typedef std::map <unsigned int, SensorInfoWnd *> SensorWatchWndArray;

        Data::DisplayedParams     params;
        Data::DaemonState         daemonState, lastState;
        time_t                    daemonStateUpdate;
        CSensorListCtrl          *sensorListCtl;
        CParamTreeCtrl           *paramTreeCtrl;
        CSimpleProtoListCtrl     *simpleProtoList;
        CAISTargetListCtrl       *aisTargetListCtrl;
        CStaticWrapper           *paramValueCtrl, *aisTargetsNumCtrl, *simpleProtoLabel;
        Comm::Socket              transmitter;
        HMENU                     menu;
        bool                      active;
        unsigned int              cmdSeqNumber, stateUpdateTimer, watchdogTimer;
        HIMAGELIST                sensorImages, paramImages;
        Sensors::SensorStateArray sensorStates, lastSensorStates;
        SensorWatchWndArray       sensorWatchWindows;
        Comm::LocalListener      *sensorStateListener, *processedDataListener, *aisTargetListener, *simpleProtoListener;

        enum SensorImage
        {
            alive    = 0,
            dead     = 1,
            inactive = 2
        };

        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnTimer (unsigned int timerID);
        virtual LRESULT OnNotify (NMHDR *pHeader);

        void RequestAppQuit ();

        void Initialize ();
        void AddSensor ();
        void EditSensor ();
        void DeleteSensor ();
        void WatchSensor ();
        void StartStop ();
        void EditAISFiltering ();
        void startService ();
        void stopService ();

        void DisplaySensorConfig (Sensors::SensorConfig *config, int item = -1);

        void changeState (const Data::DaemonState newState);

        void waitForListener (Comm::LocalListener *);

        void displayParamValue (HTREEITEM item = 0);
};

void CParamTreeCtrl::eliminateDeads (const bool lockData)
{
    time_t now = time (0);

    if (lockData)
        lock ();

    for (auto & paramType : params.map)
    {
        auto & map           = paramType.second->map;
        bool   masterRemoved = false;

        for (auto pos = map.begin (); pos != map.end (); )
        {
            auto & header = pos->second;

            if ((now - header->updateTime) > PARAM_TIMEOUT)
            {
                if (header->master)
                    masterRemoved = true;

                if (header->item)
                    DeleteItem (header->item);

                pos = map.erase (pos);
            }
            else
            {
                ++ pos;
            }
        }

        // Assign new master if no one exists
        if (masterRemoved)
        {
            bool masterFound = false;

            for (auto & item : map)
            {
                if (item.second->master)
                {
                    masterFound = true; break;
                }
            }

            if (!masterFound)
            {
                for (auto & item : map)
                {
                    if (item.second->quality == Data::Quality::Good)
                    {
                        int image;

                        item.second->master = true;
                        
                        if (Data::alwaysSelected (paramType.first))
                            image = Image::AlwaysSelected;
                        else
                            image = Image::Selected;

                        SetItemImage (item.second->item, image);

                        break;
                    }
                }
            }
        }
    }

    if (lockData)
        unlock ();
}

void CParamTreeCtrl::updateData (Data::LANParamHeader *dataHeader, Data::GenericData *data)
{
    AllParams::Map::iterator elem = params.map.find (dataHeader->type);

    if (elem == params.map.end ())
        elem = params.map.insert (params.map.end (), std::pair <Data::DataType, TypedParams *> (dataHeader->type, new TypedParams (dataHeader->type)));

    auto                       map    = elem->second;
    TypedParams::Map::iterator mapPos = map->map.find (dataHeader->sensorID);

    if (mapPos == map->map.end ())
    {
        LANDisplayParamHeader dispParam (dataHeader);
        TypedParams          *elemMap = elem->second;

        auto pair = std::pair <unsigned char, LANDisplayParamHeader *> (dataHeader->sensorID, new LANDisplayParamHeader (dataHeader));

        mapPos = elemMap->map.insert (elemMap->map.end (), pair);
    }

    mapPos->second->update (data, dataHeader->master, dataHeader->quality);
}

void CParamTreeCtrl::updateTree ()
{
    time_t now = time (0);

    lock ();

    if ((now - lastDeadsCheck) > DEAD_ELIM_PERIOD)
    {
        lastDeadsCheck = now;

        eliminateDeads ();
    }

    for (auto & paramType : params.map)
    {
        if (paramType.second->item == 0)
            paramType.second->item = InsertItem (Data::getDataTypeName (paramType.second->type), Image::Parameter, (LPARAM) paramType.second);

        for (auto & param : paramType.second->map)
        {
            int image;

            if (Data::alwaysSelected (paramType.first))
                image = Image::AlwaysSelected;
            else if (param.second->master)
                image = Image::Selected;
            else
                image = Image::Unselected;

            if (param.second->item == 0)
                param.second->item = InsertItem (param.second->sensorName, image, (LPARAM) param.second, paramType.second->item);
            else
                SetItemImage (param.second->item, image);
        }
    }

    unlock ();
}

void CSimpleProtoListCtrl::updateList ()
{
    lock ();

    if ((time(0) - lastUpdate) > PARAM_TIMEOUT)
    {
        DeleteAllItems ();
    }
    else
    {
        for (size_t i = 0, count = GetItemCount (); i < buffer.getNumOfItems (); ++ i)
        {
            const Data::SimpleProtoItem *data         = buffer.getData (i);
            const char                  *dataTypeName = data ? Data::getDataTypeName ((Data::DataType) data->dataType) : "";

            if (i >= count)
            {
                AddItem (dataTypeName);

                ++ count;
            }
            else
            {
                SetItemText (i, 0, dataTypeName);
            }

            SetItemText (i, 1, buffer.formatData (i).c_str ());
        }

        for (size_t count = GetItemCount (); count > buffer.getNumOfItems (); DeleteItem (-- count));
    }

    unlock ();
}

void CSimpleProtoListCtrl::updateData (const char *data, const int size)
{
    lock ();

    buffer.fromBuffer (data, size);

    lastUpdate = time (0);

    unlock ();
}

CAISTargetListCtrl::~CAISTargetListCtrl ()
{
    for (auto & pos : targets)
    {
        if (pos.second.second)
            delete pos.second.second;
    }
}

void CAISTargetListCtrl::updateTarget (AIS::AISDynamicData *dynData)
{
    time_t now = time (0);
    auto   pos = targets.find (dynData->mmsi);

    if (pos == targets.end ())
    {
        auto insertion = targets.emplace (dynData->mmsi, std::pair <time_t, AIS::AISTarget *> (now, new AIS::AISTarget (dynData->mmsi)));

        insertion.first->second.second->flags       = dynData->flags;
        insertion.first->second.first               = now;
        insertion.first->second.second->dynamicData = dynData->data;
    }
    else
    {
        pos->second.second->flags       = dynData->flags;
        pos->second.second->mmsi        = dynData->mmsi;
        pos->second.first               = now;
        pos->second.second->dynamicData = dynData->data;
    }
}

void CAISTargetListCtrl::updateTarget (AIS::AISStaticData *staticData)
{
    time_t now = time (0);
    auto   pos = targets.find (staticData->mmsi);

    if (pos == targets.end ())
    {
        auto insertion = targets.emplace (staticData->mmsi, std::pair <time_t, AIS::AISTarget *> (now, new AIS::AISTarget (staticData->mmsi)));

        insertion.first->second.second->flags      = staticData->flags;
        insertion.first->second.second->staticData = staticData->data;
    }
    else
    {
        pos->second.first               = now;
        pos->second.second->flags       = staticData->flags;
        pos->second.second->mmsi        = staticData->mmsi;
        pos->second.second->staticData  = staticData->data;
    }
}

void CAISTargetListCtrl::removeDeads ()
{
    std::thread remover ([this] () -> void
                         {
                             time_t now = time (0);

                             lock ();

                             for (Targets::iterator pos = targets.begin (); pos != targets.end ();)
                             {
                                if ((now - pos->second.first) > 120)
                                    pos = targets.erase (pos);
                                else
                                    ++ pos;
                             }

                             unlock ();
                         });

    remover.detach ();
}

void CAISTargetListCtrl::updateList ()
{
    Data::Pos position;
    char      buffer [100];
    int       i     = 0,
              count = GetItemCount (),
              item;

    lock ();

    for (auto & pos : targets)
    {
        if (i >= count)
        {
            item = AddItem ("", (LPARAM) pos.second.second);

            ++ count;
        }
        else
        {
            item = i;
        }

        AIS::AISTarget  *target     = pos.second.second;
        AIS::AISDynamic& dynData    = target->dynamicData;
        AIS::AISStatic&  staticData = target->staticData;

        position.lat = dynData.lat;
        position.lon = dynData.lon;

        SetItemText (item, 0, _itoa (pos.first, buffer, 10));
        SetItemText (item, 3, Formatting::formatPosition (& position, buffer, sizeof (buffer)));

        if (pos.second.second->flags & AIS::TargetFlags::Name)
            SetItemText (item, 2, staticData.name);
        else
            SetItemText (item, 2, "");

        if (target->flags & AIS::TargetFlags::AtoN)
            SetItemText (item, 1, "AtoN");
        else if (target->flags & AIS::TargetFlags::BaseStation)
            SetItemText (item, 1, "BS");
        else if (target->flags & AIS::TargetFlags::ClassB)
            SetItemText (item, 1, "B");
        else
            SetItemText (item, 1, "A");

        ++ i;
    }

    for (int j = GetItemCount () - 1; j >= i; DeleteItem (j--));

    unlock ();
}

void CMainWnd::Initialize()
{
    RECT client;

    stateUpdateTimer = SetTimer (TIMER_STATE_UPDATE, 1000);
    watchdogTimer    = SetTimer (TIMER_WATCHDOG, 5000);

    GetClientRect (& client);

    sensorListCtl = new CSensorListCtrl (m_hwndHandle, IDC_SENSORS);

    sensorListCtl->CreateControl (0, 50, client.right, 100, LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL);
    sensorListCtl->SetWholeLineSelection ();
    sensorListCtl->AddColumn ("Name", 200);
    sensorListCtl->AddColumn ("Type", 80);
    sensorListCtl->AddColumn ("Conn type", 80);
    sensorListCtl->AddColumn ("Comm parameters", client.right - 370);

    sensorListCtl->SendMessage (LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) sensorImages);

    paramTreeCtrl = new CParamTreeCtrl (m_hwndHandle, IDC_PARAMETERS);

    paramTreeCtrl->CreateControl (0, 150, 150, client.bottom - 150, WS_BORDER | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT);
    paramTreeCtrl->SendMessage (TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM) paramImages);

    paramValueCtrl = new CStaticWrapper (m_hwndHandle, IDC_PARAM_VALUE);

    //paramValueCtrl->CreateControl (150, 150, client.right - 520, client.bottom - 150, WS_BORDER | SS_CENTER);
    paramValueCtrl->CreateControl (150, 150, client.right - 520, 200, WS_BORDER | SS_CENTER);

    simpleProtoLabel = new CStaticWrapper (m_hwndHandle, IDC_STATIC);

    simpleProtoLabel->CreateControl (150, 350, client.right - 520, 20, WS_BORDER | SS_CENTER);
    simpleProtoLabel->SetText ("Simple protocol");

    simpleProtoList = new CSimpleProtoListCtrl (m_hwndHandle, IDC_SIMPLE_PROTO_LIST);

    simpleProtoList->CreateControl (150, 370, client.right - 520, client.bottom - 370, LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL);
    simpleProtoList->SetWholeLineSelection();
    simpleProtoList->AddColumn ("Data type", 100);
    simpleProtoList->AddColumn ("Value", 140);

    aisTargetsNumCtrl = new CStaticWrapper (m_hwndHandle, IDC_AIS_NUM_OF_TARGETS);
    aisTargetListCtrl = new CAISTargetListCtrl (m_hwndHandle, IDC_AIS_TARGETS);

    aisTargetsNumCtrl->CreateControl (client.right - 370, 150, 370, 20, SS_CENTER | WS_BORDER);
    aisTargetListCtrl->CreateControl (client.right - 370, 180, 380, client.bottom - 180, LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_VSCROLL);
    aisTargetListCtrl->SetWholeLineSelection ();
    aisTargetListCtrl->AddColumn ("MMSI", 60);
    aisTargetListCtrl->AddColumn ("Type", 40);
    aisTargetListCtrl->AddColumn ("Name", 100);
    aisTargetListCtrl->AddColumn ("Position", 140);

    for (auto & sensorCfg : sensors)
        DisplaySensorConfig (sensorCfg);
}

void CMainWnd::RequestAppQuit ()
{
    if (MessageBox ("Do you want to exit the application?", "Confirmation needed", MB_YESNO | MB_ICONQUESTION) == IDYES)
        Destroy ();
}

void CMainWnd::DisplaySensorConfig (Sensors::SensorConfig *config, int item)
{
    if (item < 0)
    {
        item = sensorListCtl->AddItem (config->name.c_str (), (LPARAM) config, 2);
    }
    else
    {
        sensorListCtl->SetItemText (item, 0, config->name.c_str());
        sensorListCtl->SetItemData (item, (LPARAM) config);
    }

    sensorListCtl->SetItemText (item, 1, Sensors::getOptionName (Sensors::sensorTypeOptions, config->type));
    sensorListCtl->SetItemText (item, 2, Sensors::getOptionName (Sensors::connTypeOptions, config->connection));
    sensorListCtl->SetItemText (item, 3, config->getParameterString ().c_str ());
}

LRESULT CMainWnd::OnMessage (UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            Initialize (); break;

        case WM_DESTROY:
            DestroyMenu (menu);
            PostQuitMessage (0);

            break;
    }

    return CWindowWrapper::OnMessage (message, wParam, lParam);
}

LRESULT CMainWnd::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = FALSE;

    switch (LOWORD (wParam))
    {
        case ID_SERVICE_START:
            startService (); break;

        case ID_SERVICE_STOP:
            stopService (); break;

        case ID_START:
            StartStop (); break;

        case ID_WATCH_SENSOR:
            WatchSensor (); break;

        case ID_ADD_SENSOR:
            AddSensor (); break;

        case ID_EDIT_SENSOR:
            EditSensor (); break;

        case ID_REMOVE_SENSOR:
            DeleteSensor (); break;

        case ID_EXIT:
            RequestAppQuit (); break;

        case ID_AIS_FILTERING:
            EditAISFiltering (); break;

        default:
            result = TRUE;
    }

    return result;
}

LRESULT CMainWnd::OnSysCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    switch (wParam)
    {
        case SC_CLOSE:
            RequestAppQuit();

            result = FALSE; break;

        default:
            result = CWindowWrapper::OnSysCommand (wParam, lParam);
    }

    return result;
}

LRESULT CMainWnd::OnSize (const DWORD requestType, const WORD width, const WORD height)
{
    sensorListCtl->Move (0, 0, width, 150, TRUE);
    paramTreeCtrl->Move (0, 149, 150, height - 135, TRUE);
    //paramValueCtrl->Move (150, 150, width - 520, height - 150, true);
    paramValueCtrl->Move (150, 150, width - 520, 200, true);
    simpleProtoList->Move (150, 350, width - 520, height - 350, true);
    aisTargetsNumCtrl->Move (width - 370, 150, 370, 20, true);
    aisTargetListCtrl->Move (width - 370, 170, 370, height - 170, true);

    return FALSE;
}

LRESULT CMainWnd::OnNotify (NMHDR *header)
{
    if (header->idFrom == IDC_SENSORS)
    {
        if (header->code == NM_DBLCLK)
        {
            NMITEMACTIVATE *pItemActData = (NMITEMACTIVATE *) header;

            if (daemonState == Data::DaemonState::Running)
                WatchSensor ();
            else
                EditSensor ();
        }
    }
    else if  (header->idFrom == IDC_PARAMETERS)
    {
        if (header->code == TVN_SELCHANGED)
        {
            NMTREEVIEW *changeHeader = (NMTREEVIEW *) header;

            displayParamValue (changeHeader->itemNew.hItem);
        }
        else if (header->code == NM_CLICK)
        {
            POINT     point;
            HTREEITEM item;
            UINT      flags;

            GetCursorPos (& point);
            
            paramTreeCtrl->ScreenToClient(&point);

            item = paramTreeCtrl->HitTest (point, & flags);

            if (item && (flags & TVHT_ONITEMICON))
            {
                CParamTreeCtrl::LANDisplayParamHeader *header = (CParamTreeCtrl::LANDisplayParamHeader *) paramTreeCtrl->GetItemData (item);

                if (!Data::alwaysSelected (header->type))
                    Comm::DataNode::sendCommand (& transmitter, Comm::CmdType::SelectMasterParam, (unsigned short) header->sensorID,
                                                 (unsigned short) header->type, Comm::CmdPort, "127.0.0.1");
            }
        }
    }

    return FALSE;
}

void CMainWnd::displayParamValue (HTREEITEM item)
{
    if (!item)
        item = paramTreeCtrl->GetSelectedItem ();

    if (paramTreeCtrl->GetNextItem (item, TVGN_PARENT) == 0)
    {
        paramValueCtrl->SetText("");
    }
    else
    {
        char buffer[1024];

        CParamTreeCtrl::LANDisplayParamHeader *param = (CParamTreeCtrl::LANDisplayParamHeader *) paramTreeCtrl->GetItemData(item);

        paramValueCtrl->SetText (Formatting::getStringFormatValue (param->type, param->data, buffer, sizeof (buffer)));
    }
}

CMainWnd::CMainWnd (HINSTANCE instance) :
    active (true),
    lastState (Data::DaemonState::Inactive),
    cmdSeqNumber (0),
    CWindowWrapper (instance, HWND_DESKTOP, "NaviSensorUIWnd", menu = LoadMenu (instance, MAKEINTRESOURCE(IDR_MAINMENU))),
    daemonState (Data::Inactive),
    daemonStateUpdate (0),
    transmitter()/*,
    stateUpdater (stateUpdaterProcInternal, this)*/
{
    WSADATA              winsockData;
    Comm::Socket::ReadCb processedDataCb = [this](char *data, const int size) -> void
                                           {
                                              if (paramTreeCtrl)
                                              {
                                                  paramTreeCtrl->lock ();
                                              
                                                  for (int offset = 0; offset < size;)
                                                  {
                                                      Data::LANParamHeader *param = (Data::LANParamHeader *) (data + offset);

                                                      paramTreeCtrl->updateData (param, (Data::GenericData *) (param + 1));

                                                      offset += sizeof(Data::LANParamHeader) + param->size;
                                                  }

                                                  paramTreeCtrl->unlock ();
                                              }
                                           };
    Comm::Socket::ReadCb simpleProtoCb   = [this](char *data, const int size) -> void
                                           {
                                              if (simpleProtoList)
                                                  simpleProtoList->updateData (data, size);
                                           };
    Comm::Socket::ReadCb aisDataCb       = [this](char *data, const int size) -> void
                                           {
                                              Comm::GenericMsg *message = (Comm::GenericMsg *) data;

                                              if (aisTargetListCtrl)
                                              {
                                                  aisTargetListCtrl->lock ();
                                              
                                                  if (message->msgType == Comm::MsgType::AISDynamic)
                                                  {
                                                      Comm::AISDynData *aisData = (Comm::AISDynData *) data;

                                                      for (int i = 0, count = aisData->size / sizeof (AIS::AISDynamicData); i < count; ++ i)
                                                          aisTargetListCtrl->updateTarget (aisData->targets + i);
                                                  }
                                                  else if (message->msgType == Comm::MsgType::AISStatic)
                                                  {
                                                      Comm::AISStaticData *aisData = (Comm::AISStaticData *) data;

                                                      for (int i = 0, count = aisData->size / sizeof(AIS::AISStaticData); i < count; ++i)
                                                          aisTargetListCtrl->updateTarget (aisData->targets + i);
                                                  }

                                                  aisTargetListCtrl->unlock ();
                                              }
                                           };
    Comm::Socket::ReadCb sensorStateCb   = [this] (char *data, const int size) -> void
                                           {
                                              Comm::HeartbeatData *message = (Comm::HeartbeatData *) data;

                                              daemonStateUpdate = time (0);
                                              daemonState       = (Data::DaemonState) message->daemonState;

                                              sensorStates.lock ();
                                              sensorStates.clear ();

                                              for (size_t i = 0; i < message->numOfSensors; ++ i)
                                                  sensorStates.emplace_back (message->sensorState [i].id, message->sensorState[i].alive,
                                                                             message->sensorState [i].running);

                                              sensorStates.unlock ();
                                           };

    sensorImages = ImageList_LoadBitmap (instance, MAKEINTRESOURCE (IDB_SENSOR), 16, 16, CLR_NONE);
    paramImages  = ImageList_LoadBitmap (instance, MAKEINTRESOURCE (IDB_PARAMS), 16, 1, CLR_NONE);

    sensors.loadAll ();

    sensorListCtl = NULL;

    WSAStartup (MAKEWORD (1, 1), & winsockData);

    sensorStateListener   = new Comm::LocalListener (Comm::Ports::SensorPort, & active, sensorStateCb);
    processedDataListener = new Comm::LocalListener (Comm::Ports::ProcessedDataPort, & active, processedDataCb);
    aisTargetListener     = new Comm::LocalListener (Comm::Ports::AISTargetPort, & active, aisDataCb);
    simpleProtoListener   = new Comm::LocalListener (Comm::Ports::SimpleProtocolPort, & active, simpleProtoCb);

    transmitter.create ();
}

void CMainWnd::waitForListener (Comm::LocalListener *listener)
{
    if (listener)
    {
        if (listener->joinable())
            listener->join();

        delete listener;
    }
}

CMainWnd::~CMainWnd ()
{
    active = false;

    if (sensorListCtl)
        delete sensorListCtl;

    if (paramTreeCtrl)
        delete paramTreeCtrl;

    if (paramValueCtrl)
        delete paramValueCtrl;

    waitForListener (sensorStateListener);
    waitForListener (processedDataListener);
    waitForListener (aisTargetListener);

    transmitter.close ();

    ImageList_Destroy (sensorImages);
    ImageList_Destroy (paramImages);
}

void CMainWnd::AddSensor ()
{
    Sensors::SensorConfig sensorCfg;
    SensorPropsDlg       dialog (m_hInstance, m_hwndParent, & sensorCfg);

    if (dialog.Execute() == IDOK)
    {
        Sensors::SensorConfig *newConfig = sensors.addFrom (sensorCfg);

        newConfig->save ();

        DisplaySensorConfig (newConfig);
    }
}

void CMainWnd::changeState (const Data::DaemonState newState)
{
    if (daemonState != newState && (newState == Data::DaemonState::Running || newState == Data::DaemonState::Stopped))
    {
        Comm::Command cmd;

        cmd.size      = sizeof (cmd);
        cmd.msgType   = Comm::MsgType::Cmd;
        cmd.seqNumber = cmdSeqNumber ++;
        cmd.argument  = 0;
        cmd.command   = newState == Data::DaemonState::Running ? Comm::CmdType::Start : Comm::CmdType::Stop;

        transmitter.sendTo ((const char *) & cmd, sizeof (cmd), Comm::Ports::CmdPort, "127.0.0.1");
    }
}

void CMainWnd::EditAISFiltering()
{
    AIS::Filtering  filtering;
    AISFilteringDlg dialog (m_hInstance, m_hwndHandle, filtering.get ());

    if (dialog.Execute () == IDOK)
    {
        filtering.save ();

        Comm::DataNode::sendCommand (&transmitter, Comm::CmdType::ReapplyAISFilter);
    }
}

void CMainWnd::startService ()
{
    Service naviSensor (SVC_NAME);
    bool    running;

    if (naviSensor.installed (running))
    {
        if (running)
            MessageBox ("Service already started", TITLE_INFO, MB_ICONEXCLAMATION);
        else
            naviSensor.startStop (true);
    }
    else
    {
        if (MessageBox (SVC_INSTALL_REQUEST, ERROR, MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            char path [MAX_PATH];

            GetModuleFileName (0, path, sizeof (path));
            PathRemoveFileSpec (path);
            PathAppend (path, "NaviSensor.exe");

            if (naviSensor.install (false, path))
            {
                if (MessageBox ("NaviSensor service has been installed.\n\nDo you want to run it now?", TITLE_INFO, MB_YESNO | MB_ICONQUESTION) == IDYES)
                    naviSensor.startStop (true);
            }
        }
    }
}

void CMainWnd::stopService ()
{
    Service naviSensor (SVC_NAME);
    bool    running;

    if (naviSensor.installed (running))
    {
        if (running)
            naviSensor.startStop (false);
        else
            MessageBox ("Service is not started", TITLE_INFO, MB_ICONEXCLAMATION);
    }
    else
    {
        MessageBox (SVC_NOT_INSTALLED, TITLE_ERROR, MB_ICONSTOP);
    }
}

void CMainWnd::StartStop()
{
    switch (daemonState)
    {
        case Data::DaemonState::Running:
            changeState (Data::DaemonState::Stopped); break;

        case Data::DaemonState::Stopped:
            changeState (Data::DaemonState::Running); break;

        default:
            return;
    }
}

void CMainWnd::WatchSensor ()
{
    int selection = sensorListCtl->GetSelectedItem();

    if (selection >= 0)
    {
        Sensors::SensorConfig *sensorCfg = (Sensors::SensorConfig *) sensorListCtl->GetItemData (selection);

        if (sensorCfg)
        {
            SensorWatchWndArray::iterator pos = sensorWatchWindows.find (sensorCfg->id ());

            if (pos != sensorWatchWindows.end () && !IsWindow (pos->second->GetHandle ()))
            {
                sensorWatchWindows.erase (pos);

                pos = sensorWatchWindows.end ();
            }

            if (pos == sensorWatchWindows.end ())
            {
                SensorInfoWnd *window = new SensorInfoWnd (m_hInstance, GetHandle (), sensorCfg,
                                                           [this, sensorCfg] () -> void
                                                           {
                                                               SensorWatchWndArray::iterator pos    = sensorWatchWindows.find (sensorCfg->id ());
                                                               SensorInfoWnd                *window = NULL;

                                                               if (pos != sensorWatchWindows.end ())
                                                               {
                                                                   window = pos->second;

                                                                   sensorWatchWindows.erase (pos);
                                                               }

                                                               //if (window)
                                                               //    delete window;
                                                           });

                window->Show ();

                sensorWatchWindows.insert (std::pair <unsigned int, SensorInfoWnd *> (sensorCfg->id (), window));
            }

            #if 0
            std::thread watcher ([] (CMainWnd *self, Sensors::SensorConfig *sensorCfg) -> void
                                 {
                                    SensorInfoWnd window (NULL, self->GetHandle (), sensorCfg);
                                    //HWND          wndHandle;
                                    MSG           msgMessage;

                                    window.Show ();

                                    //wndHandle = window.GetHandle ();


                                    while (window.isActive () && GetMessage (& msgMessage, /*wndHandle*/NULL, NULL, NULL))
                                    {
                                        TranslateMessage (& msgMessage);
                                        DispatchMessage (& msgMessage);
                                    }

                                    Tools::sleepFor (100);
                                 },
                                 this, sensorCfg);

            watcher.detach ();
            #endif
        }
    }
}

void CMainWnd::EditSensor ()
{
    int selection = sensorListCtl->GetSelectedItem();

    if (selection >= 0)
    {
        Sensors::SensorConfig *sensorCfg = (Sensors::SensorConfig *) sensorListCtl->GetItemData (selection);

        if (sensorCfg)
        {
            SensorPropsDlg dialog (m_hInstance, m_hwndParent, sensorCfg);

            if (dialog.Execute() == IDOK)
            {
                sensorCfg->save ();

                DisplaySensorConfig (sensorCfg, selection);
            }
        }
    }
}

LRESULT CMainWnd::OnTimer (unsigned int timerID)
{
    if (timerID == TIMER_WATCHDOG)
    {
        aisTargetListCtrl->removeDeads ();
    }
    else if (timerID == TIMER_STATE_UPDATE)
    {
        MENUITEMINFO              menuItemInfo;
        char                      caption[256];

        if ((time (0) - daemonStateUpdate) > 2)
            daemonState = Data::DaemonState::Inactive;

        paramTreeCtrl->updateTree ();
        aisTargetListCtrl->updateList ();
        simpleProtoList->updateList ();
        
        sprintf (caption, "%d AIS targets", aisTargetListCtrl->GetItemCount ());

        aisTargetsNumCtrl->SetText (caption);

        displayParamValue ();

        //if (lastSensorStates != sensorStates)
        {
            for (int i = 0, count = sensorListCtl->GetItemCount (); i < count; ++i)
            {
                int imageIndex = SensorImage::inactive;

                if (daemonState != Data::DaemonState::Inactive)
                {
                    Sensors::SensorConfig *config = (Sensors::SensorConfig *) sensorListCtl->GetItemData (i);

                    sensorStates.lock ();

                    for (Sensors::SensorState& sensorState : sensorStates)
                    {
                        if (sensorState.id == config->id ())
                        {
                            if (sensorState.running)
                                imageIndex = sensorState.alive ? SensorImage::alive : SensorImage::dead;

                            break;
                        }
                    }

                    sensorStates.unlock ();
                }

                sensorListCtl->SetItemImage (i, imageIndex);
            }
        }

        if (lastState != daemonState && IsWindow(m_hwndHandle))
        {
            const char *state;

            switch (daemonState)
            {
                case Data::DaemonState::Inactive:
                    state = "Inactive"; break;

                case Data::DaemonState::Running:
                    state = "Running"; break;

                case Data::DaemonState::Stopped:
                    state = "Stopped"; break;

                default:
                    state = "Unknown";
            }

            sprintf (caption, "NaviSensor UI [%s]", state);

            SetText(caption);

            lastState = daemonState;

            memset (&menuItemInfo, 0, sizeof (menuItemInfo));

            menuItemInfo.cbSize     = sizeof (menuItemInfo);
            menuItemInfo.fMask      = MIIM_STRING | MIIM_STATE;
            menuItemInfo.fState     = daemonState == Data::DaemonState::Inactive ? MFS_GRAYED : 0;
            menuItemInfo.dwTypeData = (char *) (daemonState == Data::DaemonState::Running ? "Stop" : "Start");

            SetMenuItemInfo (GetSubMenu (m_hmnuMenu, 1), ID_START, FALSE, &menuItemInfo);
        }
    }

    return CWindowWrapper::OnTimer (timerID);
}

void CMainWnd::DeleteSensor ()
{
    int selection = sensorListCtl->GetSelectedItem ();

    if (selection >= 0)
    {
        Sensors::SensorConfig *sensorCfg = (Sensors::SensorConfig *) sensorListCtl->GetItemData (selection);

        if (sensorCfg)
        {
            char message [500];

            sprintf (message, "Do you want to remove sensor '%s'?", sensorCfg->name.c_str ());

            if (MessageBox (message, "Sensor delete", MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
                sensors.DeleteConfigByID (sensorCfg->sensorID, true);

                sensorListCtl->DeleteItem (selection);
            }
        }
    }
}

int CALLBACK WinMain (HINSTANCE instance, HINSTANCE prevInstance, char *cmdLine, int showCmd)
{
    CMainWnd mainWindow (instance);
    Service  naviSensor (SVC_NAME);
    bool     running,
             installed = naviSensor.installed (running);

    if (!installed)
    {
        if (MessageBox (HWND_DESKTOP, "NaviSensor service is not found in the system.\n\nDo you want to install it now?", TITLE_ERROR,
                        MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            char path [MAX_PATH];

            GetModuleFileName (0, path, sizeof (path));
            PathRemoveFileSpec (path);
            PathAppend (path, "NaviSensor.exe");

            if (naviSensor.install (false, path))
            {
                if (MessageBox (HWND_DESKTOP, "NaviSensor service has been installed.\n\nDo you want to run it now?", "Service installed",
                    MB_YESNO | MB_ICONQUESTION) == IDYES)
                    naviSensor.startStop (true);
            }
            else
            {
                MessageBox (HWND_DESKTOP, "Unable to install service", TITLE_ERROR, MB_ICONSTOP);
            }
        }
    }
    else if (!running)
    {
        if (MessageBox(HWND_DESKTOP, "NaviSensor service is not started.\n\nDo you want to run it now?", "Service installed",
            MB_YESNO | MB_ICONQUESTION) == IDYES)
            naviSensor.startStop (true);
    }

    if (mainWindow.Create ("NaviSensor UI [Inactive]", 100, 100, 800, 600))
    {
        mainWindow.Show (SW_SHOW);
        mainWindow.Update ();

        CWindowWrapper::MessageLoop ();
    }

    return 0;
}
