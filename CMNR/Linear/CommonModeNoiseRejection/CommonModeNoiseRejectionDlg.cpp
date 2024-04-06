#include "pch.h"
#include "CommonModeNoiseRejection.h"
#include "CommonModeNoiseRejectionDlg.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "HFST_RandomPtTest/RandomPointTest.h"

#include "CPointCloud.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//#define EANBLE_TEK_VISA
#define LINEAR_ENABLE
#define TEKVISA_RAMP_TIME		     250
#define TEST_1000_POINT

#define INI_FILE_NAME					_T("CCommonModeNoiseRejectionDlg.ini")
#define INI_SEC_LINEAR_MACHINE	        _T("LinearMachine")
#define INI_SEC_TP_SIZE					_T("TP_SIZE")
#define INI_SEC_FUNCTION_GEN	        _T("FunctionGen")
#define INI_KEY_WAVEFORM				_T("WaveForm")
#define INI_KEY_DUTYCYCLE				_T("DutyCycle")
#define INI_KEY_AMPL_MAX				_T("AmplitudeMax")
#define INI_KEY_AMPL_MIN				_T("AmplitudeMin")
#define INI_KEY_AMPL_STEP				_T("AmplitudeStep")
#define INI_KEY_FREQ_MAX				_T("FrequencyMax")
#define INI_KEY_FREQ_MIN				_T("FrequencyMin")
#define INI_KEY_FREQ_STEP_V				_T("FrequencyStep")
#define INI_KEY_IMP						_T("Impedance")
#define INI_KEY_ROUND_COUNT				_T("RoundCount")
#define INI_KEY_X					    _T("X")
#define INI_KEY_Y				        _T("Y")
#define INI_KEY_TOUCH_HIGH  			_T("TouchHigh")
#define INI_KEY_LIFTINT_HIGH			_T("LiftingHigh")
#define INI_KEY_STOP_TIME				_T("StopTime")
#define INI_SEC_NUMBER					_T("SecNumber")
#define INI_BORDER_ERROR				_T("BorderError")
#define INI_NON_BORDER_ERROR			_T("NonBorderError")
#define INI_X0							_T("X0")
#define INI_Y0							_T("Y0")
#define INI_X1							_T("X1")
#define INI_Y1							_T("Y1")
#define INI_X2							_T("X2")
#define INI_Y2							_T("Y2")
#define INI_X3							_T("X3")
#define INI_Y3							_T("Y3")
#define INI_RESX						_T("ResX")
#define INI_RESY						_T("ResY")

#define CMNR_TIMER_LINEAR_AXIS_LIMIT    1       //Timer 事件
#define PLUS_MINUS_LIMIT_DISTANCE       330000  //正负限位之间的距离（单位μm）

CCommonModeNoiseRejectionDlg::CCommonModeNoiseRejectionDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMMONMODENOISEREJECTION_DIALOG, pParent)
	, m_CMNRTestThread(NULL)
	, m_StopThread(false)
	, m_iTestFreq(0)
	, m_fTestAmpl(0.0)
	, m_fPosValueX(0.0)
	, m_fPosValueY(0.0)
	, m_fPosValueZ(0.0)
	, m_eTestResult(TestResult::NONE)
	, m_rErrorTypeSel{ true, true, true }
	, m_gdiplusToken(0)
	, m_nRange(4.0f)
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	memset(&m_CmnrCfg, 0, sizeof(CMNRCfg));
	Gdiplus::GdiplusStartupInput gpSI;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gpSI, NULL);
}

CCommonModeNoiseRejectionDlg::~CCommonModeNoiseRejectionDlg()
{
	m_StopThread = true;
	if (m_CMNRTestThread)
	{
		m_CMNRTestThread = NULL;
	}
    m_fPosValueX = 0.0;
    m_fPosValueY = 0.0;
    m_fPosValueZ = 0.0;

	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	if (m_pLinearTest)
	{
		delete m_pLinearTest;
		m_pLinearTest = nullptr;
	}

	if ( m_PtCloudWin )
	{
		delete m_PtCloudWin;
	}
}

void CCommonModeNoiseRejectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_LEFT_X, m_BtnLeftX);
	DDX_Control(pDX, IDC_BTN_RIGHT_X, m_BtnRightX);
	DDX_Control(pDX, IDC_BTN_UP_Y, m_BtnUpY);
	DDX_Control(pDX, IDC_BTN_DOWN_Y, m_BtnDownY);
	DDX_Control(pDX, IDC_BTN_UP_Z, m_BtnUpZ);
	DDX_Control(pDX, IDC_BTN_DOWN_Z, m_BtnDownZ);
	DDX_Control(pDX, IDC_BTN_HOME, m_BtnHome);
	DDX_Control(pDX, IDC_COMBO_WAVEFORM, m_ComboWaveForm);
	DDX_Control(pDX, IDC_STATIC_MSG, m_StaticProMsg);
	DDX_Control(pDX, IDC_PROG_FG, m_FgProgress);
	DDX_Control(pDX, IDC_BTN_CMNR_START, m_BtnCMNRStart);
	DDX_Control(pDX, IDC_SLIDER_DUTY, m_SliderDuty);
	DDX_Control(pDX, IDC_LIST_RESULT, m_listResult);
	DDX_Control(pDX, IDC_LIST_PATH_TYPE, m_PathTypeList);
	DDX_Control(pDX, IDC_COMBO_LAPS, m_LapCombo);
	DDX_Control(pDX, IDC_PROGRESS_LINEAR_PROGRESS_BAR, m_LinearTProbar);
	DDX_Control(pDX, IDC_COMBO_TP_TYPE, m_ComboTpType);
	DDX_Control(pDX, IDC_EDIT_RANGE, m_Range);
	DDX_Control(pDX, IDC_SLIDER_SPEED, m_SpeedSlide);
}

BEGIN_MESSAGE_MAP(CCommonModeNoiseRejectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CMNR_START, &CCommonModeNoiseRejectionDlg::OnBnClickedBtnCmnrStart)
	ON_CBN_SELCHANGE(IDC_COMBO_WAVEFORM, &CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboWaveform)
	ON_BN_CLICKED(IDC_BTN_HOME, &CCommonModeNoiseRejectionDlg::OnBnClickedBtnHome)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_ERROR_TYPE_SEL, &CCommonModeNoiseRejectionDlg::OnBnClickedBtnErrorTypeSel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PATH_TYPE, &CCommonModeNoiseRejectionDlg::OnLvnItemchangedListPathType)
	ON_CBN_SELCHANGE(IDC_COMBO_LAPS, &CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboLaps)
	ON_BN_CLICKED(IDC_MFCBUTTON_RUN, &CCommonModeNoiseRejectionDlg::OnBnClickedMfcbuttonRun)
	ON_CBN_SELCHANGE(IDC_COMBO_TP_TYPE, &CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboTpType)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SPEED, &CCommonModeNoiseRejectionDlg::OnNMCustomdrawSliderSpeed)
	ON_CBN_SELCHANGE(IDC_COMBO_SELECT, &CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboSelect)
	ON_BN_CLICKED(IDC_RADIO_NORMAL_CMNR_TEST, &CCommonModeNoiseRejectionDlg::OnBnClickedRadioNormalCmnrTest)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_POINT_FILE, &CCommonModeNoiseRejectionDlg::OnBnClickedButtonLoadPointFile)
	ON_BN_CLICKED(IDC_RADIO_1000_RANDOM, &CCommonModeNoiseRejectionDlg::OnBnClickedRadio1000Random)
	ON_BN_CLICKED(IDC_CHECK_ACCURATE, &CCommonModeNoiseRejectionDlg::OnBnClickedCheckAccurate)

	ON_BN_CLICKED(IDC_CHECK_ACCURATE, &CCommonModeNoiseRejectionDlg::OnBnClickedCheckAccurate)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_POINT, &CCommonModeNoiseRejectionDlg::OnBnClickedButtonShowPoint)
//	ON_BN_CLICKED(IDC_BUTTON2, &CCommonModeNoiseRejectionDlg::OnBnClickedButton2)
//	ON_BN_CLICKED(IDC_BUTTON3, &CCommonModeNoiseRejectionDlg::OnBnClickedButton3)
	ON_EN_CHANGE(IDC_EDIT_OFFSET_X, &CCommonModeNoiseRejectionDlg::OnEnChangeEditOffsetX)
	ON_EN_CHANGE(IDC_EDIT_OFFSET_Y, &CCommonModeNoiseRejectionDlg::OnEnChangeEditOffsetY)
	ON_BN_CLICKED(IDC_CHECK_DEBUG, &CCommonModeNoiseRejectionDlg::OnBnClickedCheckAccurate2)
	ON_BN_CLICKED(IDC_BUTTON2, &CCommonModeNoiseRejectionDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CCommonModeNoiseRejectionDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CCommonModeNoiseRejectionDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CCommonModeNoiseRejectionDlg::OnBnClickedButton5)
	ON_EN_CHANGE(IDC_EDIT_INNER_SIZE, &CCommonModeNoiseRejectionDlg::OnEnChangeEditInnerSize)
	ON_EN_CHANGE(IDC_EDIT_RESX, &CCommonModeNoiseRejectionDlg::OnEnChangeEditResx)
	ON_EN_CHANGE(IDC_EDIT_RESY, &CCommonModeNoiseRejectionDlg::OnEnChangeEditResy)
	ON_EN_CHANGE(IDC_EDIT_CNTX, &CCommonModeNoiseRejectionDlg::OnEnChangeEditCntx)
	ON_EN_CHANGE(IDC_EDIT_CNTY, &CCommonModeNoiseRejectionDlg::OnEnChangeEditCnty)
	ON_CBN_SELCHANGE(IDC_COMBO_COORD_ZERO, &CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboCoordZero)
END_MESSAGE_MAP()

bool LoadImageFromResource(IN CImage* pImage,
	IN UINT nResID,
	IN LPCWSTR lpTyp)
{
	if (pImage == NULL) return false;

	pImage->Destroy();

	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	::GlobalUnlock(hNew);

	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		pImage->Load(pStream);

		GlobalFree(hNew);
	}

	::FreeResource(hImgData);
	return true;
}

void GetAppDir(CString* pStrDir) {

    TCHAR AppDirTmep[512] = { 0 };
    TCHAR BufTemp[512] = { 0 };
    CString StrTemp;
    GetModuleFileName(NULL, BufTemp, 256);
    StrTemp = BufTemp;
    _tcscpy_s(AppDirTmep, StrTemp.Left(StrTemp.ReverseFind('\\')));

    pStrDir->Format(_T("%s"), AppDirTmep);

}

void  CCommonModeNoiseRejectionDlg::DefaultCmnrConf() {
    //WaveForm
    m_CmnrCfg.iWaveform = 0;
    //DutyCycle
    m_CmnrCfg.iDutyCycle = 50;
    //Amplitude
	m_CmnrCfg.iMaxFreq = 500;
	m_CmnrCfg.iMinFreq = 100;
	m_CmnrCfg.iIncFreq = 100;
	m_CmnrCfg.fMaxAmpl = 10.0f;
	m_CmnrCfg.fMinAmpl = 2.0f;
	m_CmnrCfg.fAmpStep = 2.0f;
	m_CmnrCfg.fAxis_X = 0.0;
	m_CmnrCfg.fAxis_Y = 0.0;
	m_CmnrCfg.fTouchHigh = 40.0;
	m_CmnrCfg.fSafeHigh = 20.0;
	m_CmnrCfg.iStopTime = 1000;
}

void CCommonModeNoiseRejectionDlg::SaveCmnrIni() {
    CIniFile IniFile;
    CIniSection* pSection;
    CString fileName;
    CString strSection;
    CString strKey;
    CString strKeyValue;
    GetAppDir(&fileName);
    fileName.AppendFormat(_T("\\"));
    fileName.AppendFormat(INI_FILE_NAME);
    if (!IniFile.Load(fileName.GetString(), false)) {
        //[AntiNoiseConf] ==========
        //WaveForm
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iWaveform);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_WAVEFORM)->SetValue(strKeyValue.GetBuffer(0));
        //DutyCycle
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iDutyCycle);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_DUTYCYCLE)->SetValue(strKeyValue.GetBuffer(0));
        //Amplitude
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fMaxAmpl);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_AMPL_MAX)->SetValue(strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fMinAmpl);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_AMPL_MIN)->SetValue(strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fAmpStep);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_AMPL_STEP)->SetValue(strKeyValue.GetBuffer(0));
        //Frequency
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iMaxFreq);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_FREQ_MAX)->SetValue(strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iMinFreq);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_FREQ_MIN)->SetValue(strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iIncFreq);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_FREQ_STEP_V)->SetValue(strKeyValue.GetBuffer(0));
        //Impedance
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iImped);
        IniFile.AddSection(INI_SEC_FUNCTION_GEN)->AddKey(INI_KEY_IMP)->SetValue(strKeyValue.GetBuffer(0));
       
        //Test linear machine
        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fAxis_X);
        IniFile.AddSection(INI_SEC_LINEAR_MACHINE)->AddKey(INI_KEY_X)->SetValue(strKeyValue.GetBuffer(0));

        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fAxis_Y);
        IniFile.AddSection(INI_SEC_LINEAR_MACHINE)->AddKey(INI_KEY_Y)->SetValue(strKeyValue.GetBuffer(0));

        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fTouchHigh);
        IniFile.AddSection(INI_SEC_LINEAR_MACHINE)->AddKey(INI_KEY_TOUCH_HIGH)->SetValue(strKeyValue.GetBuffer(0));

        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fSafeHigh);
        IniFile.AddSection(INI_SEC_LINEAR_MACHINE)->AddKey(INI_KEY_LIFTINT_HIGH)->SetValue(strKeyValue.GetBuffer(0));
        
        //Test Time Interval
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iStopTime);
        IniFile.AddSection(INI_SEC_LINEAR_MACHINE)->AddKey(INI_KEY_STOP_TIME)->SetValue(strKeyValue.GetBuffer(0));
        

    }
    else {
        //[AntiNoiseConf] ==========
        strSection = INI_SEC_FUNCTION_GEN;
        pSection = IniFile.GetSection(strSection.GetBuffer(0));
        if (!pSection) {
            IniFile.AddSection(INI_SEC_FUNCTION_GEN);
            pSection = IniFile.GetSection(strSection.GetBuffer(0));
        }
        //WaveForm
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iWaveform);
        pSection->SetKeyValue(INI_KEY_WAVEFORM, strKeyValue.GetBuffer(0));
        //DutyCycle
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iDutyCycle);
        pSection->SetKeyValue(INI_KEY_DUTYCYCLE, strKeyValue.GetBuffer(0));
        //Amplitude
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fMaxAmpl);
        pSection->SetKeyValue(INI_KEY_AMPL_MAX, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fMinAmpl);
        pSection->SetKeyValue(INI_KEY_AMPL_MIN, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.3f"), m_CmnrCfg.fAmpStep);
        pSection->SetKeyValue(INI_KEY_AMPL_STEP, strKeyValue.GetBuffer(0));
        //Frequency
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iMaxFreq);
        pSection->SetKeyValue(INI_KEY_FREQ_MAX, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iMinFreq);
        pSection->SetKeyValue(INI_KEY_FREQ_MIN, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iIncFreq);
        pSection->SetKeyValue(INI_KEY_FREQ_STEP_V, strKeyValue.GetBuffer(0));
        //Impedance
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iImped);
        pSection->SetKeyValue(INI_KEY_IMP, strKeyValue.GetBuffer(0));
        //Test Time Interval
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iRound);
        pSection->SetKeyValue(INI_KEY_ROUND_COUNT, strKeyValue.GetBuffer(0));

        //[Linear Machine] ==========
        strSection = INI_SEC_LINEAR_MACHINE;
        pSection = IniFile.GetSection(strSection.GetBuffer(0));
        if (!pSection) {
            IniFile.AddSection(INI_SEC_LINEAR_MACHINE);
            pSection = IniFile.GetSection(strSection.GetBuffer(0));
        }
        //X
        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fAxis_X);
        pSection->SetKeyValue(INI_KEY_X, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fAxis_Y);
        pSection->SetKeyValue(INI_KEY_Y, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fTouchHigh);
        pSection->SetKeyValue(INI_KEY_TOUCH_HIGH, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%.2f"), m_CmnrCfg.fSafeHigh);
        pSection->SetKeyValue(INI_KEY_LIFTINT_HIGH, strKeyValue.GetBuffer(0));
        strKeyValue.Format(_T("%d"), m_CmnrCfg.iStopTime);
        pSection->SetKeyValue(INI_KEY_STOP_TIME, strKeyValue.GetBuffer(0));

		//[TP_SIZE] ==========
		strSection = INI_SEC_TP_SIZE;
		pSection = IniFile.GetSection(strSection.GetBuffer(0));
		if (!pSection) {
			IniFile.AddSection(INI_SEC_TP_SIZE);
			pSection = IniFile.GetSection(strSection.GetBuffer(0));
		}
		strKeyValue.Format(_T("%.3f"), m_CornerPts[0].first);
		pSection->SetKeyValue(INI_X0, strKeyValue.GetBuffer(0));
		strKeyValue.Format(_T("%.3f"), m_CornerPts[0].second);
		pSection->SetKeyValue(INI_Y0, strKeyValue.GetBuffer(0));

		strKeyValue.Format(_T("%.3f"), m_CornerPts[1].first);
		pSection->SetKeyValue(INI_X1, strKeyValue.GetBuffer(0));
		strKeyValue.Format(_T("%.3f"), m_CornerPts[1].second);
		pSection->SetKeyValue(INI_Y1, strKeyValue.GetBuffer(0));

		strKeyValue.Format(_T("%.3f"), m_CornerPts[2].first);
		pSection->SetKeyValue(INI_X2, strKeyValue.GetBuffer(0));
		strKeyValue.Format(_T("%.3f"), m_CornerPts[2].second);
		pSection->SetKeyValue(INI_Y2, strKeyValue.GetBuffer(0));

		strKeyValue.Format(_T("%.3f"), m_CornerPts[3].first);
		pSection->SetKeyValue(INI_X3, strKeyValue.GetBuffer(0));
		strKeyValue.Format(_T("%.3f"), m_CornerPts[3].second);
		pSection->SetKeyValue(INI_Y3, strKeyValue.GetBuffer(0));

		strKeyValue.Format(_T("%d"), m_ResolutionX);
		pSection->SetKeyValue(INI_RESX, strKeyValue.GetBuffer(0));
		strKeyValue.Format(_T("%d"), m_ResolutionY);
		pSection->SetKeyValue(INI_RESY, strKeyValue.GetBuffer(0));

    }
    IniFile.Save(fileName.GetString());
}

