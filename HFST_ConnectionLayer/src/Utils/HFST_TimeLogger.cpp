#include "pch.h"
#include "HFST_TimeLogger.h"
#include <conio.h>

namespace HFST
{
	TimeLogger::TimeLogger() : m_Start( SC::high_resolution_clock::now() ) {}

	void TimeLogger::Begin()
	{
		m_Start = SC::high_resolution_clock::now();
	}

	void TimeLogger::BeginFile()
	{
		ostream.open("report.txt", std::fstream::out);
		if (!ostream.is_open())
		{
			_cprintf("open file failed!\n");
			return;
		}
		m_Start = SC::high_resolution_clock::now();
	}

	void TimeLogger::LogTime(const char* discription)
	{
		auto dura = SC::duration_cast<SC::milliseconds>( SC::high_resolution_clock::now() - m_Start );
		_cprintf( "%-30s : %ld ms\n", discription, (int)dura.count() );

		m_TotalTime += dura.count();
		m_Start = SC::high_resolution_clock::now();
	}

	void TimeLogger::LogTime(const char* discription, int x, int y)
	{
		auto dura = SC::duration_cast<SC::milliseconds>(SC::high_resolution_clock::now() - m_Start);
		_cprintf("%-30s : %ld ms\n", discription, (int)dura.count());

		m_TotalTime += dura.count();
		m_Start = SC::high_resolution_clock::now();

		auto per = SC::duration_cast<SC::milliseconds>(SC::high_resolution_clock::now() - SC::high_resolution_clock::time_point());
		ostream << m_LogCount << "\t" << x << "," << y << "\t" << per.count() << "\t" << dura.count() << "\n";

		m_LogCount++;
	}

	void TimeLogger::End()
	{
		_cprintf( "%-30s : %ld ms\n", "Total", m_TotalTime );
		_cprintf( "---------------------------------------------\n\n" );
		fflush( stdout );
		m_TotalTime = 0;
		m_LogCount = 0;

		ostream.flush();
		ostream.close();
	}

	void TimeLogger::EndFile()
	{
		m_LogCount = 0;
		ostream.flush();
		ostream.close();
	}
}
