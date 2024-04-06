#include "pch.h"
#include "HFST_LinearTest.h"
#include "../CommonModeNoiseRejectionDlg.h"

namespace HFST
{
	inline void ILinearTest::SetRunStatus(void* pDlg, int theta)
	{
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;

		if (theta % 5)
		{
			((CMFCButton*)pAppDlg->GetDlgItem(IDC_MFCBUTTON_RUN))->SetWindowTextW(_T("测试中..."));
		}
		else
		{
			((CMFCButton*)pAppDlg->GetDlgItem(IDC_MFCBUTTON_RUN))->SetWindowTextW(_T("测试中.."));
		}
	}
	inline void ILinearTest::SetFinishStatus(void* pDlg)
	{
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;

		pAppDlg->m_LinearTProbar.SetPos(0);
		pAppDlg->m_LinearTProbar.ShowWindow(SW_HIDE);
		((CMFCButton*)pAppDlg->GetDlgItem(IDC_MFCBUTTON_RUN))->SetWindowTextW(_T("开始"));
		((CMFCButton*)pAppDlg->GetDlgItem(IDC_MFCBUTTON_RUN))->EnableWindow(TRUE);
		pAppDlg->m_PathTypeList.EnableWindow(TRUE);
		pAppDlg->m_LapCombo.EnableWindow(TRUE);
		pAppDlg->ClearVec();

		((CComboBox*)pAppDlg->GetDlgItem(IDC_COMBO_TP_TYPE))->EnableWindow(TRUE);
		((CEdit*)pAppDlg->GetDlgItem(IDC_EDIT_TP_WIDTH))->EnableWindow(TRUE);
		((CEdit*)pAppDlg->GetDlgItem(IDC_EDIT_TP_HEIGHT))->EnableWindow(TRUE);
		((CEdit*)pAppDlg->GetDlgItem(IDC_EDIT_RANGE))->EnableWindow(TRUE);
	}
	//----------- Spiral_Clockwise_InOut implementation -----------------//
    Spiral_Clockwise_InOut::Spiral_Clockwise_InOut(float nLaps)
        :ILinearTest(nLaps)
    {

    }
    Spiral_Clockwise_InOut::~Spiral_Clockwise_InOut() {}

    void Spiral_Clockwise_InOut::Run(void* pDlg)
    {
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;
		const int nSpeed = pAppDlg->GetSpeed();
		g_CtrlCard.Setup_Speed(1, nSpeed, nSpeed, 500, 500, 5, 2);
		g_CtrlCard.Setup_Speed(2, nSpeed, nSpeed, 500, 500, 5, 2);

		int nUpLimit = int(m_Laps * 360);

		for (int theta = 0; theta < nUpLimit; ++theta)
		{
			SpiralTestHelper( theta, &m_PreX, &m_PreY, size_t( 180 / (pAppDlg->m_nRange)) );
			pAppDlg->DrawPoint(ZOOM * m_PreX, ZOOM * m_PreY);
			pAppDlg->PushBackPoint({ ZOOM * m_PreX,ZOOM * m_PreY });
			pAppDlg->m_LinearTProbar.SetPos( theta * 100 / nUpLimit );
			SetRunStatus(pDlg, theta);
		}

		pAppDlg->BackToTpCenter();

		SetFinishStatus(pDlg);
    }

	//----------- Spiral_Clockwise_OutIn implementation -----------------//
	Spiral_Clockwise_OutIn::Spiral_Clockwise_OutIn(float nLaps)
		:ILinearTest( nLaps )
	{

	}
	Spiral_Clockwise_OutIn::~Spiral_Clockwise_OutIn() {};

	void Spiral_Clockwise_OutIn::Run(void* pDlg)
	{
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;

		const int nSpeed = pAppDlg->GetSpeed();
		g_CtrlCard.Setup_Speed(1, nSpeed, nSpeed, 500, 500, 5, 2);
		g_CtrlCard.Setup_Speed(2, nSpeed, nSpeed, 500, 500, 5, 2);

		int nUpLimit = int(m_Laps * 360);

		for ( int theta = -nUpLimit; theta < 0; ++theta )
		{
			//< out -> in
			//< first we should move to out, in this case ,move Z up first
			//< only in theta ==  -nUpLimit

			if ( theta == -nUpLimit )
			{
				pAppDlg->MoveZUp();
			}
			else if ( theta == -nUpLimit+1 )
			{
				pAppDlg->MoveZDown();
			}

			SpiralTestHelper(theta, &m_PreX, &m_PreY, size_t(180 / (pAppDlg->m_nRange)));
			pAppDlg->DrawPoint( ZOOM * m_PreX, ZOOM * m_PreY);
			pAppDlg->PushBackPoint({ ZOOM * m_PreX, ZOOM * m_PreY });
			pAppDlg->m_LinearTProbar.SetPos( (theta + nUpLimit) * 100 / nUpLimit );

			SetRunStatus(pDlg,theta);
		}
		SetFinishStatus(pDlg);
	}