void CCommonModeNoiseRejectionDlg::LoadIniFile() {
    CIniFile IniFile;
    CIniSection* pSection;
    CString fileName;
    CString strSection;
    CString strKey;
    CString strKeyValue;
    double KeyValue;
    GetAppDir(&fileName);
    fileName.AppendFormat(_T("\\"));
    fileName.AppendFormat(INI_FILE_NAME);
    if (!IniFile.Load(fileName.GetString(), false)) {
        // create default config
		DefaultCmnrConf();
		SaveCmnrIni();

    }
	else {
		//Load cfg from INI
		strSection = INI_SEC_LINEAR_MACHINE;
		pSection = IniFile.GetSection(strSection.GetBuffer(0));
		if (pSection) {
			//BUS
			//Linear machine
			strKey = INI_KEY_X;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fAxis_X = (float)KeyValue;

			strKey = INI_KEY_Y;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fAxis_Y = (float)KeyValue;

			strKey = INI_KEY_TOUCH_HIGH;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fTouchHigh = (float)KeyValue;

			strKey = INI_KEY_LIFTINT_HIGH;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fSafeHigh = (float)KeyValue;

			//Test Time Interval
			strKey = INI_KEY_STOP_TIME;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iStopTime = (int)KeyValue;
		}
		else {
			DefaultCmnrConf();
			SaveCmnrIni();
		}

		strSection = INI_SEC_FUNCTION_GEN;
		pSection = IniFile.GetSection(strSection.GetBuffer(0));
		if (pSection) {
			//WaveForm
			strKey = _T("");
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iWaveform = (int)KeyValue;
			//DutyCycle
			strKey = INI_KEY_DUTYCYCLE;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iDutyCycle = (int)KeyValue;
			//Amplitude
			strKey = INI_KEY_AMPL_MAX;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fMaxAmpl = (float)KeyValue;
			strKey = INI_KEY_AMPL_MIN;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fMinAmpl = (float)KeyValue;
			strKey = INI_KEY_AMPL_STEP;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.fAmpStep = (float)KeyValue;
			//Frequency
			strKey = INI_KEY_FREQ_MAX;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iMaxFreq = (int)KeyValue;
			strKey = INI_KEY_FREQ_MIN;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iMinFreq = (int)KeyValue;
			strKey = INI_KEY_FREQ_STEP_V;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iIncFreq = (int)KeyValue;
			//Imedance
			strKey = INI_KEY_IMP;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iImped = (int)KeyValue;

			//Test Round
			strKey = INI_KEY_ROUND_COUNT;
			KeyValue = _wtof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str());
			m_CmnrCfg.iRound = (int)KeyValue;

		}
		else {
			//Create default config
			DefaultCmnrConf();
			SaveCmnrIni();
		}

		strSection = INI_SEC_TP_SIZE;
		pSection = IniFile.GetSection(strSection.GetBuffer(0));
		if (pSection) {
			strKey = INI_SEC_NUMBER;
			KeyValue = wcstol(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), NULL, 10);
			m_SecNumber = (int)KeyValue;

			strKey = INI_BORDER_ERROR;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			m_BorderThres = (float)KeyValue;

			strKey = INI_NON_BORDER_ERROR;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			m_NonBorderThres = (float)KeyValue;

			m_CornerPts.clear();
			strKey = INI_X0;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			float x = (float)KeyValue;
			strKey = INI_Y0;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			float y = (float)KeyValue;
			m_CornerPts.emplace_back(x,y);

			strKey = INI_X1;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			x = (float)KeyValue;
			strKey = INI_Y1;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			y = (float)KeyValue;
			m_CornerPts.emplace_back(x, y);

			strKey = INI_X2;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			x = (float)KeyValue;
			strKey = INI_Y2;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			y = (float)KeyValue;
			m_CornerPts.emplace_back(x, y);

			strKey = INI_X3;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			x = (float)KeyValue;
			strKey = INI_Y3;
			KeyValue = wcstof(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr);
			y = (float)KeyValue;
			m_CornerPts.emplace_back(x, y);

			strKey = INI_RESX;
			KeyValue = wcstol(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr, 10);
			m_ResolutionX = KeyValue;

			strKey = INI_RESY;
			KeyValue = wcstol(pSection->GetKeyValue(strKey.GetBuffer(0)).c_str(), nullptr, 10);
			m_ResolutionY = KeyValue;
		}
	}
}

void  CCommonModeNoiseRejectionDlg::EnableLinearOperation(bool bEnable)
{
    m_BtnLeftX.EnableWindow(bEnable);
    m_BtnRightX.EnableWindow(bEnable);
    m_BtnUpY.EnableWindow(bEnable);
    m_BtnDownY.EnableWindow(bEnable);
    m_BtnUpZ.EnableWindow(bEnable);
    m_BtnDownZ.EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_POS_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_POS_Y)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_TOUCH_HIGH)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_SAFE_HIGH)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_STOP_TIME)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_IMP)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ROUND_CNT)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_MAX_FREQ)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_MIN_FREQ)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_INC_FREQ)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_MAX_AMP)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_MIN_AMP)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_STEP_AMP)->EnableWindow(bEnable);
    GetDlgItem(IDC_COMBO_WAVEFORM)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_DUTY)->EnableWindow(bEnable);
    GetDlgItem(IDC_SLIDER_DUTY)->EnableWindow(bEnable);
}

void CCommonModeNoiseRejectionDlg::ParseData(unsigned int nTimeStamp, bool bIsBProtocol)
{
	std::cout << "m_mapTrackingIdPos size = " << m_mapTrackingIdPos.size() << std::endl;

    for (unsigned int i = 0; i < m_VecPos.size(); i++)
    {
		std::cout << "m_VecPos[" << i << "] = " << std::endl;
		std::cout << m_VecPos[i].trackID << std::endl;
		std::cout << m_VecPos[i].x << std::endl;
		std::cout << m_VecPos[i].y << std::endl;

        if ((0 == m_VecPos[i].x) && (0 == m_VecPos[i].y))
            continue;

		if ( m_VecPos[i].trackID == 0 )
			continue;
        
        if (m_mapTrackingIdPos.empty())
        {
            if (m_VecBackLastPos.size() > i) {
                if ((0 == m_VecPos[i].y) && m_VecPos[i].x > 0) //只有X坐标，Y没有变化
                {
                    //m_VecPos[i].y = m_VecBackLastPos[i].y;
                    m_VecPos[i].y = m_VecBackLastPos.back().y;
                }
                if ((0 == m_VecPos[i].x) && m_VecPos[i].y > 0) //只有Y坐标，X没有变化
                {
                    //m_VecPos[i].x = m_VecBackLastPos[i].x;
                    m_VecPos[i].x = m_VecBackLastPos.back().x;
                }
                if (0 == m_VecPos[i].trackID && m_VecBackLastPos[i].trackID != 0)
                    m_VecPos[i].trackID = m_VecBackLastPos[i].trackID;
            }
			else if (((0 == m_VecPos[i].y) && m_VecPos[i].x > 0) || ((0 == m_VecPos[i].x) && m_VecPos[i].y > 0))
			{
                continue; //没有上次的坐标，只有X或Y，不保存
			}

			m_VecPos[i].timeTouch = (unsigned int)GetTickCount64();
            if (bIsBProtocol)
                m_mapTrackingIdPos[i].push_back(m_VecPos[i]);
            else
                m_mapTrackingIdPos[m_VecPos[i].trackID].push_back(m_VecPos[i]);
        }
        else
        {
			m_VecPos[i].timeTouch = (unsigned int)GetTickCount64();
            if (bIsBProtocol) { //B协议

				if (0xffffffff == m_VecPos[i].trackID) {
					return;
				}

                if (m_mapTrackingIdPos.find(i) != m_mapTrackingIdPos.end())
                {
                    if ((0 == m_VecPos[i].y) && m_VecPos[i].x > 0) //只有X坐标，Y没有变化
                    {
                        if (m_mapTrackingIdPos[i].size() > 0)
                            m_VecPos[i].y = m_mapTrackingIdPos[i].back().y;
                        else
                            continue;
                    }
                    if ((0 == m_VecPos[i].x) && m_VecPos[i].y > 0) //只有Y坐标，X没有变化
                    {
                        if (m_mapTrackingIdPos[i].size() > 0)
                            m_VecPos[i].x = m_mapTrackingIdPos[i].back().x;
                        else
                            continue;
                    }
                    if (0 == m_VecPos[i].trackID)
                        m_VecPos[i].trackID = m_mapTrackingIdPos[i].back().trackID;

                    auto itrFinder = std::find_if(m_mapTrackingIdPos[i].begin(), m_mapTrackingIdPos[i].end(), [&](const auto& val) {
                        return (m_VecPos[i].x == val.x) && (m_VecPos[i].y == val.y);
                        });
                    if (itrFinder == m_mapTrackingIdPos[i].end()) {
                        m_mapTrackingIdPos[i].push_back(m_VecPos[i]);
                    }
                }
                else {
                    m_mapTrackingIdPos[i].push_back(m_VecPos[i]);
                }
            }
            else   //A协议
            {
				if (0 == m_VecPos[i].trackID){
					return;
				}

				if (m_mapTrackingIdPos.find(m_VecPos[i].trackID) != m_mapTrackingIdPos.end()) {
					if ((0 == m_VecPos[i].y) && m_VecPos[i].x > 0) //只有X坐标，Y没有变化
					{
						if (m_mapTrackingIdPos.end() != m_mapTrackingIdPos.find(m_VecPos[i].trackID))
							m_VecPos[i].y = m_mapTrackingIdPos[m_VecPos[i].trackID].back().y;
						else
							continue;
					}
					else if ((0 == m_VecPos[i].x) && m_VecPos[i].y > 0) //只有Y坐标，X没有变化
					{
						if (m_mapTrackingIdPos.end() != m_mapTrackingIdPos.find(m_VecPos[i].trackID))
							m_VecPos[i].x = m_mapTrackingIdPos[m_VecPos[i].trackID].back().x;
						else
							continue;
					}

                    auto itrFinder = std::find_if(m_mapTrackingIdPos[m_VecPos[i].trackID].begin(), m_mapTrackingIdPos[m_VecPos[i].trackID].end(), [&](const auto& val) {
                        return (m_VecPos[i].x == val.x) && (m_VecPos[i].y == val.y);
                    });

					// touch x & y position not exist, add it
					if (itrFinder == m_mapTrackingIdPos[m_VecPos[i].trackID].end()){
                        m_mapTrackingIdPos[m_VecPos[i].trackID].push_back(m_VecPos[i]);
						std::cout << "ADD ID : " << m_VecPos[i].trackID << "\nX : " << m_VecPos[i].x << "\nY : " << m_VecPos[i].y << std::endl;
					}
				}
                else {
					m_mapTrackingIdPos[m_VecPos[i].trackID].push_back(m_VecPos[i]);
                }
            }
        }
        
    }		

	std::cout << "m_mapTrackingIdPos size = " << m_mapTrackingIdPos.size() << std::endl;
}
// Process touch data from adb recv
void CCommonModeNoiseRejectionDlg::pfnAnalysisDataCallbackFunc( const AnalysisDataOutputFormat & data, void * userData )
{
	CCommonModeNoiseRejectionDlg * pThis = static_cast<CCommonModeNoiseRejectionDlg *>(userData);
	if ( !pThis ) return;

	static unsigned int nTrackingID = 0;
	static unsigned int nSlotID = 0;
	static unsigned int nTimeStamp = 0;
	static bool bIsBProtocol = false;
	static bool bIsDownPosIsTheSame = false;//如果坐标和上次一样,不会报坐标,那么在Down后使用上一次的坐标.																																
	std::map< int, std::vector< PositionInfo > > mapTrackingIdPosBack;

	/**
	 * In this Main thread, this function will always called whenever new analyzed Event coming.
	 * so we will operator m_mapTrackingIdPos / m_VecBtnStatus ...etc
	 * 
	 * In thread Save CMNR report,we will judge result according to m_mapTrackingIdPos / m_VecBtnStatus ...etc. 
	 * if not lock these resources, this will be dangerous!
	 */
	switch (data.nDataType)
	{
	     case DATA_TYPE_TRACKING_ID:
		 {
			 std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			 if (data.nDataValue == 0){
				 break;
			 }
			 pThis->m_VecPos[nSlotID].trackID = data.nDataValue;
			 if ((0xffffffff == data.nDataValue) && !bIsBProtocol) {
				 //有0xffffffff上来表示是B协议的
				 bIsBProtocol = true;
				 mapTrackingIdPosBack = pThis->m_mapTrackingIdPos;
				 pThis->m_mapTrackingIdPos.clear();
				 for (auto it = mapTrackingIdPosBack.begin(); it != mapTrackingIdPosBack.end(); it++)
				 {
					 for (unsigned int i = 0; i < it->second.size(); i++)
						 pThis->m_mapTrackingIdPos[0].push_back(it->second[i]);
				 }
				 break;
			 }
			 if (data.nDataValue > 10)
			 {
				 bIsBProtocol = true;
			 }
			 bIsDownPosIsTheSame = false;					
			break;
		 }
		case DATA_TYPE_TOUCH:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			if (TOUCH_VALUE_UP == data.nDataValue)
				pThis->m_VecBtnStatus.push_back(1);
			else if (TOUCH_VALUE_DOWN == data.nDataValue)
				bIsDownPosIsTheSame = true;			   
			break;
		}
		case DATA_TYPE_X_POS:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			pThis->m_VecPos[nSlotID].x = data.nDataValue;
			bIsDownPosIsTheSame = false;				   
			break;
		}
		case DATA_TYPE_Y_POS:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			pThis->m_VecPos[nSlotID].y = data.nDataValue;
			bIsDownPosIsTheSame = false;				   
		}
			break;
        case DATA_TYPE_SLOT:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			nSlotID = data.nDataValue;
			if (nSlotID > 0 && !bIsBProtocol) {
				mapTrackingIdPosBack = pThis->m_mapTrackingIdPos;
				pThis->m_mapTrackingIdPos.clear();
				for (auto it = mapTrackingIdPosBack.begin(); it != mapTrackingIdPosBack.end(); it++)
				{
					for (unsigned int i = 0; i < it->second.size(); i++)
						pThis->m_mapTrackingIdPos[0].push_back(it->second[i]);
				}
			}
			bIsBProtocol = true;
		}
            break;
        case DATA_TYPE_TIME_STAMP:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			nTimeStamp = data.nDataValue;
		}
            break;
		case DATA_TYPE_MT_REPORT:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			for (unsigned int i = 0; i < pThis->m_VecPos.size(); i++)
			{
				if ((0 == pThis->m_VecPos[i].x) && (0 == pThis->m_VecPos[i].y))
					continue;

				if (pThis->m_VecPos[i].trackID == 0)
					continue;

				
				pThis->m_VecPos[i].timeTouch = (unsigned int)GetTickCount64();
				if (pThis->m_mapTrackingIdPos.empty())
				{
					pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID].push_back(pThis->m_VecPos[i]);
				}
				else
				{
					//A协议
					if (pThis->m_mapTrackingIdPos.end() == pThis->m_mapTrackingIdPos.find(pThis->m_VecPos[i].trackID)) {
						pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID] = std::vector< PositionInfo >{ pThis->m_VecPos[i] };
					}
					else {
						auto itrFinder = std::find_if(pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID].begin(), pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID].end(), [&](const auto& val) {
							return (pThis->m_VecPos[i].x == val.x) && (pThis->m_VecPos[i].y == val.y);
							});
						// touch x & y position not exist, add it
						if (itrFinder == pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID].end()) {
							pThis->m_mapTrackingIdPos[pThis->m_VecPos[i].trackID].push_back(pThis->m_VecPos[i]);
						}
					}
				}
				
			}
			//pThis->m_muPosOperation.unlock();						   
			//清空m_VecPos
			pThis->m_VecPos.clear();
			pThis->m_VecPos.resize(SUPPORT_TOUCH_MAX_NUM);
		}
			break;
		case DATA_TYPE_REPORT:
		{
			std::lock_guard< std::mutex > locker(pThis->GetMuPos());
			//B协议
			if (0xffffffff == pThis->m_VecPos[nSlotID].trackID) {
				TRACE(_T("B protocol report only, no x, y\n"));
				break;
			}

			if (0 == pThis->m_VecPos[nSlotID].trackID) {
				break;
			}

			for (unsigned int i = 0; i < pThis->m_VecPos.size(); i++)
			{
				if (pThis->m_VecPos[i].x > 0 || pThis->m_VecPos[i].y > 0) {
					bIsDownPosIsTheSame = false;
					break;
				}
			}
			if (bIsDownPosIsTheSame) {
				//如果按指了，但是坐标跟上一次的一样，一直没有报点上来，要把上一次的坐标加进来。
				if (pThis->m_VecBackLastPos.size() > 0 && pThis->m_VecBackLastPos.back().trackID != 0)
				{
					pThis->m_VecPos[0] = pThis->m_VecBackLastPos.back();
				}
			}
			pThis->ParseData(nTimeStamp, bIsBProtocol);

			//清空m_VecPos
			pThis->m_VecPos.clear();
			pThis->m_VecPos.resize(SUPPORT_TOUCH_MAX_NUM);
		}
			break;
		default:
			ASSERT( FALSE );
	}
}

void pfnAnalysisErrorCallbackFunc( LPCTSTR strErrorMessage, void * userData )
{
}

BOOL CCommonModeNoiseRejectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//UpdateUIVersion();
	LoadIniFile();
	InitUI_Linear();
#ifdef EANBLE_TEK_VISA
#endif	
	InitUI_FuncGen();

#ifdef LINEAR_ENABLE
	if ( !Init_CtrlCard() )
	{
		return FALSE;
	}
#endif
	InitUI_LinearTest();

#ifdef LINEAR_ENABLE
	//g_CtrlCard.Setup_Speed(1, m_Speed, m_Speed, 500, 500, 5, 2);
	//g_CtrlCard.Setup_Speed(2, m_Speed, m_Speed, 500, 500, 5, 2);
	//g_CtrlCard.Setup_Speed(3, m_Speed, m_Speed, 500, 500, 5, 2);
/*	if ( !SystemBackHome() )
	{
		AfxMessageBox( _T("Back to home failed!"), MB_OK );
		return FALSE;
	}*/

