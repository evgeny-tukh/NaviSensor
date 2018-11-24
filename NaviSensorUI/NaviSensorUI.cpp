#include "resource.h"
#include <Windows.h>
#include <commctrl.h>
#include "WindowWrapper.h"
#include "ListCtrlWrapper.h"
#include "InputBox.h"
#include "SensorPropsDlg.h"
#include "SensorInfoWnd.h"
#include "SensorConfig.h"
#include <thread>
#include "tools.h"
#include "../NaviSensor/DataDef.h"
#include "../NaviSensor/Sensor.h"
#include "../NaviSensor/Network.h"

#define IDC_SENSORS     100

Sensors::SensorConfigArray sensors;

class CSensorListCtrl : public CListCtrlWrapper
{
    public:
        CSensorListCtrl (HWND parent, UINT controlID) : CListCtrlWrapper (parent, controlID) {}
};

class CMainWnd : public CWindowWrapper
{
    public:
        CMainWnd (HINSTANCE instance);
        virtual ~CMainWnd ();

    private:
        Data::DaemonState daemonState;
        time_t            daemonStateUpdate;
        CSensorListCtrl  *sensorListCtl;
        Comm::Socket      transmitter;
        std::thread       stateUpdater;
        HMENU             menu;
        bool              active;
        unsigned int      cmdSeqNumber;
        HIMAGELIST        sensorImages;

        virtual LRESULT OnMessage (UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD requestType, const WORD width, const WORD height);
        virtual LRESULT OnNotify (NMHDR *header);

        void RequestAppQuit ();

        void Initialize ();
        void AddSensor ();
        void EditSensor ();
        void DeleteSensor ();
        void WatchSensor ();
        void StartStop ();

        void DisplaySensorConfig (Sensors::SensorConfig *config, int item = -1);

        void stateUpdaterProc ();
        static void stateUpdaterProcInternal (CMainWnd *);

        void changeState (const Data::DaemonState newState);
};

void CMainWnd::stateUpdaterProc()
{
    Data::DaemonState lastState = Data::DaemonState::Inactive;
    MENUITEMINFO      menuItemInfo;
    char              caption [256];

    Tools::sleepFor (200);

    while (active)
    {
        if ((time (0) - daemonStateUpdate) > 2)
            daemonState = Data::DaemonState::Inactive;

        if (lastState != daemonState && IsWindow (m_hwndHandle))
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

            SetText (caption);
            //PostMessage (m_hwndHandle, WM_SETTEXT, 0, (LPARAM) caption);

            lastState = daemonState;

            memset (& menuItemInfo, 0, sizeof (menuItemInfo));

            menuItemInfo.cbSize     = sizeof (menuItemInfo);
            menuItemInfo.fMask      = MIIM_STRING | MIIM_STATE;
            menuItemInfo.fState     = daemonState == Data::DaemonState::Inactive ? MFS_GRAYED : 0;
            menuItemInfo.dwTypeData = (char *) (daemonState == Data::DaemonState::Running ? "Stop" : "Start");

            SetMenuItemInfo (GetSubMenu (m_hmnuMenu, 1), ID_START, FALSE, & menuItemInfo);
        }

        Tools::sleepFor (500);
    }
}

void CMainWnd::stateUpdaterProcInternal (CMainWnd *self)
{
    if (self)
        self->stateUpdaterProc ();
}

void CMainWnd::Initialize()
{
    RECT client;

    GetClientRect (& client);

    sensorListCtl = new CSensorListCtrl (m_hwndHandle, IDC_SENSORS);

    sensorListCtl->CreateControl (0, 50, client.right, client.bottom - 200, LVS_REPORT);
    sensorListCtl->SetWholeLineSelection ();
    sensorListCtl->AddColumn ("Name", 200);
    sensorListCtl->AddColumn ("Type", 80);
    sensorListCtl->AddColumn ("Conn type", 80);
    sensorListCtl->AddColumn ("Comm parameters", client.right - 330);

    sensorListCtl->SendMessage (LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) sensorImages);

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
    sensorListCtl->Move (0, 0, width, height, TRUE);

    return FALSE;
}

LRESULT CMainWnd::OnNotify (NMHDR *header)
{
    if (header->hwndFrom == sensorListCtl->GetHandle ())
    {
        if (header->code == NM_DBLCLK)
        {
            NMITEMACTIVATE *pItemActData = (NMITEMACTIVATE *) header;
        }
    }

    return FALSE;
}

CMainWnd::CMainWnd (HINSTANCE instance) :
    active (true),
    cmdSeqNumber (0),
    CWindowWrapper (instance, HWND_DESKTOP, "NaviSensorUIWnd", menu = LoadMenu (instance, MAKEINTRESOURCE(IDR_MAINMENU))),
    daemonState (Data::Inactive),
    daemonStateUpdate (0),
    transmitter(),
    stateUpdater (stateUpdaterProcInternal, this)
{
    WSADATA              winsockData;
    Comm::Socket::ReadCb callback = [this] (char *data, const int size) -> void
                                    {
                                        daemonStateUpdate = time (0);
                                        daemonState       = (Data::DaemonState) *data;
                                    };
    std::thread         *listener;

    sensorImages = ImageList_LoadBitmap (instance, MAKEINTRESOURCE (IDB_SENSOR), 16, 16, CLR_NONE);

    sensors.loadAll ();

    sensorListCtl = NULL;

    WSAStartup (MAKEWORD (1, 1), & winsockData);

    listener = new std::thread ([this, callback] () -> void
                                {
                                    Comm::Socket     receiver;
                                    char             buffer[10000];
                                    in_addr          sender,
                                                     localhost;
                                    int              bytesRead;
                                    Tools::AddrArray interfaces;

                                    Tools::getInterfaceList (interfaces);

                                    localhost.S_un.S_addr = htonl (INADDR_LOOPBACK);

                                    interfaces.push_back (localhost);

                                    receiver.create (8001);

                                    while (active)
                                    {
                                        bytesRead = receiver.receiveFrom (buffer, sizeof (buffer), sender);

                                        if (bytesRead > 0)
                                        {
                                            bool isLocal = false;

                                            for (auto & addr : interfaces)
                                            {
                                                if (addr.S_un.S_addr == sender.S_un.S_addr)
                                                {
                                                    isLocal = true; break;
                                                }
                                            }

                                            if (isLocal)
                                                callback (buffer, bytesRead);
                                        }

                                        Tools::sleepFor (5);
                                    }
                                });

    listener->detach ();

    transmitter.create ();

    delete listener;
}

CMainWnd::~CMainWnd ()
{
    active = false;

    stateUpdater.join ();

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
            std::thread watcher ([] (CMainWnd *self, Sensors::SensorConfig *sensorCfg) -> void
                                 {
                                    SensorInfoWnd window (NULL, self->GetHandle (), sensorCfg);
                                    HWND          wndHandle;
                                    MSG           msgMessage;

                                    window.Show ();

                                    wndHandle = window.GetHandle ();

                                    while (GetMessage (& msgMessage, wndHandle, NULL, NULL))
                                    {
                                        TranslateMessage (& msgMessage);
                                        DispatchMessage (& msgMessage);
                                    }

                                    Tools::sleepFor (100);
                                 },
                                 this, sensorCfg);

            watcher.detach ();
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
