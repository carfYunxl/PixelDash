#pragma once
#include "TouchDataAnalysis.h"
#include "CErrorTypeSelectDlg.h"
#include "HFST_LinearTest/HFST_LinearTest.h"

#define SUPPORT_TOUCH_MAX_NUM   5

struct CMNRCfg
{
	int iMinFreq;
	int iMaxFreq;
	int iIncFreq;
	float fMinAmpl;
	float fMaxAmpl;
	float fAmpStep;
	int iWaveform;
	int iImped;
	int iDutyCycle;
	int iRound;
	int iInterval;
	//AM modulation
	int iAmIntFreq;
	float fAmDepth;
	//Liner Machine
	float fAxis_X;
	float fAxis_Y;
	float fTouchHigh;
	float fSafeHigh;
	int iStopTime;
};

struct PositionInfo
{
	unsigned int trackID;
	unsigned int x;
	unsigned int y;
	unsigned int timeTouch;
};

struct FreqAmpl
{
    int iFreq;
    float fAmpl;
};

enum class ErrorType 
{
	ERROR_NO_TOUCH = 0,
    ERROR_MORE_TOUCH = 1,
    ERROR_PEN_UP = 2,
	ERROR_JITTER = 3
};

enum class AxisType
{
    AXIS_X_RIGHT = 0,
	AXIS_X_LEFT = 1,
	AXIS_Y_UP = 2,
	AXIS_Y_DOWN = 3,
	AXIS_Z_UP = 4,
	AXIS_Z_DOWN = 5,
};

enum
{
	AXIS_X_NEGATIVE_LIMIT = 0,
	AXIS_X_POSITIVE_LIMIT = 1,
	AXIS_X_STOP0          = 2,
    AXIS_Y_NEGATIVE_LIMIT = 6,
    AXIS_Y_POSITIVE_LIMIT = 7,
    AXIS_Y_STOP0          = 8,
    AXIS_Z_NEGATIVE_LIMIT = 12,
    AXIS_Z_POSITIVE_LIMIT = 13,
    AXIS_Z_STOP0          = 14,
};

struct vPoint
{
	vPoint() = default;
	vPoint(long ix, long iy, int index) : x(ix), y(iy), idx(index) {}
	long x{ 0 };
	long y{ 0 };
	int idx{ 0 };

	bool operator==(const vPoint& vp) const
	{
		return x == vp.x && y == vp.y && idx == vp.idx;
	}

	bool operator!=(const vPoint& vp) const
	{
		return !operator==(vp);
	}
};

using secPointVec = std::vector<vPoint>;		// now size = 100;

enum class TestResult { NONE = 0, NG, PASS };

enum class TestMode {
	DEFAULT = 0,					// 常规Common Mode Noise Rejection测试
	POINTS_1000_RANDOM,				// 1000点随机测试
	POINTS_1000_RANDOM_CONTINUE,	// 1000点断线续打
	POINTS_1000_ACCURACY			// 1000点精准度测试
};

class CPointCloud;