#endif

	SetTimer( CMNR_TIMER_LINEAR_AXIS_LIMIT, 100, NULL );

	//InitToolBar();

	InitUI_NormalTest();

	((CMFCButton*)GetDlgItem(IDC_MFCBUTTON_RUN))->EnableWindow(FALSE);

	//IDC_CHECK_DEBUG
	GetDlgItem(IDC_CHECK_DEBUG)->ShowWindow(SW_HIDE);
	//IDC_STATIC_EDGE
	GetDlgItem(IDC_STATIC_EDGE)->ShowWindow(SW_HIDE);
	//IDC_EDIT_EDGE
	GetDlgItem(IDC_EDIT_EDGE)->ShowWindow(SW_HIDE);

	m_PtCloudWin = new CPointCloud((float)m_ResolutionX, (float)m_ResolutionY, this);
	m_PtCloudWin->Create(IDD_DIALOG_POINT_CANVAS);

	m_PtCloudWin->ShowWindow(SW_HIDE);
	m_PtCloudWin->SetDebugMode(FALSE);

	CString strCoor;
	strCoor.Format(_T("%.3f"), m_CornerPts[0].first);
	SetDlgItemText(IDC_EDIT_X0, strCoor);
	strCoor.Format(_T("%.3f"), m_CornerPts[0].second);
	SetDlgItemText(IDC_EDIT_Y0, strCoor);

	strCoor.Format(_T("%.3f"), m_CornerPts[1].first);
	SetDlgItemText(IDC_EDIT_X1, strCoor);
	strCoor.Format(_T("%.3f"), m_CornerPts[1].second);
	SetDlgItemText(IDC_EDIT_Y1, strCoor);

	strCoor.Format(_T("%.3f"), m_CornerPts[2].first);
	SetDlgItemText(IDC_EDIT_X2, strCoor);
	strCoor.Format(_T("%.3f"), m_CornerPts[2].second);
	SetDlgItemText(IDC_EDIT_Y2, strCoor);

	strCoor.Format(_T("%.3f"), m_CornerPts[3].first);
	SetDlgItemText(IDC_EDIT_X3, strCoor);
	strCoor.Format(_T("%.3f"), m_CornerPts[3].second);
	SetDlgItemText(IDC_EDIT_Y3, strCoor);

	strCoor.Format(_T("%.3f"), m_CornerPts[3].first);
	SetDlgItemText(IDC_EDIT_X4, strCoor);
	strCoor.Format(_T("%.3f"), m_CornerPts[3].second);
	SetDlgItemText(IDC_EDIT_Y4, strCoor);

	SetDlgItemText(IDC_EDIT_EDGE, _T("8"));
	SetDlgItemText(IDC_EDIT_INNER_SIZE, _T("5"));

	strCoor.Format(_T("%d"), m_ResolutionX);
	SetDlgItemText(IDC_EDIT_RESX, strCoor);
	strCoor.Format(_T("%d"), m_ResolutionY);
	SetDlgItemText(IDC_EDIT_RESY, strCoor);

	SetDlgItemText(IDC_EDIT_CNTX, _T("50"));
	SetDlgItemText(IDC_EDIT_CNTY, _T("2"));

	CComboBox* pC = (CComboBox*)GetDlgItem(IDC_COMBO_COORD_ZERO);
	if (pC)
	{
		pC->AddString(_T("左上角"));
		pC->AddString(_T("右上角"));
		pC->AddString(_T("左下角"));
		pC->AddString(_T("右下角"));
		pC->SetCurSel(1);
	}

	return TRUE;
}

void CCommonModeNoiseRejectionDlg::InitUI_NormalTest()
{
	((CButton*)GetDlgItem(IDC_RADIO_NORMAL_CMNR_TEST))->SetCheck(BST_CHECKED);

	//IDC_COMBO_SELECT
	CComboBox* pCom = (CComboBox*)GetDlgItem(IDC_COMBO_SELECT);
	pCom->AddString(_T("默认"));
	pCom->AddString(_T("ADB 断线续测"));
	pCom->SetCurSel(0);
	pCom->ShowWindow(SW_HIDE);

	//IDC_BUTTON_LOAD_POINT_FILE
	GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_HIDE);
	//IDC_STATIC_POINT_RESTART
	GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_HIDE);
	//IDC_EDIT_START
	GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_HIDE);

	// IDC_CHECK_ACCURATE
	GetDlgItem(IDC_CHECK_ACCURATE)->ShowWindow(SW_HIDE);

	SetDlgItemText(IDC_EDIT_OFFSET_X, _T("4.298"));
	SetDlgItemText(IDC_EDIT_OFFSET_Y, _T("50.027"));
}

void CCommonModeNoiseRejectionDlg::InitUI_LinearTest()
{
	InitLinearTestListCtl();

	CRect rect;
	m_PathTypeList.GetWindowRect(rect);
	ScreenToClient(rect);
	int nLeft = rect.left;
	int nRight = nLeft + rect.Width();
	int nTop = rect.bottom + 5;
	int nBottom = nTop + rect.Width();

	mDrawPathArea = CRect( nLeft, nTop, nRight, nBottom );

	m_SpeedSlide.SetRange( 1, 100 );
	m_SpeedSlide.SetPos(50);
	m_Speed = m_SpeedSlide.GetPos() * 1000;

	m_LinearTProbar.SetRange(0, 100);
	m_LinearTProbar.SetPos(0);
	m_LinearTProbar.ShowWindow(SW_HIDE);

	m_ComboTpType.AddString(_T("Rectangle"));
	m_ComboTpType.AddString(_T("Circle"));
	m_ComboTpType.SetCurSel(0);

	((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->SetWindowText(_T("80"));
	((CEdit*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->SetWindowText(_T("80"));

	CString strRange;
	strRange.Format( _T("%.1f"),m_nRange );
	SetDlgItemText( IDC_EDIT_RANGE, strRange );

	SetValidLaps(80);

	CString strMax;
	GetDlgItemText( IDC_STATIC_MAX, strMax );
	strMax = strMax.Right(strMax.GetLength() - 2 );
	((CStatic*)GetDlgItem(IDC_COMBO_LAPS))->SetWindowTextW(strMax);
}

BOOL CCommonModeNoiseRejectionDlg::Init_CtrlCard()
{
	int ret = g_CtrlCard.Init_Board();

	if (ret <= 0)
	{
		if (ret == 0)
		{
			MessageBox(_T("没有安装ADT8940A1卡!"));
			return FALSE;
		}
		else if (ret == -1)
		{
			MessageBox(_T("没有安装端口驱动程序!"));
			return FALSE;
		}
		else if (ret == -2)
		{
			MessageBox(_T("PCI桥故障!"));
			return FALSE;
		}

		MessageBox(_T("控制卡初始化失败!"));
		return FALSE;
	}

	g_CtrlCard.OnParamSet();
	return TRUE;
}

void CCommonModeNoiseRejectionDlg::UpdateUIVersion()
{
    char szModuleFileName[MAX_PATH]{ 0 };
    GetModuleFileNameA( NULL, szModuleFileName, MAX_PATH );

    std::string strFileVersion = HFST_DevToolbox::HFST_GetModuleVersion( szModuleFileName );

	CString strFileTitle = _T( "" );
	GetWindowText( strFileTitle );

	strFileTitle.AppendFormat( _T( " - %s" ), (LPTSTR)CA2T( strFileVersion.c_str() ) );
    SetWindowText( strFileTitle );
}

void CCommonModeNoiseRejectionDlg::InitUI_Linear()
{
    //按扭加载图标
    CImage img;
    LoadImageFromResource( &img, IDB_PNG_LEFT_X, _T( "PNG" ) );
    m_BtnLeftX.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_RIGHT_X, _T( "PNG" ) );
    m_BtnRightX.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_UP_Y, _T( "PNG" ) );
    m_BtnUpY.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_DOWN_Y, _T( "PNG" ) );
    m_BtnDownY.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_UP_Z, _T( "PNG" ) );
    m_BtnUpZ.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_DOWN_Z, _T( "PNG" ) );
    m_BtnDownZ.SetBitmap( img );
    LoadImageFromResource( &img, IDB_PNG_HOME, _T( "PNG" ) );
    m_BtnHome.SetBitmap( img );

	CString str;
    str.Format(_T("%0.2f"), m_CmnrCfg.fAxis_X);
    SetDlgItemText(IDC_EDIT_POS_X, str);

    str.Format(_T("%0.2f"), m_CmnrCfg.fAxis_Y);
    SetDlgItemText(IDC_EDIT_POS_Y, str);

	SetDlgItemInt(IDC_EDIT_STOP_TIME, m_CmnrCfg.iStopTime);

	str.Format(_T("%0.2f"), m_CmnrCfg.fSafeHigh);
	SetDlgItemText(IDC_EDIT_SAFE_HIGH, str);

	str.Format(_T("%0.2f"), m_CmnrCfg.fTouchHigh);
	SetDlgItemText(IDC_EDIT_TOUCH_HIGH, str);

    std::vector<PositionInfo> vecPos;
	vecPos.resize(1);

	m_mapTrackingIdPos[0] = vecPos;
    m_BtnRightX.EnableWindow( true );
    m_BtnLeftX.EnableWindow( true );
    m_BtnUpY.EnableWindow( true );
    m_BtnDownY.EnableWindow( true );
    m_BtnUpZ.EnableWindow( true );
    m_BtnDownZ.EnableWindow( true );
    m_BtnHome.EnableWindow( true );
}

void CCommonModeNoiseRejectionDlg::InitUI_FuncGen()
{
    m_ComboWaveForm.ResetContent();
    m_ComboWaveForm.InsertString( 0, _T( "Sine" ) );
    m_ComboWaveForm.InsertString( 1, _T( "Square" ) );
    m_ComboWaveForm.InsertString( 2, _T( "Pulse" ) );
    m_ComboWaveForm.SetCurSel( 0 );

    // Freq
	CString str;
	str.Format(_T("%d"), m_CmnrCfg.iMaxFreq);
    ((CEdit *)GetDlgItem( IDC_EDIT_MAX_FREQ ))->SetWindowTextW(str);
	str.Format(_T("%d"), m_CmnrCfg.iMinFreq);
    ((CEdit *)GetDlgItem( IDC_EDIT_MIN_FREQ ))->SetWindowTextW(str);
	str.Format(_T("%d"), m_CmnrCfg.iIncFreq);
    ((CEdit *)GetDlgItem( IDC_EDIT_INC_FREQ ))->SetWindowTextW(str);

    //Amplitude
	str.Format(_T("%0.2f"), m_CmnrCfg.fMaxAmpl);
    ((CEdit *)GetDlgItem( IDC_EDIT_MAX_AMP ))->SetWindowTextW(str);
	str.Format(_T("%0.2f"), m_CmnrCfg.fMinAmpl);
    ((CEdit *)GetDlgItem( IDC_EDIT_MIN_AMP ))->SetWindowTextW(str);
	str.Format(_T("%0.2f"), m_CmnrCfg.fAmpStep);
    ((CEdit *)GetDlgItem( IDC_EDIT_STEP_AMP ))->SetWindowTextW(str);

    //Duty Cycle
    m_SliderDuty.SetRange( 0, 100 );
    m_SliderDuty.SetTicFreq( 10 );
    m_SliderDuty.SetPos( 50 );
	str.Format(_T("%d%%"), m_CmnrCfg.iDutyCycle);
	((CEdit*)GetDlgItem(IDC_EDIT_DUTY))->SetWindowTextW(str);

    //Impedance
	str.Format(_T("%d"), m_CmnrCfg.iImped);
    ((CEdit *)GetDlgItem( IDC_EDIT_IMP ))->SetWindowTextW(str);

	//Round count
	((CEdit*)GetDlgItem(IDC_EDIT_ROUND_CNT))->SetWindowTextW(_T("1"));

	m_VecPos.resize(SUPPORT_TOUCH_MAX_NUM);
	m_dataAnalysis.Init(pfnAnalysisDataCallbackFunc, pfnAnalysisErrorCallbackFunc, this);
}

void CCommonModeNoiseRejectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CCommonModeNoiseRejectionDlg::OnPaint()
{
	CPaintDC dc(this);
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
        if (m_StopThread)
        {
            if (m_eTestResult == TestResult::PASS)
            {
                DrawResultUI(&dc, RGB(61, 145, 64), _T("PASS"));
            }
            else if (m_eTestResult == TestResult::NG)
            {
                DrawResultUI(&dc, RGB(255, 0, 0), _T("NG"));
            }
        }
		dc.FillSolidRect( mDrawPathArea, RGB( 180, 180, 180 ) );

		if ( m_PathTypeList )
		{
			/* draw tp border */
			switch ( m_ComboTpType.GetCurSel() )
			{
				case 0:/* Rectangle */
				{
					DrawRectTp();
					break;
				}
				case 1:/* Circle */
				{
					DrawCircleTp();
					break;
				}
			}

			/* draw spiral */
			int idx = GetCheckIndex();
			DrawSpiral(idx);

			/* draw points that have tested */
			for (auto [x, y] : m_BackPtVec)
			{
				DrawPoint( x, y );
			}

		}

		CDialogEx::OnPaint();
	}
}
void CCommonModeNoiseRejectionDlg::DrawRectTp()
{
	CString strSize;
	((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->GetWindowText(strSize);
	int nWidth = _ttoi(strSize) * ZOOM;

	((CEdit*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->GetWindowText(strSize);
	int nHeight = _ttoi(strSize) * ZOOM;

	int nCenterX = mDrawPathArea.left + (mDrawPathArea.right - mDrawPathArea.left) / 2;
	int nCenterY = mDrawPathArea.top + (mDrawPathArea.bottom - mDrawPathArea.top) / 2;

	CRect recTp( nCenterX - nWidth/2, nCenterY- nHeight/2, nCenterX + nWidth / 2, nCenterY + nHeight / 2 );

	CClientDC pDc(this);
	pDc.FillSolidRect( recTp , RGB( 0, 0, 0 ) );
}
void CCommonModeNoiseRejectionDlg::DrawCircleTp()
{
	CString strSize;
	((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->GetWindowText(strSize);
	int nWidth = _ttoi(strSize) * ZOOM;

	int nCenterX = mDrawPathArea.left + (mDrawPathArea.right - mDrawPathArea.left) / 2;
	int nCenterY = mDrawPathArea.top + (mDrawPathArea.bottom - mDrawPathArea.top) / 2;

	CDC* pDc = GetDC();
	Gdiplus::Graphics graphics( pDc->GetSafeHdc() );
	Gdiplus::Rect recTp( nCenterX - nWidth / 2, nCenterY - nWidth / 2, nWidth, nWidth );
	const Gdiplus::SolidBrush mBrush( Gdiplus::Color( 255, 0, 0, 0 ) );
	const Gdiplus::Pen mPen( Gdiplus::Color(255, 0, 0, 0 ), 1 );

	Gdiplus::Status status =  graphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
	status = graphics.DrawEllipse( &mPen, recTp );
	status = graphics.FillEllipse( &mBrush, recTp );
	ReleaseDC(pDc);
}

/* Find proper laps settings based on typical Tp limit size */
void CCommonModeNoiseRejectionDlg::SetValidLaps(int nLimit)
{
	m_LapCombo.ResetContent();

	float max = 0.0;
	for ( float i = 0.5f; i < 100.0f; i+=0.5f )
	{
		if ( IsOutOfTpRange(i, nLimit) )
		{
			max = i;
			break;
		}
	}

	CString strMax;
	strMax.Format( _T("~ %.1f"), max - 0.5 );
	((CStatic*)GetDlgItem(IDC_STATIC_MAX))->SetWindowTextW(strMax);

	CString strInput;
	GetDlgItemText( IDC_COMBO_LAPS, strInput );

	if ( _tcstof(strInput,NULL) > (max-0.5) )
	{
		SetDlgItemText( IDC_COMBO_LAPS, strMax.Right(strMax.GetLength()-2) );
	}
}

/* when set laps to nlap, see whether the total size will beyond the nLimit size */
BOOL CCommonModeNoiseRejectionDlg::IsOutOfTpRange(float nLap, int nLimit)
{
	int thetaMax = (int)nLap * 360;
	double max = 0.0;
	for ( int theta = 0; theta < thetaMax; ++theta )
	{
		double x = ((double)A + (double)B * theta / (180 / m_nRange)) * cos(theta * PI / 180);
		double y = ((double)A + (double)B * theta / (180 / m_nRange)) * sin(theta * PI / 180);

		double maxXY = max( x, y );
		if ( 2 * maxXY > max )
		{
			max = 2 * maxXY;
		}
	}

	if ((int)max + 1 >= nLimit)
	{
		return true;
	}
	return false;
}

void CCommonModeNoiseRejectionDlg::DrawSpiral(int index)
{
	CString strLap;
	//m_LapCombo.GetLBText( m_LapCombo.GetCurSel(), strLap );	
	m_LapCombo.GetWindowText(strLap);
	float LAP_NUM = (float)_ttof( strLap );

	CClientDC dc(this);

	CPen pen(PS_SOLID, 2, RGB(255, 255, 255));
	CBrush brush(RGB(255, 255, 255));

	int nCenterX = mDrawPathArea.left + (mDrawPathArea.right - mDrawPathArea.left) / 2;
	int nCenterY = mDrawPathArea.top + (mDrawPathArea.bottom - mDrawPathArea.top) / 2;

	CRect textRec( mDrawPathArea.left + 1, mDrawPathArea.top + 1, mDrawPathArea.left + 70, mDrawPathArea.top + 15 );
	CFont font;
	VERIFY(font.CreateFont(
		20,                       // nHeight
		0,                        // nWidth
		0,                        // nEscapement
		0,                        // nOrientation
		FW_BOLD,                  // nWeight
		FALSE,                    // bItalic
		FALSE,                    // bUnderline
		0,                        // cStrikeOut
		ANSI_CHARSET,             // nCharSet
		OUT_DEFAULT_PRECIS,       // nOutPrecision
		CLIP_DEFAULT_PRECIS,      // nClipPrecision
		DEFAULT_QUALITY,          // nQuality
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
		_T("微软雅黑")));         // lpszFacename

	CFont* def_font = dc.SelectObject(&font);
	dc.SetBkMode(TRANSPARENT);

	BOOL bClockWise = TRUE;
	dc.SetTextColor( RGB( 255, 0, 0 ) );

	switch (index)
	{
		case 0:
		{
			//< 顺时针，由内到外
			dc.DrawText( _T("由内到外"), textRec, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
			break;
		}
		case 1:
		{
			//< 顺时针，由外到内
			dc.DrawText( _T("由外到内"), textRec, DT_SINGLELINE | DT_CENTER | DT_VCENTER );	
			bClockWise = FALSE;
			break;
		}
		case 2:
		{
			//< 逆时针，由内到外
			dc.DrawText(_T("由内到外"), textRec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			bClockWise = FALSE;
			break;
		}
		case 3:
		{
			//< 逆时针，由外到内
			dc.DrawText(_T("由外到内"), textRec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			break;
		}
	}
	dc.SelectObject(&pen);
	dc.SelectObject(&brush);
	dc.SelectObject(def_font);
	font.DeleteObject();

	if ( bClockWise )
	{
		for (int theta = 0; theta < (int)(LAP_NUM * 360); ++theta)
		{
			//< calculate x/y pos
			double x = nCenterX + ZOOM * ( ((double)A + (double)B * (double)theta / (180 / m_nRange)) * cos(theta * PI / 180) );
			double y = nCenterY + ZOOM * ( ((double)A + (double)B * (double)theta / (180 / m_nRange)) * sin(theta * PI / 180) );

			//< draw point
			dc.MoveTo( (int)x, (int)y );
			dc.LineTo( (int)x, (int)y );
		}
	}
	else
	{
		for (int theta = 0; theta > -LAP_NUM * 360; --theta)
		{
			//< calculate x/y pos
			double x = nCenterX + ZOOM * ( ((double)A + (double)B * (double)abs(theta) / (180 / m_nRange)) * cos(theta * PI / 180) );
			double y = nCenterY + ZOOM * ( ((double)A + (double)B * (double)abs(theta) / (180 / m_nRange)) * sin(theta * PI / 180) );

			//< draw point
			dc.MoveTo( (int)x, (int)y );
			dc.LineTo( (int)x, (int)y );
		}
	}

}

void CCommonModeNoiseRejectionDlg::DrawResultUI(CDC* pDc, COLORREF color, CString result)
{
    CRect wRec;
    ((CButton*)GetDlgItem(IDC_BTN_CMNR_START))->GetWindowRect(wRec);
    ScreenToClient(wRec);

	int top = wRec.top + wRec.Height() + 5;
	wRec.bottom = wRec.bottom + wRec.Height() + 5;
	wRec.top = top;

    CFont mFont;
    VERIFY(mFont.CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Consolas")));
    CFont* defaultFont = pDc->SelectObject(&mFont);
    pDc->SetBkMode(TRANSPARENT);
    pDc->SetTextColor(color);

    pDc->DrawText(result, wRec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    pDc->SelectObject(defaultFont);
    mFont.DeleteObject();
}

HCURSOR CCommonModeNoiseRejectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Function Gen Operation
bool CCommonModeNoiseRejectionDlg::GetConfig() {
	CEdit* pEdit;
	CString text, msg;

	m_CmnrCfg.iWaveform = m_ComboWaveForm.GetCurSel();

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MAX_FREQ);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.iMaxFreq = _tcstol(text, NULL, 10);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MIN_FREQ);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.iMinFreq = _tcstol(text, NULL, 10);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_INC_FREQ);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.iIncFreq = _tcstol(text, NULL, 10);

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MAX_AMP);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.fMaxAmpl = (float)_tcstod(text, NULL);

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MIN_AMP);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.fMinAmpl = (float)_tcstod(text, NULL);

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STEP_AMP);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.fAmpStep = (float)_tcstod(text, NULL);

	m_CmnrCfg.iDutyCycle = m_SliderDuty.GetPos();

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMP);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.iImped = _tcstol(text, NULL, 10);

	//Test Round
	((CEdit*)GetDlgItem(IDC_EDIT_ROUND_CNT))->GetWindowTextW(text);
	m_CmnrCfg.iRound = _tcstol(text, NULL, 10);
	if (m_CmnrCfg.iRound <= 0) {
		msg.Format(_T("Test Round should NOT be zero"));
		AfxMessageBox(msg);
		return false;
	}

	//Test X
    pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POS_X);
    pEdit->GetWindowTextW(text);
    m_CmnrCfg.fAxis_X = (float)_tcstod(text, NULL);

    pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POS_Y);
    pEdit->GetWindowTextW(text);
    m_CmnrCfg.fAxis_Y = (float)_tcstod(text, NULL);

    pEdit = (CEdit*)GetDlgItem(IDC_EDIT_TOUCH_HIGH);
    pEdit->GetWindowTextW(text);
    m_CmnrCfg.fTouchHigh = (float)_tcstod(text, NULL);

    pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SAFE_HIGH);
    pEdit->GetWindowTextW(text);
    m_CmnrCfg.fSafeHigh = (float)_tcstod(text, NULL);

	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STOP_TIME);
	pEdit->GetWindowTextW(text);
	m_CmnrCfg.iStopTime = _tcstol(text, NULL, 10);
	//check values
	if (m_CmnrCfg.iMaxFreq <= m_CmnrCfg.iMinFreq) {
		msg.Format(_T("Max frequency is less than min."));
		AfxMessageBox(msg);
		return false;
	}

	if (m_CmnrCfg.fAmpStep <= 0) {
		msg.Format(_T("Increment should NOT be zero."));
		AfxMessageBox(msg);
		return false;
	}

	if (m_CmnrCfg.iIncFreq <= 0) {
		msg.Format(_T("Increment should NOT be zero."));
		AfxMessageBox(msg);
		return false;
	}

	if (m_CmnrCfg.fMaxAmpl <= 0 || m_CmnrCfg.fMinAmpl <= 0) {
		msg.Format(_T("Invalid Amplitude."));
		AfxMessageBox(msg);
		return false;
	}

	if (m_CmnrCfg.iStopTime <= 0) {
		msg.Format(_T("Interval should NOT be zero."));
		AfxMessageBox(msg);
		return false;
	}

	SaveCmnrIni();
	return true;
}

