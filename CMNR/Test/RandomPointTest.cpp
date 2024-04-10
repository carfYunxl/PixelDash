#include "pch.h"
#include "RandomPointTest.h"
#include <filesystem>

namespace HFST
{
	void RandomPointTest::Init(CCommonModeNoiseRejectionDlg* pdlg)
	{
		m_pAppDlg = pdlg;
	}

	void RandomPointTest::CreateResultDir()
	{
		// 时间戳
		CString tStamp = CTime::GetCurrentTime().Format("%Y%m%d_%H%M%S");
		USES_CONVERSION;
		std::string strTimeStamp = CT2A(tStamp);

		// 创建结果保存目录
		std::filesystem::path current_path = std::filesystem::current_path() / "RandomPointTest" / strTimeStamp;
		if (!std::filesystem::exists(current_path))
			std::filesystem::create_directories(current_path);

		m_strResultDir = current_path.string();
	}

    void RandomPointTest::CalculateRandomPoint()
    {
		// 文件名
		std::string filename = "RandomPointTest_All.txt";

		std::filesystem::path all_point = std::filesystem::path( m_strResultDir ) / filename;
		m_FileAll.open( all_point, std::ios::out );
		if (!m_FileAll.is_open())
			return;

		std::default_random_engine e( time( 0 ) );

		int nStep = static_cast<double>( round(m_TpHeight / m_SectionNumber) );
		int nSecWidth = m_TpWidth;
		int nSecHeight = nStep;

		int nStartHeight = 0;
		int secCnt = 0;
		int nPointIdx = 0;
		while ( nStartHeight < m_TpHeight )
		{
			std::uniform_int_distribution<unsigned> u_width( 1, m_TpWidth );
			std::uniform_int_distribution<unsigned> u_Hight( nStartHeight + 1, nSecHeight );

			m_FileAll << "//----------------------------------------------------------------\n";
			m_FileAll << "//--  Point " << secCnt * 100 << " - " << secCnt * 100 + 99 << "\n";
			m_FileAll << "//----------------------------------------------------------------\n";

			std::vector<RandomPoint> sVec;
			for (int i = 0; i < m_CountX; ++i)
			{
				for (int j = 0; j < m_CountY; ++j)
				{
					unsigned int x = u_width(e);
					unsigned int y = u_Hight(e);
					if (y > m_TpHeight)
					{
						y = m_TpHeight - 5;
					}

					if (x > m_TpWidth)
					{
						x = m_TpWidth - 5;
					}

					sVec.emplace_back( x, y, nPointIdx );

					m_FileAll << "Point-" << nPointIdx << "\n";
					m_FileAll << "X-tagPt" << nPointIdx << "(mm)\t" << x << std::endl;
					m_FileAll << "Y-tagPt" << nPointIdx << "(mm)\t" << y << std::endl << std::endl;

					nPointIdx++;
				}
			}
			m_vecPoint.push_back(sVec);

			nStartHeight += nStep;
			nSecHeight += nStep;

			secCnt++;
		}

		m_FileAll.flush();
		m_FileAll.close();
    }

	void RandomPointTest::Sort()
	{
		for ( auto& vec : m_vecPoint )
		{
			std::sort(vec.begin(), vec.end(), [](const RandomPoint& p1, const RandomPoint& p2) {
				return p1.x < p2.x;
			});
		}
	}

	void RandomPointTest::TestStep1()
	{
		std::filesystem::path fileFail = std::filesystem::path( m_strResultDir ) / "RandomPointTest_Failed_Step1.txt";
		m_FileStep1.open(fileFail, std::ios::out);
		if (!m_FileStep1.is_open()) {
			return;
		}

		int nLastX = 0;
		int nLastY = 0;

		//将铜柱提起来
		g_CtrlCard.OnButtonPmove(AXIS_Z, -m_TestSafeHeight); //提上来安全高度的距离

		for (size_t i = 0; i < m_vecPoint.size(); ++i)
		{
			const auto& pVec = m_vecPoint.at(i);
			for (size_t j = 0; j < pVec.size(); ++j)
			{
				const auto& point = pVec.at(j);
				int OffsetX = point.x - nLastX;
				int OffsetY = point.y - nLastY;

				//< calculate pulse value
				long nPulseX = (long)(OffsetX / ACCX);
				long nPulseY = (long)(OffsetY / ACCY);

				//< Move !
				g_CtrlCard.Axis_Pmove(1, nPulseX);
				g_CtrlCard.Axis_Pmove(2, nPulseY);

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

				//将铜柱放下
				int ret = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
				if (ret > 0)
					return;

				g_CtrlCard.OnButtonPmove(AXIS_Z, m_TestTouchHeight - g_log);

				//去拿手机的坐标结果, 铜柱停留时间 
				Sleep( m_pAppDlg->m_CmnrCfg.iStopTime );

				//m_pAppDlg->m_FgProgress.SetPos( progPos );

				//将铜柱提起来
				g_CtrlCard.OnButtonPmove(AXIS_Z, -m_TestSafeHeight); //提上来安全高度的距离

				while (!m_pAppDlg->m_dataAnalysis.IsQueueEmpty())
				{
					Sleep(200);
				}

				std::lock_guard< std::mutex > locker( m_pAppDlg->GetMuPos() );
				if ( m_pAppDlg->m_mapTrackingIdPos.empty() && m_pAppDlg->m_rErrorTypeSel.fgNoTouch )
				{
					if ( m_vecStep1Failed.empty() )
					{
						m_FileStep1 << "//----------------------------------------------------------------\n";
						m_FileStep1 << "//--  Failed Point :\n";
						m_FileStep1 << "//----------------------------------------------------------------\n";
					}
					RandomPoint fail;
					fail.x = point.x;
					fail.y = point.y;
					fail.index = i * pVec.size() + j;

					m_vecStep1Failed.push_back(fail);

					m_FileStep1 << "Point-" << fail.index << "\n";
					m_FileStep1 << "X-tagPt" << fail.index << "(mm)\t" << point.x << std::endl;
					m_FileStep1 << "Y-tagPt" << fail.index << "(mm)\t" << point.y << "\n" << std::endl;
				}

				nLastX = point.x;
				nLastY = point.y;

				for ( unsigned int i = 0; i < m_pAppDlg->m_mapTrackingIdPos.size(); i++ )
				{
					if ( m_pAppDlg->m_mapTrackingIdPos[i].size() > 0 )
						m_pAppDlg->m_VecBackLastPos.push_back( m_pAppDlg->m_mapTrackingIdPos[i].back() );
				}

				m_pAppDlg->m_mapTrackingIdPos.clear();
				m_pAppDlg->m_VecBtnStatus.clear();
				m_pAppDlg->m_VecCmnrResult.clear();
			}
		}
		m_FileStep1.flush();
		m_FileStep1.close();
	}

