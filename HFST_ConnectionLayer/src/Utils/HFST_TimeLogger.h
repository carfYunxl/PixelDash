#pragma once
#include <chrono>
#include <fstream>

namespace HFST
{
	namespace SC = std::chrono;
	class TimeLogger
	{
	public:
		TimeLogger();


		void Begin();		// output to console
		void BeginFile();	// output to file

		void LogTime(const char* discription);
		void LogTime(const char* discription, int x, int y);
		
		void End();			// end console
		void EndFile();		// end file

	private:
		SC::high_resolution_clock::time_point m_Start;
		unsigned long long m_TotalTime{0};

		unsigned long m_LogCount{1};
		std::ofstream ostream;
	};
}