void GetRemainTime(CTimeSpan remainTime, CString& timeStr) {
	if (remainTime.GetDays()) {
		//timeStrTmp.Format(_T("About %d day(s)"));
		if (remainTime.GetHours() > 12) {
			timeStr.AppendFormat(_T("About %d day(s)"), remainTime.GetDays() + 1);
		}
		else {
			timeStr.AppendFormat(_T("About %d day(s)"), remainTime.GetDays());
		}
	}
	if (remainTime.GetHours()) {
		//timeStrTmp.Format(_T("About %d hour(s)"));
		if (remainTime.GetMinutes() > 30) {
			timeStr.AppendFormat(_T("about %d hour(s)"), remainTime.GetHours() + 1);
		}
		else {
			timeStr.AppendFormat(_T("about %d hour(s)"), remainTime.GetHours());
		}
	}
	else if (remainTime.GetMinutes()) {
		//timeStrTmp.Format(_T("About %d minute(s)"));
		if (remainTime.GetSeconds() > 30) {
			timeStr.AppendFormat(_T("about %d minute(s)"), remainTime.GetMinutes() + 1);
		}
		else {
			timeStr.AppendFormat(_T("about %d minute(s)"), remainTime.GetMinutes());
		}
	}
	else {
		//timeStrTmp.Format(_T("About %d second(s)"));
		timeStr.AppendFormat(_T("about %d second(s)"), remainTime.GetSeconds());
	}
}

//adb callback data function
void CCommonModeNoiseRejectionDlg::fnAdbOutputCallback(const char* data, int size, void* LParam)
{
	CCommonModeNoiseRejectionDlg* pThis = (CCommonModeNoiseRejectionDlg*)LParam;
	if (!data && 0 == size) {
		TRACE(_T("=========== ERROR ============\n"));
		return;
	}

	if (data && strstr(data, "device") != NULL){
		return;
	}

	std::cout << "==========  data coming......" << std::endl;
	pThis->m_dataAnalysis.PushData( data, size );
	std::cout << data << std::endl;
}

int CCommonModeNoiseRejectionDlg::StartAdbConnect() {

	int res = m_adb.RunExe("./platform-tools/adb.exe root", true);
	if (res <= 0) return -1;

	res = m_adb.RunExe("./platform-tools/adb.exe shell", false, fnAdbOutputCallback, this);
	if (res <= 0) return -2;

	res = m_adb.RunCmd("getevent -rl");

	return 0;
}

UINT CCommonModeNoiseRejectionDlg::ThreadDoCMNRTest(LPVOID LParam)
{
	CCommonModeNoiseRejectionDlg * pThis = (CCommonModeNoiseRejectionDlg*)LParam;
	return pThis->DoCMNRTest();
}

void CCommonModeNoiseRejectionDlg::MoveXYZ(long x, long y, long z)
{
	g_CtrlCard.Setup_Speed( 1, 25000, m_Speed, 20000, 20000, 5, 2 );
	g_CtrlCard.Setup_Speed( 2, 25000, m_Speed, 20000, 20000, 5, 2 );
	g_CtrlCard.Setup_Speed( 3, 25000, m_Speed, 20000, 20000, 5, 2 );

	g_CtrlCard.Axis_Pmove( AXIS_X, x );
	g_CtrlCard.Axis_Pmove( AXIS_Y, y );
	g_CtrlCard.Axis_Pmove( AXIS_Z, z );
	int nStatusX = -1;
	int nStatusY = -1;
	int nStatusZ = -1;
	while (1)
	{
		g_CtrlCard.Get_Status( AXIS_X, nStatusX, 0 );
		g_CtrlCard.Get_Status( AXIS_Y, nStatusY, 0 );
		g_CtrlCard.Get_Status( AXIS_Z, nStatusZ, 0 );

		int nSpeedX = g_CtrlCard.GetSpeed( 0, AXIS_X );
		int nSpeedY = g_CtrlCard.GetSpeed( 0, AXIS_Y );
		int nSpeedZ = g_CtrlCard.GetSpeed( 0, AXIS_Z );

		if (nStatusX == 0 && nStatusY == 0 && nStatusZ == 0 && nSpeedX == 0 && nSpeedY == 0 && nSpeedZ == 0)
		{
			break;
		}
	}
}

void CCommonModeNoiseRejectionDlg::GenerateTestPoints(std::vector<secPointVec>& PointVec, std::fstream& ofs)
{
	if (m_bDebugMode)	// 调试模式，产出均匀的测试点
	{
		float nGapX = m_TpWidth / (float(m_CountX - 1));
		float nGapY = m_TpHeight / (float(m_CountY - 1));

		int nPointIdx = 0;
		ofs << "//----------------------------------------------------------------\n";
		ofs << "//--  Point " << 0 * 100 << " - " << 0 * 100 + 99 << "\n";
		ofs << "//----------------------------------------------------------------\n";

		secPointVec sVec;
		for (int i = 0; i < m_CountX; ++i)
		{
			for (int j = 0; j < m_CountY; ++j)
			{
				float x = float(i) * nGapX;
				float y = float(j) * nGapY;
				if (y >= m_TpHeight)
				{
					y = m_TpHeight - 1;
				}

				if (x >= m_TpWidth)
				{
					x = m_TpWidth - 1;
				}

				x += m_InnerSize;
				y += m_InnerSize;

				long pulseX = long(x / ACCX);
				long pulseY = long(y / ACCY);

				sVec.emplace_back( pulseX, pulseY, 0 );

				ofs << "Point-" << nPointIdx << "\n";
				ofs << "X-tagPt" << nPointIdx << "(mm)\t" << pulseX * ACCX << "\n";
				ofs << "Y-tagPt" << nPointIdx << "(mm)\t" << pulseY * ACCY << "\n" << std::endl;

				nPointIdx++;
			}
		}
		PointVec.push_back(sVec);
	}
	else
	{
		unsigned int nStep = static_cast<unsigned int>(round(m_TpHeight / m_SecNumber));
		unsigned int nSecWidth = m_TpWidth;
		unsigned int nSecHeight = nStep;

		std::default_random_engine e(time(0));
		unsigned int nStartHeight = 0;
		int secCnt = 0;
		int nPointIdx = 0;
		while (secCnt < m_SecNumber)
		{
			std::uniform_int_distribution<unsigned> u_width(1, (unsigned int)m_TpWidth);
			std::uniform_int_distribution<unsigned> u_Hight(nStartHeight + 1, nSecHeight);

			ofs << "//----------------------------------------------------------------\n";
			ofs << "//--  Point " << secCnt * 100 << " - " << secCnt * 100 + 99 << "\n";
			ofs << "//----------------------------------------------------------------\n";

			secPointVec sVec;
			for (int i = 0; i < m_CountX; ++i)
			{
				for (int j = 0; j < m_CountY; ++j)
				{
					unsigned int x = u_width(e);
					unsigned int y = u_Hight(e);
					if (y >= m_TpHeight)
					{
						y = m_TpHeight - 1;
					}

					if (x >= m_TpWidth)
					{
						x = m_TpWidth - 1;
					}

					x += m_InnerSize;
					y += m_InnerSize;

					long pulseX = long(x / ACCX);
					long pulseY = long(y / ACCY);

					sVec.emplace_back( pulseX, pulseY, 0 );

					ofs << "Point-" << nPointIdx << "\n";
					ofs << "X-tagPt" << nPointIdx << "(mm)\t" << pulseX * ACCX << "\n";
					ofs << "Y-tagPt" << nPointIdx << "(mm)\t" << pulseY * ACCY << "\n" << std::endl;

					nPointIdx++;
				}
			}
			PointVec.push_back(sVec);

			nStartHeight += nStep;
			nSecHeight += nStep;

			secCnt++;
		}
	}
}

std::pair<CPoint, float> CCommonModeNoiseRejectionDlg::GetNearestPtAndDistance(
	const std::vector<CPoint>& info,	// 所有的ADB报点坐标
	const CPoint& device				// 计算出的报点坐标
)
{
	float distance{ 9999.0f };
	CPoint reportPt;
	for (const auto& pt : info)
	{
		float dis = Distance(pt, device);
		if (dis < distance)
		{
			distance = dis;
			reportPt = pt;
		}
	}

	return{ reportPt , distance };
}

UINT CCommonModeNoiseRejectionDlg::DoCMNRTest()
{
	int iRet = 0;
	char szModuleFileName[MAX_PATH]{ 0 };
    CString StrTemp;
	int currentRunCount = 0;
	int maxRunCount = 1;
	LONGLONG remainSecond;
	CString tmpStr, timeStr, timeStrTmp, tStamp, fileName;
	int progRange = 90;
	int progPos = 0;
	bool fgAdbConnect = false;
	GetDlgItemText(IDC_EDIT_TOUCH_HIGH, tmpStr);
	double dTouchHigh = _tcstod( tmpStr, NULL );
    GetDlgItemText(IDC_EDIT_SAFE_HIGH, tmpStr);
	double dSafeHigh = _tcstod( tmpStr, NULL );
	if ( dTouchHigh < dSafeHigh)
	{
		AfxMessageBox( _T( "TouchHigh need larger than safeHigh" ) );
		return 1;
	}
	long lTouchPulse = static_cast<long>(dTouchHigh * 1000);
	long lZbackPulse = static_cast<long>(dSafeHigh * 1000);
#ifdef MODE_NOISE
	// 所以 Freq 搭配 Amp 组合数都跑一次
	std::vector< std::pair<int, float > > vecFreqAmpScanGroup;
	for (int freq = m_CmnrCfg.iMinFreq; freq <= m_CmnrCfg.iMaxFreq; freq = freq + m_CmnrCfg.iIncFreq)
	{
		for (float amp = m_CmnrCfg.fMinAmpl; amp <= m_CmnrCfg.fMaxAmpl; amp = amp + m_CmnrCfg.fAmpStep)
		{
			vecFreqAmpScanGroup.push_back( { freq, amp } );
		}
	}

	maxRunCount = m_CmnrCfg.iRound;
#endif
	m_StopThread = false;

	if ( maxRunCount <= 0 ) { maxRunCount = 1;}

#ifdef MODE_NOISE
	m_iTestFreq = m_CmnrCfg.iMinFreq;
	m_fTestAmpl = m_CmnrCfg.fMinAmpl;
    //Directory
	GetModuleFileNameA(NULL, szModuleFileName, MAX_PATH);
    StrTemp = szModuleFileName;
	fileName = StrTemp.Left(StrTemp.ReverseFind('\\'));
#endif

#ifdef EANBLE_TEK_VISA
	iRet = m_TekVisa.TekStart();
	if ( iRet < 0 ) {
		TRACE( "TekStart failed." );
		AfxMessageBox( _T( "TekStart failed." ) );
		m_StaticProMsg.SetWindowText(_T("TekStart failed!"));
		return -1;
	}
#endif
	m_StaticProMsg.ShowWindow(SW_SHOWNORMAL);
	m_FgProgress.ShowWindow(SW_SHOWNORMAL);
#ifdef MODE_NOISE
	m_FgProgress.SetRange(0, (short)(maxRunCount* vecFreqAmpScanGroup.size()));
#endif
	m_FgProgress.SetPos(0);

	EnableLinearOperation(false);
	m_BtnHome.EnableWindow(FALSE);
    int     retn[] = { -1, -1, -1 };
    char    Axis[] = { 'X', 'Y', 'Z' };
#ifdef LINEAR_ENABLE
 //   for (int i = MAXAXIS - 1; i > 0; i--)
 //   {
 //       retn[i - 1] = g_CtrlCard.BackHome(i, m_StaticProMsg);
 //       if (retn[i - 1] < 0)
 //       {
 //           tmpStr.Format(_T("%c return Home fail, return value %d"), Axis[i - 1], retn[i - 1]);
 //           AfxMessageBox(tmpStr);
 //       }
 //   }
	//Interp_moveXY(); //X,Y移动到指定位置
#endif

	//将铜柱提起来
#ifdef LINEAR_ENABLE
	g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse); //提上来安全高度的距离
#endif

	//Connect adb
    iRet = StartAdbConnect();
    if (iRet < 0) {
        m_StaticProMsg.SetWindowText(_T("Adb connect failed!"));
        if (-1 == iRet)
            fgAdbConnect = false;
        else
            fgAdbConnect = true;
        //goto TEARDOWN_OUTPUT_WAV;
    }
    fgAdbConnect = true;

	m_BtnCMNRStart.SetWindowText( _T( "Stop" ) );
	Invalidate(); //清除Paint的矩形区域

    tStamp = CTime::GetCurrentTime().Format("%Y%m%d_%H%M%S");
	USES_CONVERSION;
	std::string sTimeStamp = CT2A(tStamp);
