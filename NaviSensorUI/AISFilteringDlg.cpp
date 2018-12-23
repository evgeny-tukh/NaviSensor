#include <Windows.h>
#include "AISFilteringDlg.h"
#include "resource.h"

AISFilteringDlg::AISFilteringDlg (HINSTANCE instance, HWND parent, AIS::Filtering *filtering) :
    CDialogWrapper (instance, parent, IDD_AIS_FILTERING),
    maxAmount (IDC_MAX_TARGETS_NUMBER), maxRange (IDC_MAX_TARGET_RANGE),
    maxAmountSpin (IDC_MAX_TARGETS_NUMBER_SPIN), maxRangeSpin (IDC_MAX_TARGET_RANGE_SPIN),
    limitAmount (IDC_LIMIT_TARGET_AMOUNT), limitRange (IDC_LIMIT_TARGET_RANGE)
{
    this->filtering = filtering;
}

LRESULT AISFilteringDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CDialogWrapper::OnCommand(wParam, lParam);

    switch (LOWORD(wParam))
    {
        case IDC_LIMIT_TARGET_AMOUNT:
        {
            BOOL enableFiltering = limitAmount.IsChecked ();

            maxAmount.Enable (enableFiltering);
            maxAmountSpin.Enable (enableFiltering);

            break;
        }

        case IDC_LIMIT_TARGET_RANGE:
        {
            BOOL enableFiltering = limitRange.IsChecked ();

            maxRange.Enable (enableFiltering);
            maxRangeSpin.Enable (enableFiltering);

            break;
        }
    }

    return result;
}

BOOL AISFilteringDlg::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    BOOL result = CDialogWrapper::OnInitDialog (wParam, lParam);

    maxAmount.FindAndAttach (m_hwndHandle);
    maxAmountSpin.FindAndAttach (m_hwndHandle);
    maxRange.FindAndAttach (m_hwndHandle);
    maxRangeSpin.FindAndAttach (m_hwndHandle);
    limitAmount.FindAndAttach (m_hwndHandle);
    limitRange.FindAndAttach (m_hwndHandle);

    maxRangeSpin.SetRange (1, 200);
    maxAmountSpin.SetRange (5, 1000);

    limitAmount.Check (filtering->limitAmount);
    limitRange.Check (filtering->limitRange);

    maxAmount.SetInt (filtering->maxAmount);
    maxRange.SetInt (filtering->maxRange);

    maxAmount.Enable (filtering->limitAmount);
    maxAmountSpin.Enable (filtering->limitAmount);

    maxRange.Enable (filtering->limitRange);
    maxRangeSpin.Enable (filtering->limitRange);

    return result;
}

void AISFilteringDlg::OnOK ()
{
    filtering->limitAmount = limitAmount.IsChecked ();
    filtering->limitRange  = limitRange.IsChecked ();
    filtering->maxAmount   = maxAmount.GetInt ();
    filtering->maxRange    = maxRange.GetInt ();

    CDialogWrapper::OnOK ();
}