	void RandomPointTest::TestStep2()
	{
		std::filesystem::path sFailFinal = std::filesystem::path( m_strResultDir ) / "RandomPointTest_Failed_Step1.txt";
		m_FileStep2.open(sFailFinal, std::ios::out);
		if ( !m_FileStep2.is_open() )
			return;

		int nLastX = 0;
		int nLastY = 0;

		//测完之后， 继续测失败的点
		for (size_t i = 0; i < m_vecStep1Failed.size(); ++i)
		{
			auto& point = m_vecStep1Failed.at(i);

			int OffsetX = point.x - nLastX;
			int OffsetY = point.y - nLastY;

			//< calculate pulse value
			long nPulseX = (long)(OffsetX / ACCX);
			long nPulseY = (long)(OffsetY / ACCY);

			//< Move !
			g_CtrlCard.Axis_Pmove(1, nPulseX);
			g_CtrlCard.Axis_Pmove(2, nPulseY);

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

			//将铜柱放下
			int ret = g_CtrlCard.Get_CurrentInf(AXIS_Z, g_log, g_act, g_spd);
			if ( ret > 0 )
				return;

			g_CtrlCard.OnButtonPmove(AXIS_Z, m_TestTouchHeight - g_log);
			//去拿手机的坐标结果, 铜柱停留时间 
			Sleep( m_pAppDlg->m_CmnrCfg.iStopTime );

			//m_pAppDlg->m_FgProgress.SetPos(progPos);

			//将铜柱提起来
			g_CtrlCard.OnButtonPmove(AXIS_Z, -m_TestSafeHeight); //提上来安全高度的距离

			while ( !m_pAppDlg->m_dataAnalysis.IsQueueEmpty() ) {
				Sleep(200);
			}

			std::lock_guard< std::mutex > locker( m_pAppDlg->m_muPosOperation );
			if ( m_pAppDlg->m_mapTrackingIdPos.empty() && m_pAppDlg->m_rErrorTypeSel.fgNoTouch )
			{
				if ( m_vecStep2Failed.empty() )
				{
					m_FileStep2 << "//----------------------------------------------------------------\n";
					m_FileStep2 << "//--  Failed Point :\n";
					m_FileStep2 << "//----------------------------------------------------------------\n";
				}
				RandomPoint fl;
				fl.x = point.x;
				fl.y = point.y;
				fl.index = point.index;

				m_FileStep2 << "Point-" << fl.index << "\n";
				m_FileStep2 << "X-tagPt" << fl.index << "(mm)\t" << point.x << std::endl;
				m_FileStep2 << "Y-tagPt" << fl.index << "(mm)\t" << point.y << std::endl;

				m_vecStep2Failed.emplace_back(fl);
			}

			nLastX = point.x;
			nLastY = point.y;

			for (unsigned int i = 0; i < m_pAppDlg->m_mapTrackingIdPos.size(); i++)
			{
				if ( m_pAppDlg->m_mapTrackingIdPos[i].size() > 0 )
					m_pAppDlg->m_VecBackLastPos.push_back(m_pAppDlg->m_mapTrackingIdPos[i].back());
			}

			m_pAppDlg->m_mapTrackingIdPos.clear();
			m_pAppDlg->m_VecBtnStatus.clear();
			m_pAppDlg->m_VecCmnrResult.clear();
		}
		m_FileStep2.flush();
		m_FileStep2.close();
	}

	void RandomPointTest::NormalStart()
	{
		CreateResultDir();

		CalculateRandomPoint();

		Sort();

		m_pAppDlg->m_mapTrackingIdPos.clear();
		m_pAppDlg->m_VecBtnStatus.clear();
		m_pAppDlg->m_VecCmnrResult.clear();

		TestStep1();

		TestStep2();
	}

	void RandomPointTest::ReStart()
	{
		
	}
}