#ifdef MODE_NOISE
    fileName.AppendFormat(_T("\\CMNRResult\\CMNRTestResult_%s.txt"), tStamp);
    m_FileCMNRTestResult.Open(fileName, CFile::modeCreate | CFile::modeWrite);
	fileName = StrTemp.Left(StrTemp.ReverseFind('\\'));
	fileName.AppendFormat(_T("\\CMNRResult\\CMNRTestAdbReportInfo_%s.txt"), tStamp);
	m_dataAnalysis.mrCreateFile(fileName);						   
#endif

	long nLastX{0};
	long nLastY{0};

	if ( m_TestMode == TestMode::POINTS_1000_RANDOM )
	{
		std::filesystem::path cPath = std::filesystem::current_path() / "Point_Test_1000" / sTimeStamp;
		if (!std::filesystem::exists(cPath)) {
			std::filesystem::create_directories(cPath);
		}
		std::string filename = "POINT_TEST_1000_point_coordinate_detail.txt";
		std::filesystem::path fileDetail = cPath / filename;
		std::fstream ofs(fileDetail, std::ios::out);
		if (!ofs) {
			return -1;
		}

		// calculate tp_width & tp_height
		std::sort(m_CornerPts.begin(), m_CornerPts.end(), [&](const std::pair<float, float>& pt1, const std::pair<float, float>& pt2) {
			return pt1.first < pt2.first;
			});

		m_TpTotalWidth = m_CornerPts[3].first - m_CornerPts[0].first;
		m_TpTotalHeight = m_CornerPts[3].second - m_CornerPts[0].second;

		m_PtCloudWin->SetRadius( (float)m_ResolutionX / m_TpTotalWidth + 1 );
		m_PtCloudWin->ClearNG();
#ifdef LINEAR_ENABLE
		if ( m_bIsFirstTest )
		{
			// 第一次测试，先移到CCD所在的位置。之后，会移动到（0.0f, 0.0f）
			long pulseX = long( 4.298f / ACCX );
			long pulseY = long( 50.027f / ACCY );
			MoveXYZ( pulseX, pulseY, 0 );
		}
		// 移动到（0.0f, 0.0f）,以m_CurPt的坐标计
		long pX = long( (m_CornerPts[0].first - m_CurPt.first) / ACCX );
		long pY = long( (m_CornerPts[0].second - m_CurPt.second) / ACCY );
		MoveXYZ( pX, pY, 0 );
		// 移动新的offset差值
		pX = long((m_OffsetX_New - 4.298f) / ACCX);
		pY = long((m_OffsetY_New - 50.027f) / ACCY);
		MoveXYZ( pX, pY, 0 );
#endif
		m_TpWidth = m_TpTotalWidth - m_InnerSize * 2;
		m_TpHeight = m_TpTotalHeight - m_InnerSize * 2;

		std::vector<secPointVec> PointVec;
		std::vector<vPoint> FailPoint;
		// 生成待打点的坐标
		GenerateTestPoints( PointVec, ofs );
		ofs.flush();
		ofs.close();

		std::vector<CPoint> allPt;
		for (const auto& vec : PointVec)
		{
			for (const auto& pt : vec)
			{
				allPt.push_back({
					int( (float)pt.x * ACCX * (float)m_ResolutionX / m_TpTotalWidth ),
					int( pt.y * ACCY * (float)m_ResolutionY / m_TpTotalHeight )
					});
			}
		}
		m_PtCloudWin->SetAllPoint(allPt);

		// 排序
		for (auto& vec : PointVec)
		{
			std::sort(vec.begin(), vec.end(), [](const vPoint& p1, const vPoint& p2) {
				return p1.x < p2.x;
				});
		}

		filename = "POINT_TEST_1000_Round_1.txt";
		std::filesystem::path fileFail = cPath / filename;
		ofs.open(fileFail, std::ios::out);
		if (!ofs.is_open()) {
			return -1;
		}
		m_mapTrackingIdPos.clear();
		m_VecBtnStatus.clear();
		m_VecCmnrResult.clear();
		m_VecBackLastPos.clear();

		for (size_t i = 0;i < PointVec.size();++i)
		{
			const auto& pVec = PointVec.at(i);
			for (size_t j = 0; j < pVec.size(); ++j)
			{
				if ( m_StopThread )
					goto TEARDOWN_OUTPUT_WAV;

				const auto& point = pVec.at(j);

	#ifdef LINEAR_ENABLE
				MoveXYZ( point.x - nLastX, point.y - nLastY, 0 );
				//将铜柱放下
				iRet = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
				if (iRet > 0) {
					return -1;
				}
				g_CtrlCard.OnButtonPmove(AXIS_Z, lTouchPulse - g_log);

				Sleep(m_CmnrCfg.iStopTime);

				//将铜柱提起来安全高度的距离
				g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse);
				while (!m_dataAnalysis.IsQueueEmpty()) {
					Sleep(200);
				}
	#endif
				progPos++;
				m_FgProgress.SetPos(progPos);
				std::lock_guard< std::mutex > locker(m_muPosOperation);

				if ( m_mapTrackingIdPos.empty() && m_rErrorTypeSel.fgNoTouch ) {
					// No Touch
					LogFailPt(FailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), i* pVec.size() + j, _T("[漏报点]"));
				}
				else if ( m_mapTrackingIdPos.size() > 1 && m_rErrorTypeSel.fgMoreTouch ) {
					// More Touch
					LogFailPt(FailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), i* pVec.size() + j, _T("[多报点]"));
				}
				else
				{
					if (m_bAccuraceEnable)
					{
						std::vector< CPoint > info = GetAllPt();
						if (info.empty())
						{
							LogFailPt(FailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), i * pVec.size() + j, _T("[漏报点]"));
						}
						else
						{
							CPoint pointDevice(std::move(LogicToDevice(point)));

							auto [ reportPt, distance ] = GetNearestPtAndDistance( info, pointDevice );

							float nAccuraceThresHold = IsEdgeArea(point.x, point.y) ? m_BorderThres : m_NonBorderThres;

							if ( distance > ceil(float(m_ResolutionX) / m_TpTotalWidth) * nAccuraceThresHold )
								LogFailPt(FailPoint, ofs, point, pointDevice, reportPt, i * pVec.size() + j, _T("[不符合精准度要求]"));
							else
								LogFailPt(FailPoint, ofs, point, pointDevice, reportPt, i* pVec.size() + j, _T("[PASS]"));
						}
					}
				}

				nLastX = point.x;
				nLastY = point.y;

				for (unsigned int i = 0; i < m_mapTrackingIdPos.size(); i++)
				{
					if (m_mapTrackingIdPos[i].size() > 0)
						m_VecBackLastPos.push_back(m_mapTrackingIdPos[i].back());
				}

				m_mapTrackingIdPos.clear();
				m_VecBtnStatus.clear();
				m_VecCmnrResult.clear();
			}
		}
		ofs.flush();
		ofs.close();

		if ( m_bDebugMode )
			goto TEARDOWN_OUTPUT_WAV;

		m_PtCloudWin->ClearNG();
		filename = "POINT_TEST_1000_Round_2.txt";
		std::filesystem::path sFailFinal = cPath / filename;
		ofs.open(sFailFinal, std::ios::out);
		if ( !ofs.is_open() )
		{
			return -1;
		}
		std::vector<vPoint> finalFailPoint;
		for (size_t i = 0;i < FailPoint.size(); ++i)
		{
			if ( m_StopThread ) 
				goto TEARDOWN_OUTPUT_WAV;

			const auto& point = FailPoint.at(i);
	#ifdef LINEAR_ENABLE
			MoveXYZ( point.x - nLastX, point.y - nLastY, 0 );

			//将铜柱放下
			iRet = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
			if (iRet > 0) {
				return -1;
			}
			g_CtrlCard.OnButtonPmove(AXIS_Z, lTouchPulse - g_log);
			Sleep(m_CmnrCfg.iStopTime);

			progPos++;
			m_FgProgress.SetPos(progPos);

			//将铜柱提起来安全高度的距离
			g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse);
	#endif

			while (!m_dataAnalysis.IsQueueEmpty()) {
				Sleep(200);
			}

			std::lock_guard< std::mutex > locker(m_muPosOperation);

			if (m_mapTrackingIdPos.empty() && m_rErrorTypeSel.fgNoTouch)
			{
				// No Touch
				LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
			}
			else if (m_mapTrackingIdPos.size() > 1 && m_rErrorTypeSel.fgMoreTouch)
			{
				// More Touch
				LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[多报点]"));
			}
			else
			{
				if (m_bAccuraceEnable)
				{
					// 判断单指是否满足精准度要求
					std::vector< PositionInfo > info = (*m_mapTrackingIdPos.begin()).second;
					if (info.empty())
					{
						LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
					}
					else
					{
						std::vector< CPoint > info = GetAllPt();
						if (info.empty())
						{
							LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
						}
						else
						{
							CPoint pointDevice(std::move(LogicToDevice(point)));

							auto [reportPt, distance] = GetNearestPtAndDistance( info, pointDevice );

							float nAccuraceThresHold = IsEdgeArea(point.x, point.y) ? m_BorderThres : m_NonBorderThres;

							if ( distance > ceil(float(m_ResolutionX) / m_TpTotalWidth) * nAccuraceThresHold)
								LogFailPt(finalFailPoint, ofs, point, pointDevice, reportPt, point.idx, _T("[不符合精准度要求]"));
							else
								LogFailPt(finalFailPoint, ofs, point, pointDevice, reportPt, point.idx, _T("[PASS]"));
						}
					}
				}
			}

			nLastX = point.x;
			nLastY = point.y;

			for (unsigned int i = 0; i < m_mapTrackingIdPos.size(); i++)
			{
				if (m_mapTrackingIdPos[i].size() > 0)
					m_VecBackLastPos.push_back(m_mapTrackingIdPos[i].back());
			}

			m_mapTrackingIdPos.clear();
			m_VecBtnStatus.clear();
			m_VecCmnrResult.clear();
		}
		ofs.flush();
		ofs.close();
	}
	else if (m_TestMode == TestMode::POINTS_1000_RANDOM_CONTINUE)
	{
		mStartIndex = GetDlgItemInt(IDC_EDIT_START);

		std::vector<vPoint> failPoints;
		
		std::filesystem::path cPath = std::filesystem::current_path() / "Point_Test_1000" / "Retest" / sTimeStamp;
		if (!std::filesystem::exists(cPath))
		{
			std::filesystem::create_directories(cPath);
		}
		std::string filename = "Retest_Round_1.txt";
		std::filesystem::path fileFail = cPath / filename;
		std::fstream ofs(fileFail, std::ios::out);
		if ( !ofs )
		{
			return -1;
		}
		m_mapTrackingIdPos.clear();
		m_VecBtnStatus.clear();
		m_VecCmnrResult.clear();

		float nLastX = 0.0f;
		float nLastY = 0.0f;

		//将铜柱提起来安全高度的距离
	#ifdef LINEAR_ENABLE
		g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse);
	#endif
		for (size_t j = mStartIndex; j < mAllPoint.size(); ++j)
		{
			if (m_StopThread)
				goto TEARDOWN_OUTPUT_WAV;

			const auto& point = mAllPoint.at(j);

#ifdef LINEAR_ENABLE
			MoveXYZ( point.x - nLastX, point.y - nLastY, 0 );

			//将铜柱放下
			iRet = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
			if (iRet > 0) {
				return -1;
			}
			g_CtrlCard.OnButtonPmove(AXIS_Z, lTouchPulse - g_log);

			//去拿手机的坐标结果, 铜柱停留时间 
			Sleep(m_CmnrCfg.iStopTime);

			//将铜柱提起来安全高度的距离
			g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse);

			while (!m_dataAnalysis.IsQueueEmpty()) {
				Sleep(200);
			}
#endif
			progPos++;
			m_FgProgress.SetPos(progPos);
			std::lock_guard< std::mutex > locker(m_muPosOperation);
			if (m_mapTrackingIdPos.empty() && m_rErrorTypeSel.fgNoTouch)
			{
				// No Touch
				LogFailPt(failPoints, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
			}
			else if (m_mapTrackingIdPos.size() > 1 && m_rErrorTypeSel.fgMoreTouch)
			{
				// More Touch
				LogFailPt(failPoints, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[多报点]"));
			}
			else
			{
				if (m_bAccuraceEnable)
				{
					// 判断单指是否满足精准度要求
					std::vector< PositionInfo > info = (*m_mapTrackingIdPos.begin()).second;
					if (info.empty())
					{
						LogFailPt(failPoints, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
					}
					else
					{
						std::vector< CPoint > info = GetAllPt();
						CPoint pointDevice( std::move( LogicToDevice(point) ) );

						auto [reportPt, distance] = GetNearestPtAndDistance( info, pointDevice );

						float nAccuraceThresHold = IsEdgeArea(point.x, point.y) ? m_BorderThres : m_NonBorderThres;

						if (distance > ceil(float(m_ResolutionX) / m_TpTotalWidth) * nAccuraceThresHold)
							LogFailPt(failPoints, ofs, point, pointDevice, reportPt, point.idx, _T("[不符合精准度要求]"));
						else
							LogFailPt(failPoints, ofs, point, pointDevice, reportPt, point.idx, _T("[PASS]"));
					}
				}
			}

			nLastX = point.x;
			nLastY = point.y;

			for (unsigned int i = 0; i < m_mapTrackingIdPos.size(); i++)
			{
				if (m_mapTrackingIdPos[i].size() > 0)
					m_VecBackLastPos.push_back(m_mapTrackingIdPos[i].back());
			}

			m_mapTrackingIdPos.clear();
			m_VecBtnStatus.clear();
			m_VecCmnrResult.clear();
		}
		ofs.flush();
		ofs.close();

		filename = "Retest_Round_2.txt";
		std::filesystem::path sFailFinal = cPath / filename;
		ofs.open(sFailFinal, std::ios::out);
		if (!ofs.is_open()) {
			return -1;
		}
		std::vector<vPoint> finalFailPoint;
		// 测完之后， 继续测失败的点
		for (size_t i = 0; i < failPoints.size(); ++i)
		{
			if (m_StopThread)
				goto TEARDOWN_OUTPUT_WAV;

			const auto& point = failPoints.at(i);

	#ifdef LINEAR_ENABLE
			MoveXYZ( point.x - nLastX, point.y - nLastY, 0 );

			// 将铜柱放下
			iRet = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
			if (iRet > 0) {
				return -1;
			}
			g_CtrlCard.OnButtonPmove(AXIS_Z, lTouchPulse - g_log);
			// 去拿手机的坐标结果, 铜柱停留时间 
			Sleep( m_CmnrCfg.iStopTime );

			// 将铜柱提起来安全高度的距离
			g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse);
	#endif
			progPos++;
			m_FgProgress.SetPos(progPos);
			while (!m_dataAnalysis.IsQueueEmpty())
			{
				std::cout << "waiting for analysis data OK!" << std::endl;
				Sleep(200);
			}

			std::lock_guard< std::mutex > locker(m_muPosOperation);

			if (m_mapTrackingIdPos.empty() && m_rErrorTypeSel.fgNoTouch)
			{
				// No Touch
				LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
			}
			else if (m_mapTrackingIdPos.size() > 1 && m_rErrorTypeSel.fgMoreTouch)
			{
				// More Touch
				LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[多报点]"));
			}
			else
			{
				if (m_bAccuraceEnable)
				{
					// 判断单指是否满足精准度要求
					std::vector< PositionInfo > info = (*m_mapTrackingIdPos.begin()).second;
					if (info.empty())
					{
						LogFailPt(finalFailPoint, ofs, point, CPoint(0, 0), CPoint(0, 0), point.idx, _T("[漏报点]"));
					}
					else
					{
						std::vector< CPoint > info = GetAllPt();
						CPoint pointDevice( std::move(LogicToDevice(point)) );

						auto [reportPt, distance] = GetNearestPtAndDistance( info, pointDevice );
						float nAccuraceThresHold = IsEdgeArea(point.x, point.y) ? m_BorderThres : m_NonBorderThres;

						if ( distance > ceil(float(m_ResolutionX) / m_TpTotalWidth) * nAccuraceThresHold )
							LogFailPt(finalFailPoint, ofs, point, pointDevice, reportPt, point.idx, _T("[不符合精准度要求]"));
						else
							LogFailPt(finalFailPoint, ofs, point, pointDevice, reportPt, point.idx, _T("[PASS]"));
					}
				}
			}

			nLastX = point.x;
			nLastY = point.y;

			for (unsigned int i = 0; i < m_mapTrackingIdPos.size(); i++)
			{
				if (m_mapTrackingIdPos[i].size() > 0)
					m_VecBackLastPos.push_back(m_mapTrackingIdPos[i].back());
			}

			m_mapTrackingIdPos.clear();
			m_VecBtnStatus.clear();
			m_VecCmnrResult.clear();
		}
		ofs.flush();
		ofs.close();
	}

#ifdef MODE_NOISE
	do {
		shuffle( vecFreqAmpScanGroup.begin(), vecFreqAmpScanGroup.end(), std::default_random_engine( std::random_device{}() ) );

		for ( int idx = 0; idx < (int)vecFreqAmpScanGroup.size(); ++idx ) {
			m_iTestFreq = vecFreqAmpScanGroup[idx].first;	// ((rand() % (maxFreqEnd - minFreqStart + 1)) + minFreqStart)* pCfg->iIncFreq;
			m_fTestAmpl = vecFreqAmpScanGroup[idx].second;	// ((rand() % (maxAmpEnd - minAmpStart + 1)) + minAmpStart)* pCfg->fAmpStep;

			remainSecond = (m_CmnrCfg.iStopTime * (maxRunCount * vecFreqAmpScanGroup.size() - (idx + currentRunCount * vecFreqAmpScanGroup.size()))) / 1000;
			tmpStr.Format( _T( "[#%d] " ), currentRunCount + 1 );

			tmpStr.AppendFormat( _T( "Frequency %d Hz, Amplitude %.2fVpp (%d/%d)" ), m_iTestFreq, m_fTestAmpl, idx + (currentRunCount * vecFreqAmpScanGroup.size()) + 1, maxRunCount * vecFreqAmpScanGroup.size() );

			m_StaticProMsg.SetWindowTextW( tmpStr );
#ifdef EANBLE_TEK_VISA
			//Set TekVISA Waveform
			iRet = m_TekVisa.TekOutWaveForm( m_CmnrCfg.iWaveform, m_CmnrCfg.iDutyCycle, m_CmnrCfg.iImped, m_iTestFreq, m_fTestAmpl);
			if ( iRet < 0 ) {
				tmpStr.Format( _T( "Failed to operate TekVISA TekOutWaveForm" ) );
				AfxMessageBox( tmpStr );
				goto TEARDOWN_OUTPUT_WAV;
			}
#endif			
			m_dataAnalysis.mrSetFreqAmpl(m_iTestFreq, m_fTestAmpl);											  

#ifdef LINEAR_ENABLE
			//将铜柱放下
			//Todo
			iRet = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd); //拿当前Z轴的逻辑位置
			if (iRet > 0)
			{
                tmpStr.Format(_T("Get_CurrentInf fail"));
                AfxMessageBox(tmpStr);
				goto TEARDOWN_OUTPUT_WAV;
			}
			g_CtrlCard.OnButtonPmove(AXIS_Z, lTouchPulse - g_log);
#endif
			//去拿手机的坐标结果, 铜柱停留时间 
			Sleep(m_CmnrCfg.iStopTime);

			progPos++;
			m_FgProgress.SetPos( progPos );

			//将铜柱提起来
#ifdef LINEAR_ENABLE
			g_CtrlCard.OnButtonPmove(AXIS_Z, -lZbackPulse); //提上来安全高度的距离
#endif		
			SaveCMNRTestReport();

			if ( m_StopThread ) {
				break;
			}
		}
		currentRunCount++;
		TRACE( "[FunctionGen] Test #%d was completed.\n", currentRunCount);
	} while ( currentRunCount < maxRunCount );
	SaveCMNRTestResult();
