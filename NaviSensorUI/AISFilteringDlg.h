#pragma once

#include "DialogWrapper.h"
#include "EditWrapper.h"
#include "ButtonWrapper.h"
#include "UpDownWrapper.h"
#include "../NaviSensor/AIS.h"
#include "../NaviSensor/AISConfig.h"

class AISFilteringDlg : public CDialogWrapper
{
    public:
        AISFilteringDlg (HINSTANCE instance, HWND parent, AIS::Filtering *filtering);

    protected:
        AIS::Filtering      *filtering;
        CEditWrapper         maxAmount, maxRange;
        CUpDownWrapper       maxAmountSpin, maxRangeSpin;
        CButtonWrapper       limitAmount, limitRange;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