	//------------ Spiral_UnClockwise_InOut implementation ---------------//
	Spiral_UnClockwise_InOut::Spiral_UnClockwise_InOut(float nLaps)
		:ILinearTest( nLaps )
	{

	}
	Spiral_UnClockwise_InOut::~Spiral_UnClockwise_InOut() {};

	void Spiral_UnClockwise_InOut::Run(void* pDlg)
	{
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;

		const int nSpeed = pAppDlg->GetSpeed();
		g_CtrlCard.Setup_Speed(1, nSpeed, nSpeed, 500, 500, 5, 2);
		g_CtrlCard.Setup_Speed(2, nSpeed, nSpeed, 500, 500, 5, 2);

		int nUpLimit = int(m_Laps * 360);

		for (int theta = 0; theta > -nUpLimit; --theta)
		{
			SpiralTestHelper(theta, &m_PreX, &m_PreY, size_t(180 / (pAppDlg->m_nRange)));
			pAppDlg->DrawPoint(ZOOM * m_PreX, ZOOM * m_PreY);
			pAppDlg->PushBackPoint({ ZOOM * m_PreX,ZOOM * m_PreY });
			pAppDlg->m_LinearTProbar.SetPos( abs(theta) * 100 / nUpLimit );

			SetRunStatus(pDlg,theta);
		}

		pAppDlg->BackToTpCenter();

		SetFinishStatus(pDlg);
	}

	//------------ Spiral_UnClockwise_InOut implementation ---------------//
	Spiral_UnClockwise_OutIn::Spiral_UnClockwise_OutIn(float nLaps)
		:ILinearTest( nLaps )
	{

	}
	Spiral_UnClockwise_OutIn::~Spiral_UnClockwise_OutIn() {}

	void Spiral_UnClockwise_OutIn::Run(void* pDlg)
	{
		CCommonModeNoiseRejectionDlg* pAppDlg = (CCommonModeNoiseRejectionDlg*)pDlg;

		const int nSpeed = pAppDlg->GetSpeed();
		g_CtrlCard.Setup_Speed(1, nSpeed, nSpeed, 500, 500, 5, 2);
		g_CtrlCard.Setup_Speed(2, nSpeed, nSpeed, 500, 500, 5, 2);

		int nUpLimit = int(m_Laps * 360);

		for (int theta = nUpLimit; theta > 0; --theta)
		{
			//< out -> in
			//< first we should move to out, in this case ,move Z up first
			//< only in theta ==  -nUpLimit
			//< else we should move Z down for test

			if (theta == nUpLimit)
			{
				pAppDlg->MoveZUp();
			}
			else if( theta == nUpLimit - 1 )
			{
				pAppDlg->MoveZDown();
			}

			SpiralTestHelper(theta, &m_PreX, &m_PreY, size_t(180 / (pAppDlg->m_nRange)));
			pAppDlg->DrawPoint(ZOOM * m_PreX, ZOOM * m_PreY);
			pAppDlg->PushBackPoint({ ZOOM * m_PreX,ZOOM * m_PreY });
			pAppDlg->m_LinearTProbar.SetPos( (1000 - theta) * 100 / nUpLimit );

			SetRunStatus(pDlg,theta);
		}
		SetFinishStatus(pDlg);
	}

	static void SpiralTestHelper(int theta, double* xPos, double* yPos, size_t nRange)
	{
		TRACE("Running ...theta = %d, [%lf, %lf]\n",theta, *xPos, *yPos);
		double nRealTheta = theta < 0 ? abs(theta) : theta;

		//< calculate X/Y pos
		double x = ((double)A + (double)B * nRealTheta / nRange) * cos(theta * PI / 180);
		double y = ((double)A + (double)B * nRealTheta / nRange) * sin(theta * PI / 180);

		//< calculate pulse value
		long nPulseX = (long)((x - *xPos) / ACCX);
		long nPulseY = (long)((y - *yPos) / ACCY);

		//< Move !
		g_CtrlCard.Axis_Pmove(1, nPulseX);
		g_CtrlCard.Axis_Pmove(2, nPulseY);

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

		//< remember to store the previous X/Y pos
		*xPos = x;
		*yPos = y;
	}
}