#endif
    
TEARDOWN_OUTPUT_WAV:
    m_StopThread = true;
    m_FgProgress.SetPos(100);
    Invalidate();
    Sleep(TEKVISA_RAMP_TIME);
	AfxMessageBox( _T( "Test completed!" ) );
	m_FgProgress.ShowWindow( SW_HIDE );
#ifdef EANBLE_TEK_VISA
	//disable output
	iRet = m_TekVisa.TekEnd();
	if ( iRet < 0 ) {
		TRACE( "TekEnd was failed." );
		AfxMessageBox( _T( "Failed to operate TekVISA." ) );
	}
#endif

	//线性机台归零
#ifdef LINEAR_ENABLE
	long pX = 0 - nLastX + long((4.298f - m_OffsetX_New) / ACCX);
	long pY = 0 - nLastY + long((50.027f - m_OffsetY_New) / ACCY);
	MoveXYZ( pX, pY, 0 );
#endif

	CString strCur;
	strCur.Format(_T("%.3f"), m_CornerPts[0].first);
	SetDlgItemTextW(IDC_EDIT_X4, strCur);
	strCur.Format(_T("%.3f"), m_CornerPts[0].second);
	SetDlgItemTextW(IDC_EDIT_Y4, strCur);

	if (fgAdbConnect) {
		iRet = m_adb.RunCmd("exit", true);
		if (iRet < 0)
			m_StaticProMsg.SetWindowText(_T("Adb exit failed!"));
	}

	m_bIsFirstTest = FALSE;
	m_StaticProMsg.ShowWindow( SW_HIDE );
	m_StaticProMsg.SetWindowTextW( _T( "" ) );
	m_BtnCMNRStart.SetWindowText(_T("Start"));
    EnableLinearOperation(true);
    m_BtnHome.EnableWindow(true);
	return 0;
}

void CCommonModeNoiseRejectionDlg::SaveCMNRTestReport() {

    while (!m_dataAnalysis.IsQueueEmpty()) {
		std::cout << "waiting for analysis data OK!" << std::endl;
        Sleep(200);
    }

	std::cout << "=== CMNR Save Result : " << std::endl;

	for (const auto& val : m_mapTrackingIdPos)
	{
		std::cout << "ID : " << val.first << std::endl;
		for (size_t i = 0;i < val.second.size();++i)
		{
			std::cout << "idx" << i << " : " << std::endl;
			std::cout << "\t" << "posx : " << val.second.at(i).x << std::endl;
			std::cout << "\t" << "posy : " << val.second.at(i).y << std::endl;
			std::cout << "\t" << "track id : " << val.second.at(i).trackID << std::endl;
			std::cout << "\t" << "time : " << val.second.at(i).timeTouch << std::endl;
		}
	}

	CString strErrResult, tmpStr;
	FreqAmpl rFreqAmpl;
	ErrorType eErrType;
	bool  fgJitterError = false;
	std::map< int, bool > mapTrackJitter;
	std::lock_guard< std::mutex > locker(m_muPosOperation);
	if (m_mapTrackingIdPos.empty() && m_rErrorTypeSel.fgNoTouch)
	{
#ifdef MODE_NOISE
		strErrResult.Format(_T("[Error] No Touch : freq:%d, ampl:%0.1f\r\n"), m_iTestFreq, m_fTestAmpl);
		m_listResult.InsertString(0, strErrResult);
		TRACE(_T("Error(No Touch), freq:%d, ampl:%0.f\n"), m_iTestFreq, m_fTestAmpl);
		rFreqAmpl.iFreq = m_iTestFreq;
		rFreqAmpl.fAmpl = m_fTestAmpl;
		eErrType = ErrorType::ERROR_NO_TOUCH;
		m_VecCmnrResult.push_back({ rFreqAmpl, eErrType });

		//no touch 
        tmpStr.Format(_T("Noise level:(%d, %0.1f)\n"), m_iTestFreq, m_fTestAmpl);
        m_FileCMNRTestResult.WriteString(tmpStr);
        m_FileCMNRTestResult.Flush();

        tmpStr.Format(_T("Touches:0\n"));
        m_FileCMNRTestResult.WriteString(tmpStr);
        m_FileCMNRTestResult.Flush();

        tmpStr.Format(_T("Motions:0\n"));
        m_FileCMNRTestResult.WriteString(tmpStr);

        tmpStr.Format(_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));
        m_FileCMNRTestResult.WriteString(tmpStr);

        m_FileCMNRTestResult.Flush();
#endif
	}
	if (m_VecBtnStatus.size() > 1 && m_rErrorTypeSel.fgPenUp)
	{
		//UP标记出现1次以上，判提笔
		TRACE("Pen up %d\n", m_VecBtnStatus.size());
		strErrResult.Format(_T("[Error] Pen Up : %d, freq:%d, ampl:%0.1f\r\n"), m_VecBtnStatus.size() - 1, m_iTestFreq, m_fTestAmpl);
		m_listResult.InsertString(0, strErrResult);
		rFreqAmpl.iFreq = m_iTestFreq;
		rFreqAmpl.fAmpl = m_fTestAmpl;
		eErrType = ErrorType::ERROR_PEN_UP;
		m_VecCmnrResult.push_back({ rFreqAmpl, eErrType });
	}
	if (m_FileCMNRTestResult && m_mapTrackingIdPos.size() > 0)
	{
		bool fgTheSameTrackId = false;
		int motionCnt = 0;
		int i = 0;
		if (m_mapTrackingIdPos.size() > 1 && m_rErrorTypeSel.fgMoreTouch)
		{
			// track id position 大于1，则判more touch
			strErrResult.Format(_T("[Error] More touch : "), m_iTestFreq, m_fTestAmpl);
			for (auto it = m_mapTrackingIdPos.begin(); it != m_mapTrackingIdPos.end(); it++)
			{
				strErrResult.AppendFormat(_T("No.%d ID:%d; "), i, it->second[0].trackID);
				i++;
			}
			strErrResult.AppendFormat(_T("freq %d, ampl %0.1f"), m_iTestFreq, m_fTestAmpl);
			m_listResult.InsertString(0, strErrResult);
			rFreqAmpl.iFreq = m_iTestFreq;
			rFreqAmpl.fAmpl = m_fTestAmpl;
			eErrType = ErrorType::ERROR_MORE_TOUCH;
			m_VecCmnrResult.push_back({ rFreqAmpl, eErrType });
		}

		tmpStr.Format(_T("Noise level:(%d, %0.1f)\n"), m_iTestFreq, m_fTestAmpl);
		m_FileCMNRTestResult.WriteString(tmpStr);
		m_FileCMNRTestResult.Flush();

		tmpStr.Format(_T("Touches:%d\n"), m_mapTrackingIdPos.size());
		m_FileCMNRTestResult.WriteString(tmpStr);
		m_FileCMNRTestResult.Flush();
		for (auto it = m_mapTrackingIdPos.begin(); it != m_mapTrackingIdPos.end(); it++)
		{
			motionCnt += it->second.size();
		}
		tmpStr.Format(_T("Motions:%d\n"), motionCnt);
		m_FileCMNRTestResult.WriteString(tmpStr);
		m_FileCMNRTestResult.Flush();
		std::map< int, bool >::iterator itVecJitter;
		for (auto it = m_mapTrackingIdPos.begin(); it != m_mapTrackingIdPos.end(); it++) {
			for (unsigned int i = 0; i < it->second.size(); i++)
			{
				tmpStr.Format(_T("Position: (X:%4d, Y:%4d),  ID:%4d,  Time:%d\n"), it->second[i].x, it->second[i].y, it->second[i].trackID, it->second[i].timeTouch);
				m_FileCMNRTestResult.WriteString(tmpStr);
                
				if (m_rErrorTypeSel.fgJitter && (i + 1 < it->second.size()))
				{
					//Jitter打开，同一个ID，第二个及以后的pos都跟第一个进行比较，只记录一次。
                    itVecJitter = std::find_if(mapTrackJitter.begin(), mapTrackJitter.end(), [&](const auto& val) {
                        return (it->second[i].trackID == val.first);
                        });
					if (itVecJitter == mapTrackJitter.end()) {
						int x = it->second[i + 1].x - it->second[0].x;
						int y = it->second[i + 1].y - it->second[0].y;
						if ((abs(x) > m_rErrorTypeSel.uiJitterValue)
							|| (abs(y) > m_rErrorTypeSel.uiJitterValue))
						{
							mapTrackJitter[it->second[i].trackID] = true;
						}
					}
				}
			}
		}

		tmpStr.Format(_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));
		m_FileCMNRTestResult.WriteString(tmpStr);
		m_FileCMNRTestResult.Flush();
	}
	int count = 0;
	for (auto it = mapTrackJitter.begin(); it != mapTrackJitter.end(); it++)
	{
        TRACE("Jitter %d\n", m_rErrorTypeSel.uiJitterValue);
        strErrResult.Format(_T("[Error] Jitter : TrackId:%d, Jitter > %d, freq:%d, ampl:%0.1f\r\n"), it->first, m_rErrorTypeSel.uiJitterValue, m_iTestFreq, m_fTestAmpl);
        m_listResult.InsertString(0, strErrResult);
		if (0 == count) {
			rFreqAmpl.iFreq = m_iTestFreq;
			rFreqAmpl.fAmpl = m_fTestAmpl;
			eErrType = ErrorType::ERROR_JITTER;
			m_VecCmnrResult.push_back({ rFreqAmpl, eErrType });
		}
		count++;
	}
	for (unsigned int i = 0; i < m_mapTrackingIdPos.size(); i++)
	{
		if (m_mapTrackingIdPos[i].size() > 0)
			m_VecBackLastPos.push_back(m_mapTrackingIdPos[i].back());
	}
	m_mapTrackingIdPos.clear();
	m_VecBtnStatus.clear();
}

