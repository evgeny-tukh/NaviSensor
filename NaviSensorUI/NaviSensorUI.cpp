#include "resource.h"
#include <Windows.h>
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
#include "LocalListener.h"

#define IDC_SENSORS     100
#define IDC_PARAM_VALUE 101

#define TIMER_STATE_UPDATE  101

Sensors::SensorConfigArray sensors;

class CSensorListCtrl : public CListCtrlWrapper
{
    public:
        CSensorListCtrl (HWND parent, UINT controlID) : CListCtrlWrapper (parent, controlID) {}
};

class CParamTreeCtrl : public CTreeCtrlWrapper
{
    public:
        CParamTreeCtrl (HWND parent, UINT controlID) : CTreeCtrlWrapper (parent, controlID) {}

        void updateData (Data::LANParamHeader *header, Data::GenericData *data);
        void updateTree ();

        inline void lock () { locker.lock (); }
        inline void unlock () { locker.unlock (); }

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

            void update (Data::GenericData *sourceData, Data::Quality sourceQuality = Data::Quality::Good)
            {
                if (sourceData && data)
                {
                    updateTime = time (0);
                    quality    = sourceQuality;

                    memcpy (data, sourceData, size);
                }
            }

            void update (Data::GenericData& sourceData, Data::Quality sourceQuality = Data::Quality::Good)
            {
                update (& sourceData, sourceQuality);
            }
        };

    protected:
        class TypedParams : public std::map <unsigned char, LANDisplayParamHeader *>
        {
            public:
                TypedParams (Data::DataType type)
                {
                    this->type = type;
                    this->item = 0;
                }

                virtual ~TypedParams ()
                {
                    for (auto & pos : *this)
                    {
                        if (pos.second)
                            delete pos.second;
                    }
                }

                HTREEITEM      item;
                Data::DataType type;
        };

        class AllParams : public std::map <Data::DataType, TypedParams *>
        {
            public:
                virtual ~AllParams ()
                {
                    for (auto & pos : *this)
                    {
                        if (pos.second)
                            delete pos.second;
                    }
                }
        };

        std::mutex locker;
        AllParams  params;
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
        CStaticWrapper           *paramValueCtrl;
        Comm::Socket              transmitter;
        HMENU                     menu;
        bool                      active;
        unsigned int              cmdSeqNumber, stateUpdateTimer;
        HIMAGELIST                sensorImages;
        Sensors::SensorStateArray sensorStates, lastSensorStates;
        SensorWatchWndArray       sensorWatchWindows;
        Comm::LocalListener      *sensorStateListener, *processedDataListener;

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

        void DisplaySensorConfig (Sensors::SensorConfig *config, int item = -1);

        void changeState (const Data::DaemonState newState);

        void waitForListener (Comm::LocalListener *);

        void displayParamValue (HTREEITEM item = 0);
};

void CParamTreeCtrl::updateData (Data::LANParamHeader *dataHeader, Data::GenericData *data)
{
    AllParams::iterator elem = params.find (dataHeader->type);

    if (elem == params.end ())
        elem = params.insert (params.end (), std::pair <Data::DataType, TypedParams *> (dataHeader->type, new TypedParams (dataHeader->type)));

    auto                  map    = elem->second;
    TypedParams::iterator mapPos = map->find (dataHeader->sensorID);

    if (mapPos == map->end ())
    {
        LANDisplayParamHeader dispParam (dataHeader);
        TypedParams          *elemMap = elem->second;

        auto pair = std::pair <unsigned char, LANDisplayParamHeader *> (dataHeader->sensorID, new LANDisplayParamHeader (dataHeader));

        mapPos = elemMap->insert (elemMap->end (), pair);
    }

    mapPos->second->update (data, dataHeader->quality);
}

void CParamTreeCtrl::updateTree ()
{
    lock ();

    for (auto & paramType : params)
    {
        if (paramType.second->item == 0)
            paramType.second->item = InsertItem (Data::getDataTypeName (paramType.second->type), (LPARAM)paramType.second);

        for (auto & param : *(paramType.second))
        {
            if (param.second->item == 0)
                param.second->item = InsertItem (param.second->sensorName, (LPARAM) param.second, paramType.second->item);
        }
    }

    unlock ();
}

void CMainWnd::Initialize()
{
    RECT client;

    stateUpdateTimer = SetTimer (TIMER_STATE_UPDATE, 1000);

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

    paramTreeCtrl->CreateControl(0, 150, 150, client.bottom - 150, WS_BORDER | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT);

    paramValueCtrl = new CStaticWrapper (m_hwndHandle, IDC_PARAM_VALUE);

    paramValueCtrl->CreateControl (150, 150, client.right - 150, client.bottom - 150, WS_BORDER | SS_CENTER);

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
    paramValueCtrl->Move (150, 150, width - 150, height - 150, true);

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
    else if (header->code == TVN_SELCHANGED && header->hwndFrom == paramTreeCtrl->GetHandle ())
    {
        NMTREEVIEW *changeHeader = (NMTREEVIEW *) header;

        displayParamValue (changeHeader->itemNew.hItem);
        /*if (paramTreeCtrl->GetNextItem (changeHeader->itemNew.hItem, TVGN_PARENT) == 0)
        {
            paramValueCtrl->SetText ("");
        }
        else
        {
            char buffer [1024];

            CParamTreeCtrl::LANDisplayParamHeader *param = (CParamTreeCtrl::LANDisplayParamHeader *) paramTreeCtrl->GetItemData (changeHeader->itemNew.hItem);

            paramValueCtrl->SetText (Formatting::getStringFormatValue (param->type, param->data, buffer, sizeof (buffer)));
        }*/
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
                                              paramTreeCtrl->lock ();
                                              
                                              for (int offset = 0; offset < size;)
                                              {
                                                  Data::LANParamHeader *param = (Data::LANParamHeader *) (data + offset);

                                                  paramTreeCtrl->updateData (param, (Data::GenericData *) (param + 1));

                                                  offset += sizeof(Data::LANParamHeader) + param->size;
                                              }

                                              paramTreeCtrl->unlock ();
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

    sensors.loadAll ();

    sensorListCtl = NULL;

    WSAStartup (MAKEWORD (1, 1), & winsockData);

    sensorStateListener   = new Comm::LocalListener (Comm::Ports::SensorPort, & active, sensorStateCb);
    processedDataListener = new Comm::LocalListener (Comm::Ports::ProcessedDataPort, &active, processedDataCb);

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

    transmitter.close ();

    ImageList_Destroy (sensorImages);
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
    if (timerID == TIMER_STATE_UPDATE)
    {
        MENUITEMINFO              menuItemInfo;
        char                      caption[256];

        if ((time (0) - daemonStateUpdate) > 2)
            daemonState = Data::DaemonState::Inactive;

        paramTreeCtrl->updateTree ();
        
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

    if (mainWindow.Create ("NaviSensor UI [Inactive]", 100, 100, 800, 600))
    {
        mainWindow.Show (SW_SHOW);
        mainWindow.Update ();

        CWindowWrapper::MessageLoop ();
    }

    return 0;
}