class CCommonModeNoiseRejectionDlg : public CDialogEx
{
public:
	CCommonModeNoiseRejectionDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CCommonModeNoiseRejectionDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMONMODENOISEREJECTION_DIALOG};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage( MSG * pMsg );

	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnHome();
	afx_msg void OnBnClickedMfcbuttonRun();
    afx_msg void OnBnClickedBtnCmnrStart();
	afx_msg void OnBnClickedBtnErrorTypeSel();
	afx_msg void OnCbnSelchangeComboLaps();
	afx_msg void OnCbnSelchangeComboTpType();
    afx_msg void OnCbnSelchangeComboWaveform();
    afx_msg void OnClose();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar * pScrollBar );
	afx_msg void OnLvnItemchangedListPathType(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMCustomdrawSliderSpeed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboSelect();
	afx_msg void OnBnClickedRadioNormalCmnrTest();
	afx_msg void OnBnClickedButtonLoadPointFile();
	afx_msg void OnBnClickedRadio1000Random();
	afx_msg void OnBnClickedCheckAccurate();
	afx_msg void OnBnClickedButtonShowPoint();
	afx_msg void OnEnChangeEditOffsetX();
	afx_msg void OnEnChangeEditOffsetY();
	afx_msg void OnBnClickedCheckAccurate2();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnEnChangeEditInnerSize();
	afx_msg void OnEnChangeEditResx();
	afx_msg void OnEnChangeEditResy();
	afx_msg void OnEnChangeEditCntx();
	afx_msg void OnEnChangeEditCnty();
public:
	void		BackToTpCenter();
	inline void ClearVec() { m_BackPtVec.clear(); }
	UINT		DoCMNRTest();
	void		DrawPoint( double xPos, double yPos );
	static void fnAdbOutputCallback( const char * data, int size, void * LParam );
	static UINT ThreadDoCMNRTest( LPVOID lpParam );
	static void pfnAnalysisDataCallbackFunc( const AnalysisDataOutputFormat & data, void * userData );
	inline void	PushBackPoint(const std::pair<double, double>& point) { m_BackPtVec.emplace_back(point); }
	void		MoveZUp();
	void		MoveZDown();

	inline const int GetSpeed() const { return m_Speed; }

public:
    void    DefaultCmnrConf();
	void	DrawCircleTp();
	void	DrawSpiral(int index);
	void	DrawRectTp();
	void    DrawResultUI(CDC* pDc, COLORREF color, CString result);
	void    EnableLinearOperation(bool bEnable);
	int		GetCheckIndex();
    bool    GetConfig();
	BOOL	Init_CtrlCard();
	void	InitLinearTestListCtl();
	void	InitUI_FuncGen();
	void	InitUI_Linear();
	void	InitUI_LinearTest();
	void    Interp_moveXY();
	void	InitUI_NormalTest();
	BOOL	IsOutOfTpRange(float nLap, int nLimit);
	void    JudgeLimit();
    void    LoadIniFile();
	void    ParseData(unsigned int nTimeStamp, bool bIsBProtocol);
    void    SaveCmnrIni();
    void    SaveCMNRTestReport();
	void    SaveCMNRTestResult();
	void	SetValidLaps( int nLimit);
    int     StartAdbConnect();
	BOOL	SystemBackHome();
	void	UnCheckOtherItem(CListCtrl& listCtrl, int index);
	void	UpdateUIVersion();
	void	InitToolBar();
	const std::mutex& GetMuPos() const { return m_muPosOperation; }
	std::mutex& GetMuPos(){ return m_muPosOperation; }

	void	GetCornerPtFromUI();
public:
	std::vector< std::pair< FreqAmpl, ErrorType > >			m_VecCmnrResult;
	std::map< int, std::vector< PositionInfo > >			m_mapTrackingIdPos;
	HFST_DevToolbox::HFST_Utility_RedirectExeInputOutput	m_adb;

	std::vector< PositionInfo >				m_VecBackLastPos;
	std::vector< PositionInfo >				m_VecPos;
	std::vector< int >						m_VecBtnStatus;
	std::vector<std::pair<double, double>>	m_BackPtVec;

	ULONG_PTR			m_gdiplusToken;
	TekVisaApi			m_TekVisa;
	CMNRCfg				m_CmnrCfg;
	CButton				m_BtnLeftX;
	CButton				m_BtnRightX;
	CButton				m_BtnUpY;
	CButton				m_BtnDownY;
	CButton				m_BtnUpZ;
	CButton				m_BtnDownZ;
	CButton				m_BtnHome;
	CComboBox			m_ComboWaveForm;
	CButton				m_BtnCMNRStart;
	CStatic				m_StaticProMsg;
	CProgressCtrl		m_FgProgress;
	CSliderCtrl			m_SliderDuty;
	CListBox			m_listResult;
	bool				m_StopThread;
	int					m_iTestFreq;
	float				m_fTestAmpl;
	CStdioFile			m_FileCMNRTestResult;
	HICON				m_hIcon;
	CWinThread*			m_CMNRTestThread;
	std::mutex			m_muPosOperation;
	float				m_fPosValueX;
	float				m_fPosValueY;
	float				m_fPosValueZ;
	TouchDataAnalysis	m_dataAnalysis;
	TestResult          m_eTestResult;
	AxisType            m_eAxisType;
	CErrorTypeSelectDlg m_cErrTypeSelDlg;
	ErrorTypeSel        m_rErrorTypeSel;
	CComboBox			m_ComboTpType;
	CRect				mDrawPathArea;
	HFST::ILinearTest*  m_pLinearTest{ nullptr };
	CToolBar			m_ToolBar;
	CImageList			m_ToolBarImage;
	CSliderCtrl			m_SpeedSlide;
	int					m_Speed;
	CEdit				m_Range;
	float				m_TpTotalWidth{172.0f};
	float				m_TpTotalHeight{108.0f};
	int					m_ResolutionX{1280};
	int					m_ResolutionY{800};
	float				m_TpWidth{161.0f};
	float				m_TpHeight{97.0f};
	int					m_SecNumber{10};
	int					m_CountX{50};
	int					m_CountY{2};
	float				m_OffsetX_New{ 4.298f };
	float				m_OffsetY_New{ 50.027f };
	float				m_InnerSize{5.0f};
	BOOL				m_bIsFirstTest{TRUE};

	std::vector<std::pair<float, float>>	m_CornerPts;	// TP four corner points
	std::pair<float, float>					m_CurPt;		// current point
	BOOL									m_bAccuraceEnable{ FALSE };			// 开启精准度判定
	BOOL									m_bDebugMode{ FALSE };				// 开启精准度判定
	TestMode								m_TestMode{ TestMode::DEFAULT };
	int										m_EdgeSize{5};
	float									m_BorderThres{1.5};
	float									m_NonBorderThres{1.0};
	CPointCloud*							m_PtCloudWin{nullptr};
	size_t									mStartIndex{ 0 };
	float									m_nRange;
	std::vector<vPoint>						mAllPoint;
	CComboBox				m_LapCombo;
	CProgressCtrl			m_LinearTProbar;
	CListCtrl				m_PathTypeList;
	int						m_CoordPos{1};
private:
	void LogFailPt(
		std::vector<vPoint>& failPt, 
		std::fstream& ofs, 
		const vPoint& TestPoint,			// 计算坐标，单位mm 
		const CPoint& TestPointDevice,		// 计算坐标，单位像素pixel
		const CPoint& ReportPoint,			// 实际报点坐标，单位pixel
		int index,							// 报点坐标的下标，0~1000
		const CString& strWhy,				// 成功或失败的原因 => PASS / 多报点 / 漏保点 / 不符合精准度
		BOOL bRetry = FALSE					// 是否是NG重测
	);
	
	// mm point convert to pixel point
	CPoint LogicToDevice(const vPoint& cp);

	float Distance(const CPoint& p1, const CPoint& p2) {
		return (float)sqrt( pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) );
	}

	std::vector<CPoint> GetAllPt();

	void MoveXYZ(long x, long y, long z);

	void GenerateTestPoints(std::vector<secPointVec>& PointVec, std::fstream& ofs);

	std::pair<CPoint, float> GetNearestPtAndDistance(
		const std::vector<CPoint>& info,	// 所有的ADB报点坐标
		const CPoint& device				// 计算出的报点坐标
	);

	BOOL IsEdgeArea(long x, long y) {
		return x * ACCX < m_EdgeSize || y * ACCY < m_EdgeSize || x * ACCX > (m_TpWidth - m_EdgeSize) || y * ACCY > (m_TpHeight - m_EdgeSize);
	}
public:
	afx_msg void OnCbnSelchangeComboCoordZero();
};