void CCommonModeNoiseRejectionDlg::SaveCMNRTestResult() {
	CString tmpStr;
    std::vector< std::pair<float, bool > > vecAmplResultFail;
	vecAmplResultFail.clear();
    bool bAmplFail = false;
	unsigned int countFail = 0;
    tmpStr.Format(_T("\n\n*** Frequency, pass / fail list of voltages ***\n"));
    m_FileCMNRTestResult.WriteString(tmpStr);
    for (int freq = m_CmnrCfg.iMinFreq; freq <= m_CmnrCfg.iMaxFreq; freq = freq + m_CmnrCfg.iIncFreq) {
        for (float amp = m_CmnrCfg.fMinAmpl; amp <= m_CmnrCfg.fMaxAmpl; amp = amp + m_CmnrCfg.fAmpStep)
        {
            for (auto it = m_VecCmnrResult.begin(); it != m_VecCmnrResult.end(); it++)
            {
                if (freq == it->first.iFreq && amp == it->first.fAmpl) {
                    vecAmplResultFail.push_back({ amp, false });
                }
            }
        }
        tmpStr.Format(_T("%d, {"), freq);
        if (vecAmplResultFail.empty())
        {
            for (float amp = m_CmnrCfg.fMinAmpl; amp <= m_CmnrCfg.fMaxAmpl; amp = amp + m_CmnrCfg.fAmpStep)
            {
				if (amp >= m_CmnrCfg.fMaxAmpl || (amp + m_CmnrCfg.fAmpStep > m_CmnrCfg.fMaxAmpl))
					tmpStr.AppendFormat(_T("%0.1f: Pass}\n"), amp);
				else
					tmpStr.AppendFormat(_T("%0.1f: Pass, "), amp);
                
            }
			//m_eTestResult = PASS;
        }
        else
        {
            for (float amp = m_CmnrCfg.fMinAmpl; amp <= m_CmnrCfg.fMaxAmpl; amp = amp + m_CmnrCfg.fAmpStep)
            {
                tmpStr.AppendFormat(_T("%0.1f: "), amp);
                for (auto it = vecAmplResultFail.begin(); it != vecAmplResultFail.end(); it++)
                {
                    if (amp == it->first)
                    {
                        bAmplFail = true;
                        break;
                    }
                }
                if (bAmplFail)
                {
                    if (amp >= m_CmnrCfg.fMaxAmpl || (amp + m_CmnrCfg.fAmpStep > m_CmnrCfg.fMaxAmpl))
                        tmpStr.AppendFormat(_T("Fail}\n"));
                    else
                        tmpStr.AppendFormat(_T("Fail, "));
                }
                else {
                    if (amp >= m_CmnrCfg.fMaxAmpl || (amp + m_CmnrCfg.fAmpStep > m_CmnrCfg.fMaxAmpl))
                        tmpStr.AppendFormat(_T("Pass}\n"));
                    else
                        tmpStr.AppendFormat(_T("Pass, "));
                }
                bAmplFail = false;
            }
			countFail++; //m_eTestResult = NG;
        }
        m_FileCMNRTestResult.WriteString(tmpStr);
        vecAmplResultFail.clear();
    }
	if (countFail > 0)
		m_eTestResult = TestResult::NG;
	else
		m_eTestResult = TestResult::PASS;
    m_FileCMNRTestResult.Flush();

	m_FileCMNRTestResult.WriteString(_T("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));

	m_FileCMNRTestResult.WriteString(_T("*** Test Result Error Type ***\n\n"));
	for (int i = 0; i < m_listResult.GetCount(); i++)
	{
		m_listResult.GetText(i, tmpStr);
		m_FileCMNRTestResult.WriteString(tmpStr);
	}
	m_FileCMNRTestResult.Flush();
}

void CCommonModeNoiseRejectionDlg::GetCornerPtFromUI()
{
	CString strTitle;
	float x{ 0.0 }, y{ 0.0 };

	m_CornerPts.clear();

	GetDlgItemTextW( IDC_EDIT_X0, strTitle );
	if (!strTitle.IsEmpty())
		x = (float)_ttof(strTitle);
	GetDlgItemTextW(IDC_EDIT_Y0, strTitle);
	if (!strTitle.IsEmpty())
		y = (float)_ttof(strTitle);
	m_CornerPts.emplace_back(x, y);

	GetDlgItemTextW(IDC_EDIT_X1, strTitle);
	if (!strTitle.IsEmpty())
		x = (float)_ttof(strTitle);
	GetDlgItemTextW(IDC_EDIT_Y1, strTitle);
	if (!strTitle.IsEmpty())
		y = (float)_ttof(strTitle);
	m_CornerPts.emplace_back(x, y);

	GetDlgItemTextW(IDC_EDIT_X2, strTitle);
	if (!strTitle.IsEmpty())
		x = (float)_ttof(strTitle);
	GetDlgItemTextW(IDC_EDIT_Y2, strTitle);
	if (!strTitle.IsEmpty())
		y = (float)_ttof(strTitle);
	m_CornerPts.emplace_back(x, y);

	GetDlgItemTextW(IDC_EDIT_X3, strTitle);
	if (!strTitle.IsEmpty())
		x = (float)_ttof(strTitle);
	GetDlgItemTextW(IDC_EDIT_Y3, strTitle);
	if (!strTitle.IsEmpty())
		y = (float)_ttof(strTitle);
	m_CornerPts.emplace_back(x, y);

	GetDlgItemTextW(IDC_EDIT_X4, strTitle);
	if (!strTitle.IsEmpty())
		x = (float)_ttof(strTitle);
	GetDlgItemTextW(IDC_EDIT_Y4, strTitle);
	if (!strTitle.IsEmpty())
		y = (float)_ttof(strTitle);
	
	m_CurPt.first = x;
	m_CurPt.second = y;

	CString strEdgeSize;
	GetDlgItem(IDC_EDIT_EDGE)->GetWindowTextW(strEdgeSize);
	m_EdgeSize = _ttoi(strEdgeSize);

	m_PtCloudWin->SetEdgeSize(m_EdgeSize);
}

void CCommonModeNoiseRejectionDlg::OnBnClickedBtnCmnrStart()
{
	CString str;
	m_BtnCMNRStart.GetWindowText(str);
	m_listResult.ResetContent();
	KillTimer(CMNR_TIMER_LINEAR_AXIS_LIMIT);
	if (!str.Compare(_T("Start")))
	{
		m_StopThread = false;
		GetCornerPtFromUI();

		m_PtCloudWin->ClearAll();


		if (GetConfig()) {
			m_CMNRTestThread = AfxBeginThread(ThreadDoCMNRTest, (LPVOID)this);
		}
	}
	else if (!str.Compare(_T("Stop")))
	{
		m_BtnCMNRStart.SetWindowText(_T("Start"));
		m_StopThread = true;
	}
	
}

void CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboWaveform()
{
	// TODO: 在此添加控件通知处理程序代码
	int wfIdx = m_ComboWaveForm.GetCurSel();
	int show = (wfIdx == 2) ? SW_SHOW : SW_HIDE;
	m_SliderDuty.ShowWindow(show);
	((CEdit*)GetDlgItem(IDC_EDIT_DUTY))->ShowWindow(show);
	GetDlgItem(IDC_STATIC_DUTY)->ShowWindow(show);
}

void CCommonModeNoiseRejectionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_SliderDuty) {
		int value = m_SliderDuty.GetPos();
		//TRACE("duty cycle value = %d\n", value);
		CString stDuty;
		stDuty.Format(_T("%d%%\n"), value);
		((CEdit*)GetDlgItem(IDC_EDIT_DUTY))->SetWindowTextW(stDuty);
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CCommonModeNoiseRejectionDlg::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && ( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE ) )
	{
		if ( pMsg->wParam == VK_RETURN ) 
		{
			CString strSize;
			((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->GetWindowText(strSize);
			int nWidth = _ttoi(strSize);

			((CEdit*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->GetWindowText(strSize);
			int nHeight = _ttoi(strSize);

			int minSize = min(nWidth,nHeight);

			if ( pMsg->hwnd == GetDlgItem(IDC_EDIT_TP_WIDTH)->m_hWnd ||pMsg->hwnd == GetDlgItem(IDC_EDIT_TP_HEIGHT)->m_hWnd )
			{
				SetValidLaps(minSize);
			}
			else if ( pMsg->hwnd == GetDlgItem(IDC_EDIT_RANGE)->m_hWnd )
			{
				CString strRange;
				GetDlgItemText( IDC_EDIT_RANGE, strRange );
				m_nRange = _tcstof(strRange,NULL);

				if ( m_nRange < 1.0 )
				{
					m_nRange = 1.0;

					strRange.Format( _T("%.1f"),m_nRange );
					SetDlgItemText( IDC_EDIT_RANGE, strRange );
					AfxMessageBox(_T("螺旋间距不能小于1.0mm，请重新设定！"),MB_OK );
				}

				SetValidLaps(minSize);
			}
			else if ( pMsg->hwnd == GetDlgItem(IDC_COMBO_LAPS)->m_hWnd )
			{
				SetValidLaps(minSize);

				CString strInput, strMax, strMin;

				GetDlgItemText(IDC_COMBO_LAPS, strInput);
				GetDlgItemText(IDC_STATIC_MAX, strMax);
				GetDlgItemText(IDC_STATIC_MIN, strMin);

				strMax = strMax.Right(strMax.GetLength() - 2);
				strMin = strMin.Left(strMin.GetLength() - 2);

				if ( strInput > strMax || strInput <= strMin )
				{
					SetDlgItemText(IDC_COMBO_LAPS, strMax);

					AfxMessageBox(_T("圈数设定超出推荐设定范围!\n 已默认设定到最大值，请重新设定！"), MB_OK);

					return FALSE;
				}
			}
		}

		UpdateData();
		InvalidateRect(mDrawPathArea);

		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}

	if (pMsg->message == WM_LBUTTONDOWN)//按钮摁下
	{
		//SetCapture();
		if (pMsg->hwnd == GetDlgItem(IDC_BTN_RIGHT_X)->m_hWnd)
		{
			g_CtrlCard.OnButtonCmove(AXIS_X, POSITION_DIRECTION); //正方向
			m_eAxisType = AxisType::AXIS_X_RIGHT;
			TRACE("XRD\n");
			pMsg->hwnd = NULL;
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_BTN_LEFT_X)->m_hWnd) {
			g_CtrlCard.OnButtonCmove(AXIS_X, NEGATIVE_DIRECTION); //反方向
			m_eAxisType = AxisType::AXIS_X_LEFT;
			TRACE("XLD\n");
			pMsg->hwnd = NULL;
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_BTN_UP_Y)->m_hWnd) {
			g_CtrlCard.OnButtonCmove(AXIS_Y, NEGATIVE_DIRECTION); //向里方向
			m_eAxisType = AxisType::AXIS_Y_UP;
			TRACE("YUD\n");
			pMsg->hwnd = NULL;
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN_Y)->m_hWnd) {
			g_CtrlCard.OnButtonCmove(AXIS_Y, POSITION_DIRECTION); //向外方向
			m_eAxisType = AxisType::AXIS_Y_DOWN;
			TRACE("YDD\n");
			pMsg->hwnd = NULL;
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_BTN_UP_Z)->m_hWnd) {
			g_CtrlCard.OnButtonCmove(AXIS_Z, NEGATIVE_DIRECTION); //往上方向
			m_eAxisType = AxisType::AXIS_Z_UP;
			TRACE("ZUD\n");
			pMsg->hwnd = NULL;
		}
		else if (pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN_Z)->m_hWnd) {
			g_CtrlCard.OnButtonCmove(AXIS_Z, POSITION_DIRECTION); //往下方向
			m_eAxisType = AxisType::AXIS_Z_DOWN;
			TRACE("ZDD\n");
			pMsg->hwnd = NULL;
		}
	}
	if (pMsg->message == WM_LBUTTONUP)//按钮松开
	{
		//按钮松开
		CString str = _T("");

		//X轴停止
		if ((pMsg->hwnd == GetDlgItem(IDC_BTN_RIGHT_X)->m_hWnd) || (pMsg->hwnd == GetDlgItem(IDC_BTN_LEFT_X)->m_hWnd))
		{
			// 在此调用Button按钮抬起的操作
			sudden_stop(g_cardno, AXIS_X);
			g_WorkStatus = 0;
			g_CtrlCard.Get_CurrentInf(AXIS_X, g_log, g_act, g_spd);
			m_fPosValueX = ((float)g_log / (float)1000);
			m_CmnrCfg.fAxis_X = m_fPosValueX;
			str.Format(_T("%0.3f"), m_fPosValueX);
			((CEdit*)GetDlgItem(IDC_EDIT_POS_X))->SetWindowTextW(str);
			TRACE("XU\n");
			//pMsg->hwnd = NULL;
		}
		//Y轴停止
		else if ((pMsg->hwnd == GetDlgItem(IDC_BTN_UP_Y)->m_hWnd) || (pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN_Y)->m_hWnd))
		{
			// 在此调用Button按钮抬起的操作
			sudden_stop(g_cardno, AXIS_Y);
			g_WorkStatus = 0;
			g_CtrlCard.Get_CurrentInf(AXIS_Y, g_log, g_act, g_spd);
			m_fPosValueY = ((float)g_log / (float)1000);
			m_CmnrCfg.fAxis_Y = m_fPosValueY;
			str.Format(_T("%0.3f"), m_fPosValueY);
			((CEdit*)GetDlgItem(IDC_EDIT_POS_Y))->SetWindowTextW(str);
			TRACE("YU\n");
			//pMsg->hwnd = NULL;
		}
		//Z轴停止
		else if ((pMsg->hwnd == GetDlgItem(IDC_BTN_UP_Z)->m_hWnd) || (pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN_Z)->m_hWnd))
		{
			// 在此调用Button按钮抬起的操作
			sudden_stop(g_cardno, AXIS_Z);
			g_WorkStatus = 0;
			g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
			m_fPosValueZ = ((float)g_log / (float)1000);
			str.Format(_T("%0.3f"), m_fPosValueZ);
			((CEdit*)GetDlgItem(IDC_EDIT_TOUCH_HIGH))->SetWindowTextW(str);
			TRACE("ZU\n");
			//pMsg->hwnd = NULL;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CCommonModeNoiseRejectionDlg::OnBnClickedBtnHome()
{
	int     retn[] = { -1, -1, -1 };   
	char    Axis[] = { 'X', 'Y', 'Z' };
	CString str;
	//运行Home时不允许其它箭头可操作。
	EnableLinearOperation(false);

    m_StaticProMsg.ShowWindow(SW_SHOWNORMAL);

	//多轴回原点,必须等待上一轴回原点结束后，才能执行下一轴的回原点动作。
	//先让Z轴回零，再Y，X轴，防止Z轴太低撞机。
	for (int i = MAXAXIS - 1; i > 0; i--)
	{
		retn[i - 1] = g_CtrlCard.BackHome(i, m_StaticProMsg);
		if (retn[i - 1] < 0)
		{
			str.Format(_T("%c return Home fail, return value %d"), Axis[i - 1], retn[i - 1]);
			AfxMessageBox(str);
			goto HOME_EXIT;
		}
	}
	
	AfxMessageBox(_T("Return Home success"));
	SetDlgItemInt(IDC_EDIT_POS_X, 0);
	SetDlgItemInt(IDC_EDIT_POS_Y, 0);

HOME_EXIT:
	m_fPosValueX = 0.0;
	m_fPosValueY = 0.0;
	m_fPosValueZ = 0.0;
	EnableLinearOperation(true);
	m_StaticProMsg.ShowWindow(SW_HIDE);
}

void CCommonModeNoiseRejectionDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
#ifdef LINEAR_ENABLE
	for (int i = 1; i <= MAXAXIS; i++)
	{
		sudden_stop(g_cardno, i);
	}

	g_WorkStatus = 0;

	Delay(100);

	g_CtrlCard.End_Board();	//进行对卡的释放
#endif
	CDialogEx::OnClose();
}

void CCommonModeNoiseRejectionDlg::JudgeLimit()
{
    int value;
    if (AxisType::AXIS_X_RIGHT == m_eAxisType)//X 轴正限位
    {
        value = g_CtrlCard.Read_Input(AXIS_X_POSITIVE_LIMIT); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_X);
        }
    }
    else if (AxisType::AXIS_X_LEFT == m_eAxisType)//X 轴负限位
    {
        value = g_CtrlCard.Read_Input(AXIS_X_STOP0); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_X);
        }
        else
        {
            value = g_CtrlCard.Read_Input(AXIS_X_NEGATIVE_LIMIT); //读取信号
            if (0 == value)
            {
                sudden_stop(g_cardno, AXIS_X);
            }
        }
    }
    else if (AxisType::AXIS_Y_DOWN == m_eAxisType)//Y 轴正限位
    {
        value = g_CtrlCard.Read_Input(AXIS_Y_POSITIVE_LIMIT); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_Y);
        }
    }
    else if (AxisType::AXIS_Y_UP == m_eAxisType)//Y 轴负限位
    {
        value = g_CtrlCard.Read_Input(AXIS_Y_STOP0); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_Y);
        }
        else
        {
            value = g_CtrlCard.Read_Input(AXIS_Y_NEGATIVE_LIMIT); //读取信号
            if (0 == value)
            {
                sudden_stop(g_cardno, AXIS_Y);
            }
        }
    }
    else if (AxisType::AXIS_Z_DOWN == m_eAxisType)//Z 轴正限位
    {
        value = g_CtrlCard.Read_Input(AXIS_Z_POSITIVE_LIMIT); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_Z);
        }
    }
    else if (AxisType::AXIS_Z_UP == m_eAxisType)//Z 轴负限位
    {
        value = g_CtrlCard.Read_Input(AXIS_Z_STOP0); //读取信号
        if (0 == value)
        {
            sudden_stop(g_cardno, AXIS_Z);
        }
        else
        {
            value = g_CtrlCard.Read_Input(AXIS_Z_NEGATIVE_LIMIT); //读取信号
            if (0 == value)
            {
                sudden_stop(g_cardno, AXIS_Z);
            }
        }
    }
}

void CCommonModeNoiseRejectionDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (CMNR_TIMER_LINEAR_AXIS_LIMIT == nIDEvent)
	{
		JudgeLimit();
	}

	CDialogEx::OnTimer(nIDEvent);
}

//< 两轴插补
void CCommonModeNoiseRejectionDlg::Interp_moveXY()
{
    CString text, msg;

    long lAxisX = 0, lAxisY = 0;
    //< 获取X轴值
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POS_X);
    pEdit->GetWindowTextW(text);
    lAxisX = static_cast<long>(_tcstof(text, NULL) * 1000); //< mm 转化成 pulse

    if (lAxisX > PLUS_MINUS_LIMIT_DISTANCE)
    {
        msg.Format(_T("X值不能超过X轴正负限位距离 %dmm\n"), PLUS_MINUS_LIMIT_DISTANCE / 1000);
        AfxMessageBox(msg);
        return;
    }
    //获取Y轴值
    pEdit = (CEdit*)GetDlgItem(IDC_EDIT_POS_Y);
    pEdit->GetWindowTextW(text);
    lAxisY = static_cast<long>(_tcstof(text, NULL) * 1000);

    if (lAxisY > PLUS_MINUS_LIMIT_DISTANCE)
    {
        msg.Format(_T("Y值不能超过Y轴正负限位距离 %dmm\n"), PLUS_MINUS_LIMIT_DISTANCE / 1000);
        AfxMessageBox(msg);
        return;
    }
    if (lAxisX > 0 || lAxisY > 0) //< 以原点出发，肯定都是正值。
    {
        g_CtrlCard.OnButtonInp(AXIS_X, AXIS_Y, lAxisX, lAxisY);
    }

}

void CCommonModeNoiseRejectionDlg::OnBnClickedBtnErrorTypeSel()
{
	memcpy(&m_cErrTypeSelDlg.m_rErrTypeSel, &m_rErrorTypeSel, sizeof(ErrorTypeSel));
	int ret = m_cErrTypeSelDlg.DoModal();
	if (IDOK == ret)
	{
		//DoSomething
		memcpy(&m_rErrorTypeSel, &m_cErrTypeSelDlg.m_rErrTypeSel, sizeof(ErrorTypeSel));
	}
}

void CCommonModeNoiseRejectionDlg::InitLinearTestListCtl()
{
	LONG lStyle;
	lStyle = GetWindowLong(m_PathTypeList.m_hWnd, GWL_STYLE );
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_PathTypeList.m_hWnd, GWL_STYLE, lStyle );

	DWORD dwStyle = m_PathTypeList.GetExtendedStyle();
	m_PathTypeList.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES | LVS_EDITLABELS | LVS_EX_CHECKBOXES );
	m_PathTypeList.InsertColumn(0, _T("轨迹类型"), LVCFMT_LEFT, 300);

	m_PathTypeList.InsertItem( 0, _T("螺旋打点-顺时针-由内到外"));
	m_PathTypeList.InsertItem( 1, _T("螺旋打点-顺时针-由外到内"));
	m_PathTypeList.InsertItem( 2, _T("螺旋打点-逆时针-由内到外"));
	m_PathTypeList.InsertItem( 3, _T("螺旋打点-逆时针-由外到内"));

	m_PathTypeList.SetCheck( 0, TRUE );
}

void CCommonModeNoiseRejectionDlg::UnCheckOtherItem(CListCtrl& listCtrl, int index)
{
	for (int i = 0; i < listCtrl.GetItemCount(); ++i)
	{
		if (i == index)
			continue;
		listCtrl.SetCheck(i, FALSE);
	}
}

void CCommonModeNoiseRejectionDlg::OnLvnItemchangedListPathType(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ( m_PathTypeList.GetCheck( pNMLV->iItem) )
	{
		UnCheckOtherItem( m_PathTypeList, pNMLV->iItem );
	}

	InvalidateRect(mDrawPathArea);
	UpdateWindow();
	*pResult = 0;
}


void CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboLaps()
{
	InvalidateRect(mDrawPathArea);
	UpdateWindow();
	int idx = GetCheckIndex();
	DrawSpiral(idx);
}

/* get which index has be checked in listctrl */
int CCommonModeNoiseRejectionDlg::GetCheckIndex()
{
	for ( int i = 0; i < m_PathTypeList.GetItemCount(); ++i )
	{
		if ( m_PathTypeList.GetCheck(i) )
		{
			return i;
		}
	}
	return -1;
}

void run( HFST::ILinearTest* p , void* pAppDlg)
{
	std::thread thread( [&]() 
	{
		p->Run( pAppDlg );
	});

	thread.join();
#if 0
	HFST::ScopedThread sThread( std::thread(
		[&]()
		{
			p->Run(pAppDlg);
		}
	) );
#endif
}

void CCommonModeNoiseRejectionDlg::OnBnClickedMfcbuttonRun()
{
	//< check whether Linear Machine is safe
	int nTpWidth = GetDlgItemInt( IDC_EDIT_TP_WIDTH );
	int nTpHeight = GetDlgItemInt( IDC_EDIT_TP_HEIGHT );
	int nLinearX = GetDlgItemInt(IDC_EDIT_POS_X);
	int nLinearY = GetDlgItemInt(IDC_EDIT_POS_Y);
	if (  max( nLinearX,nLinearY ) <= min( nTpWidth, nTpHeight ) )
	{
		AfxMessageBox( _T("System detected that Linear Machine position is dangerous!\n\nTest Canceled!\n\nMove machine to safe position and try again!\n\n"), MB_ICONWARNING | MB_OK );
		return;
	}

	m_LinearTProbar.ShowWindow(SW_SHOW);
	((CMFCButton*)GetDlgItem(IDC_MFCBUTTON_RUN))->SetWindowTextW(_T("测试中..."));
	((CMFCButton*)GetDlgItem(IDC_MFCBUTTON_RUN))->EnableWindow(FALSE);
	m_PathTypeList.EnableWindow(FALSE);
	m_LapCombo.EnableWindow(FALSE);
	((CComboBox*)GetDlgItem(IDC_COMBO_TP_TYPE))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_EDIT_RANGE))->EnableWindow(FALSE);

	//< check which item to test
	int nCheckIdx = GetCheckIndex();
	if ( nCheckIdx == -1 )
	{
		return;
	}

	//< see how many laps to test
	CString strLap;
	m_LapCombo.GetWindowText(strLap);
	float nLaps = _ttof(strLap);

	std::unique_ptr<HFST::LinearFactory> pFactory( HFST::CreateLinearFactory(nCheckIdx, nLaps) );
	if ( !pFactory )
	{
		AfxMessageBox(_T("fail, invalid selection!"),MB_OK);
		return;
	}
	m_pLinearTest = pFactory->CreateLinearTest();

	std::thread thread( run, std::ref(m_pLinearTest), this );
	thread.detach();
}

void CCommonModeNoiseRejectionDlg::DrawPoint( double xPos, double yPos)
{
	CClientDC dc(this);
	CPen pen( PS_SOLID, 4, RGB(0, 255, 0) );
	CBrush brush( RGB(0, 255, 0) );

	dc.SelectObject( &pen );
	dc.SelectObject( &brush );

	int nCenterX = mDrawPathArea.left + ( mDrawPathArea.right - mDrawPathArea.left ) / 2;
	int nCenterY = mDrawPathArea.top + ( mDrawPathArea.bottom - mDrawPathArea.top ) / 2;

	dc.MoveTo( nCenterX + (int)xPos, nCenterY + (int)yPos );
	dc.LineTo( nCenterX + (int)xPos, nCenterY + (int)yPos );
}


void CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboTpType()
{
	int nSel = m_ComboTpType.GetCurSel();

	if ( nSel == 0 )	//< Rectangle
	{
		((CStatic*)GetDlgItem(IDC_STATIC_TAG))->SetWindowTextW(_T("宽/高(mm)"));
		((CStatic*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->ShowWindow(SW_SHOW);

		CString strSize;
		((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->GetWindowText(strSize);
		int nWidth = _ttoi(strSize);

		((CEdit*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->GetWindowText(strSize);
		int nHeight = _ttoi(strSize);

		SetValidLaps( min( nWidth, nHeight ) );
	}
	else if( nSel == 1 )//< Circle	
	{
		((CStatic*)GetDlgItem(IDC_STATIC_TAG))->SetWindowTextW(_T("直径(mm)"));
		((CStatic*)GetDlgItem(IDC_EDIT_TP_HEIGHT))->ShowWindow(SW_HIDE);

		CString strSize;
		((CEdit*)GetDlgItem(IDC_EDIT_TP_WIDTH))->GetWindowText(strSize);
		int nWidth = _ttoi(strSize);
		SetValidLaps( nWidth );
	}

	InvalidateRect( mDrawPathArea );
}

BOOL CCommonModeNoiseRejectionDlg::SystemBackHome()
{
	g_CtrlCard.Setup_Speed( 1, m_Speed, m_Speed, 500, 500, 5, 2 );
	for ( int i = 3; i > 0; --i )
	{
		if (g_CtrlCard.BackHome(i, m_StaticProMsg) < 0)
			return FALSE;
	}

	return TRUE;
}

void CCommonModeNoiseRejectionDlg::BackToTpCenter()
{
	MoveZUp();

	//< see how many laps to test
	CString strLap;
	m_LapCombo.GetLBText(m_LapCombo.GetCurSel(), strLap);
	float nLaps = _ttof(strLap);

	auto [toX, toY] = m_BackPtVec.front();
	auto [fromX, fromY] = m_BackPtVec.back();

	double nPulseX = (toX / ZOOM - fromX / ZOOM) / ACCX;
	double nPulseY = (toY / ZOOM - fromY / ZOOM) / ACCY;
	g_CtrlCard.Axis_Pmove(1, (long)nPulseX);
	g_CtrlCard.Axis_Pmove(2, (long)nPulseY);

	//< get status of every axis, if status == 0 && speed == 0, means move finish
	while (1)
	{
		int nStatusX = -1;
		int nStatusY = -1;
		g_CtrlCard.Get_Status(1, nStatusX, 0);
		g_CtrlCard.Get_Status(2, nStatusY, 0);

		int nSpeedX = g_CtrlCard.GetSpeed(0, 1);
		int nSpeedY = g_CtrlCard.GetSpeed(0, 2);

		if (nStatusX == 0 && nStatusY == 0 && nSpeedX == 0 && nSpeedY == 0)
		{
			break;
		}
	}

	MoveZDown();
}

void CCommonModeNoiseRejectionDlg::MoveZUp()
{
	g_CtrlCard.Axis_Pmove(3, -Dist_Z);
	while (1)
	{
		int nStatus = -1;
		g_CtrlCard.Get_Status(3, nStatus, 0);
		int nSpeedZ = g_CtrlCard.GetSpeed(0, 3);

		if (nStatus == 0 && nSpeedZ == 0)
		{
			break;
		}
	}
}
void CCommonModeNoiseRejectionDlg::MoveZDown()
{
	g_CtrlCard.Axis_Pmove(3, Dist_Z);
	while (1)
	{
		int nStatus = -1;
		g_CtrlCard.Get_Status(3, nStatus, 0);
		int nSpeedZ = g_CtrlCard.GetSpeed(0, 3);

		if (nStatus == 0 && nSpeedZ == 0)
		{
			break;
		}
	}
}

void CCommonModeNoiseRejectionDlg::InitToolBar()
{
	m_ToolBar.Create
	(
		this,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS
	);
	m_ToolBar.LoadToolBar(IDR_TOOLBAR_Settings);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	m_ToolBarImage.Create(32, 32, ILC_COLOR32 | ILC_MASK, 2, 2);
	m_ToolBarImage.SetBkColor(RGB(255, 255, 255));

	CPngImage* pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_LINEAR);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_FUN_GEN);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	m_ToolBar.SetButtonText(0, _T("&Linear"));
	m_ToolBar.SetButtonText(1, _T("&显示打点轨迹"));

	m_ToolBar.GetToolBarCtrl().SetImageList(&m_ToolBarImage);
	m_ToolBar.GetToolBarCtrl().EnableButton(IDB_PNG_LINEAR, TRUE);
	m_ToolBar.GetToolBarCtrl().EnableButton(IDB_PNG_FUN_GEN, FALSE);
	m_ToolBar.ShowWindow(SW_SHOW);
}

void CCommonModeNoiseRejectionDlg::OnNMCustomdrawSliderSpeed(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	m_Speed = m_SpeedSlide.GetPos() * 1000;
	
	CString strSpeed;
	strSpeed.Format( _T("%d ( pulse/s )"),m_Speed );
	SetDlgItemText( IDC_STATIC_PULSE, strSpeed );

	*pResult = 0;
}

void CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboSelect()
{
	CComboBox* pCom = (CComboBox*)GetDlgItem(IDC_COMBO_SELECT);
	int nSel = pCom->GetCurSel();
	if (nSel == 0)
	{
		//IDC_BUTTON_LOAD_POINT_FILE
		GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_HIDE);
		//IDC_STATIC_POINT_RESTART
		GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_HIDE);
		//IDC_EDIT_START
		GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_HIDE);

		m_TestMode = TestMode::POINTS_1000_RANDOM;
	}
	else if (nSel == 1)
	{
		//IDC_BUTTON_LOAD_POINT_FILE
		GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_SHOW);
		//IDC_STATIC_POINT_RESTART
		GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_SHOW);
		//IDC_EDIT_START
		GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_SHOW);

		m_TestMode = TestMode::POINTS_1000_RANDOM_CONTINUE;
	}
}

void CCommonModeNoiseRejectionDlg::OnBnClickedRadioNormalCmnrTest()
{
	//IDC_BUTTON_LOAD_POINT_FILE
	GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_HIDE);
	//IDC_STATIC_POINT_RESTART
	GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_HIDE);
	//IDC_EDIT_START
	GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_HIDE);
	//IDC_COMBO_SELECT
	CComboBox* pCom = ((CComboBox*)GetDlgItem(IDC_COMBO_SELECT));
	pCom->ShowWindow(SW_HIDE);

	// IDC_CHECK_ACCURATE
	GetDlgItem(IDC_CHECK_ACCURATE)->ShowWindow(SW_HIDE);

	//IDC_CHECK_DEBUG
	GetDlgItem(IDC_CHECK_DEBUG)->ShowWindow(SW_HIDE);
	//IDC_STATIC_EDGE
	GetDlgItem(IDC_STATIC_EDGE)->ShowWindow(SW_HIDE);
	//IDC_EDIT_EDGE
	GetDlgItem(IDC_EDIT_EDGE)->ShowWindow(SW_HIDE);

	m_TestMode = TestMode::DEFAULT;
}

void CCommonModeNoiseRejectionDlg::OnBnClickedButtonLoadPointFile()
{
	mAllPoint.clear();

	CString		FileName;
	CFileDialog	FDlg(TRUE, _T("*.txt"), _T("*.txt"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("TXT File|*.txt|"), this);

	if (FDlg.DoModal() == IDCANCEL)
	{
		return;
	}
	FileName = FDlg.GetPathName();

	USES_CONVERSION;
	std::ifstream ifs(T2A(FileName), std::ios::in);
	if (!ifs)
	{
		return;
	}

	std::string sRead;
	while (getline(ifs, sRead))
	{
		if (sRead.empty())
		{
			continue;
		}

		if (sRead.find("Point-") != std::string::npos)
		{
			sRead = sRead.substr(6, sRead.size());
			int nPointIdx = stoi(sRead);

			int xPos = 0, yPos = 0;
			if (getline(ifs, sRead))
			{
				int idx = sRead.find("\t");
				xPos = stoi(sRead.substr(idx, sRead.size()));
			}
			if (getline(ifs, sRead))
			{
				int idx = sRead.find("\t");
				yPos = stoi(sRead.substr(idx, sRead.size()));
			}

			mAllPoint.emplace_back(xPos, yPos, nPointIdx);
		}
	}

	m_TestMode = TestMode::POINTS_1000_RANDOM_CONTINUE;

	ifs.close();
}


void CCommonModeNoiseRejectionDlg::OnBnClickedRadio1000Random()
{
	//IDC_COMBO_SELECT
	CComboBox* pCom = ((CComboBox*)GetDlgItem(IDC_COMBO_SELECT));
	pCom->ShowWindow(SW_SHOW);

	int nSel = pCom->GetCurSel();
	if ( nSel == 0 )
	{
		//IDC_BUTTON_LOAD_POINT_FILE
		GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_HIDE);
		//IDC_STATIC_POINT_RESTART
		GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_HIDE);
		//IDC_EDIT_START
		GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_HIDE);

		//IDC_CHECK_DEBUG
		GetDlgItem(IDC_CHECK_DEBUG)->ShowWindow(SW_SHOW);
		//IDC_STATIC_EDGE
		GetDlgItem(IDC_STATIC_EDGE)->ShowWindow(SW_SHOW);
		//IDC_EDIT_EDGE
		GetDlgItem(IDC_EDIT_EDGE)->ShowWindow(SW_SHOW);

		m_TestMode = TestMode::POINTS_1000_RANDOM;
	}
	else if( nSel == 1 )
	{
		//IDC_BUTTON_LOAD_POINT_FILE
		GetDlgItem(IDC_BUTTON_LOAD_POINT_FILE)->ShowWindow(SW_SHOW);
		//IDC_STATIC_POINT_RESTART
		GetDlgItem(IDC_STATIC_POINT_RESTART)->ShowWindow(SW_SHOW);
		//IDC_EDIT_START
		GetDlgItem(IDC_EDIT_START)->ShowWindow(SW_SHOW);

		m_TestMode = TestMode::POINTS_1000_RANDOM_CONTINUE;
	}

	// IDC_CHECK_ACCURATE
	GetDlgItem( IDC_CHECK_ACCURATE )->ShowWindow(SW_SHOW);
}

void CCommonModeNoiseRejectionDlg::LogFailPt(
	std::vector<vPoint>& failPt,
	std::fstream& ofs,
	const vPoint& ActualPoint,			// pulse 
	const CPoint& ActualPointDevice,	// pixel
	const CPoint& RealPoint,			// pixel
	int index, 
	const CString& strWhy,
	BOOL bRetry/* = FALSE*/
)
{
	CString strErrResult = strWhy;
	strErrResult.AppendFormat(_T("[%d] , 计算坐标(像素)[x:%d, y:%d], 报点坐标(像素)[x:%d, y:%d]\r\n"), index, ActualPointDevice.x, ActualPointDevice.y, RealPoint.x, RealPoint.y);
	m_listResult.InsertString(0, strErrResult);

	if (strWhy != _T("[PASS]"))
	{
		if (failPt.empty())
		{
			ofs << "//----------------------------------------------------------------\n";
			ofs << "//--  Test Result :\n";
			ofs << "//----------------------------------------------------------------\n";
		}
		vPoint fail;
		fail.x = ActualPoint.x;
		fail.y = ActualPoint.y;
		fail.idx = index;
		failPt.push_back(fail);

		ofs << "Point-" << fail.idx << "\n";
		ofs << "Result： " << std::string(CT2A(strWhy)) << "\n";
		ofs << "Touch(mm)：" << "\t[" << m_TpTotalWidth - ActualPoint.x * ACCX << "," << ActualPoint.y * ACCY << "]\n";
		ofs << "Report(mm)：" << "\t[" << (float(RealPoint.y) * m_TpTotalWidth) / float(m_ResolutionX) << "," << (float(RealPoint.x) * m_TpTotalHeight) / float(m_ResolutionY) << "]\n";
		ofs << "Touch(pixel)：" << "\t[" << ActualPointDevice.y << "," << ActualPointDevice.x << "]\n";
		ofs << "Report(pixel)：" << "\t[" << RealPoint.y << "," << RealPoint.x << "]\n";

		float disX = m_TpTotalWidth - ActualPoint.x * ACCX - (float(RealPoint.y) * m_TpTotalWidth) / float(m_ResolutionX);
		float disY = ActualPoint.y * ACCY - (float(RealPoint.x) * m_TpTotalHeight) / float(m_ResolutionY);
		ofs << "Distance(mm):" << "\t" << sqrt(pow(disX, 2) + pow(disY, 2)) << "\n" << std::endl;

		if(!bRetry)
			m_PtCloudWin->AddFailPt( { 
				m_ResolutionX - RealPoint.y ,
				RealPoint.x ,
				index
			} );
	}
	else
	{
		if (!bRetry)
		{
			m_PtCloudWin->AddSuccessPt({
				m_ResolutionX - RealPoint.y ,
				RealPoint.x ,
				index
			});
		}
		else
		{
			// PASS ,remove fail point
			m_PtCloudWin->RemoveFailPt( { ActualPoint.x ,ActualPoint.y, index } );
		}
	}
}

void CCommonModeNoiseRejectionDlg::OnBnClickedCheckAccurate()
{
	BOOL checked = ((CButton*)GetDlgItem(IDC_CHECK_ACCURATE))->GetCheck();

	m_bAccuraceEnable = checked ? TRUE : FALSE;
}

void CCommonModeNoiseRejectionDlg::OnBnClickedButtonShowPoint()
{
	m_PtCloudWin->ShowWindow(SW_SHOW);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditOffsetX()
{
	CString strOffsetX;
	GetDlgItemText(IDC_EDIT_OFFSET_X, strOffsetX);
	m_OffsetX_New = (float)_ttof(strOffsetX);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditOffsetY()
{
	CString strOffsetY;
	GetDlgItemText(IDC_EDIT_OFFSET_Y, strOffsetY);
	m_OffsetY_New = (float)_ttof(strOffsetY);
}


void CCommonModeNoiseRejectionDlg::OnBnClickedCheckAccurate2()
{
	BOOL checked = ((CButton*)GetDlgItem(IDC_CHECK_DEBUG))->GetCheck();

	m_bDebugMode = checked ? TRUE : FALSE;

	m_PtCloudWin->SetDebugMode(m_bDebugMode);
}


void CCommonModeNoiseRejectionDlg::OnBnClickedButton2() // x +20
{
	g_CtrlCard.Setup_Speed(1, m_Speed, m_Speed, 500, 500, 5, 2);
	int nPulse = static_cast<int>(double(20) / ACCX + 1);
	g_CtrlCard.Axis_Pmove(AXIS_X, nPulse);
}


void CCommonModeNoiseRejectionDlg::OnBnClickedButton3() // x -20
{
	g_CtrlCard.Setup_Speed(1, m_Speed, m_Speed, 500, 500, 5, 2);
	int nPulse = static_cast<int>(double(-20) / ACCX + 1);
	g_CtrlCard.Axis_Pmove(AXIS_X, nPulse);
}


void CCommonModeNoiseRejectionDlg::OnBnClickedButton4() // y +20
{
	g_CtrlCard.Setup_Speed(2, m_Speed, m_Speed, 500, 500, 5, 2);
	int nPulse = static_cast<int>(double(20) / ACCY + 1);
	g_CtrlCard.Axis_Pmove(AXIS_Y, nPulse);
}


void CCommonModeNoiseRejectionDlg::OnBnClickedButton5() // y -20
{
	g_CtrlCard.Setup_Speed(2, m_Speed, m_Speed, 500, 500, 5, 2);
	int nPulse = static_cast<int>(double(-20) / ACCY + 1);
	g_CtrlCard.Axis_Pmove(AXIS_Y, nPulse);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditInnerSize()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_INNER_SIZE, strText);
	if (strText.IsEmpty())
		return;

	m_InnerSize = (float)_ttof(strText);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditResx()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_RESX, strText);
	if (strText.IsEmpty())
		return;

	m_ResolutionX = _ttoi(strText);

	m_PtCloudWin->SetResX(m_ResolutionX);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditResy()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_RESY, strText);
	if (strText.IsEmpty())
		return;

	m_ResolutionY = _ttoi(strText);

	m_PtCloudWin->SetResY(m_ResolutionY);
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditCntx()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_CNTX, strText);
	if (strText.IsEmpty())
		return;
	int nCnt = _ttoi(strText);
	if (nCnt < 2)
	{
		MessageBox(_T("点数不能小于2！"), _T("Tips"), MB_OK | MB_ICONWARNING);
		return;
	}

	m_CountX = nCnt;
}


void CCommonModeNoiseRejectionDlg::OnEnChangeEditCnty()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_CNTY, strText);
	if (strText.IsEmpty())
		return;

	int nCnt = _ttoi(strText);
	if (nCnt < 2)
	{
		MessageBox(_T("点数不能小于2！"), _T("Tips"), MB_OK | MB_ICONWARNING);
		return;
	}

	m_CountY = nCnt;
}

std::vector<CPoint> CCommonModeNoiseRejectionDlg::GetAllPt()
{
	std::vector<CPoint> all;
	for (const auto& pair : m_mapTrackingIdPos)
	{
		for (const auto& psInfo : pair.second)
		{
			all.emplace_back(psInfo.x, psInfo.y);
		}
	}
	return all;
}


void CCommonModeNoiseRejectionDlg::OnCbnSelchangeComboCoordZero()
{
	m_CoordPos = ((CComboBox*)GetDlgItem(IDC_COMBO_COORD_ZERO))->GetCurSel();
}

CPoint CCommonModeNoiseRejectionDlg::LogicToDevice(const vPoint& cp)
{
	CPoint pt;
	/*switch (m_CoordPos)
	{
	case 0:
	{
		pt.x = static_cast<int>(cp.x);
		pt.y = static_cast<int>(cp.y);
		break;
	}
	case 1:
	{
		pt.x = static_cast<int>(cp.y * (float)m_ResolutionY / m_TpTotalHeight) + 1;
		pt.y = static_cast<int>((float)m_ResolutionX - cp.x * (float)m_ResolutionX / m_TpTotalWidth) + 1;
		break;
	}
	case 2:
	{
		pt.x = static_cast<int>((float)m_ResolutionY - cp.y * (float)m_ResolutionY / m_TpTotalHeight) + 1;
		pt.y = static_cast<int>((float)m_ResolutionX - cp.x * (float)m_ResolutionX / m_TpTotalWidth) + 1;
		break;
	}
	case 3:
		break;
	}*/
	pt.x = static_cast<int>(cp.y * ACCY * (float)m_ResolutionY / m_TpTotalHeight) + 1;
	pt.y = static_cast<int>((float)m_ResolutionX - cp.x * ACCX * (float)m_ResolutionX / m_TpTotalWidth) + 1;
	return pt;
}
