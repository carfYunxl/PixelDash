
#ifndef __TOUCHDATAANALYSIS_H__
#define __TOUCHDATAANALYSIS_H__

/**
 * @brief Analysis touch data recv from "adb getevent -l" command
 */
#include <deque>
#include <mutex>
#include <map>

///< Touch analysis data type format
enum { 
    DATA_TYPE_NONE = 0,
    DATA_TYPE_SLOT,
    DATA_TYPE_TRACKING_ID, 
    DATA_TYPE_TOUCH, 
    DATA_TYPE_X_POS, 
    DATA_TYPE_Y_POS,
    DATA_TYPE_TIME_STAMP,
    DATA_TYPE_MT_REPORT, //For A protocol
    DATA_TYPE_REPORT //For B protocol
};

///< Touch value if data type is DATA_TYPE_TOUCH
enum {
    TOUCH_VALUE_NONE = -2,
    TOUCH_VALUE_DOWN = 0,
    TOUCH_VALUE_UP
};

typedef struct _AnalysisDataOutputFormat_ {
    unsigned char   nDataType;      // Reference DATA_TYPE_XX
    unsigned int    nDataValue;     // If DATA_TYPE_TOUCH, reference TOUCH_VALUE_XX
} AnalysisDataOutputFormat;

using pfnAnalysisDataCallback = void (*)(const AnalysisDataOutputFormat & data, void * userData);
using pfnAnalysisErrorCallback = void (*)(LPCTSTR strErrorMessage, void * userData);

class TouchDataAnalysis
{
public:
    TouchDataAnalysis();
    ~TouchDataAnalysis();

    int     Init( pfnAnalysisDataCallback fnDataCallback, pfnAnalysisErrorCallback fnErrorCallback, void * userData );
    int     PushData( const char * data, int dataSize );
    bool    IsQueueEmpty();
    void    mrSetFreqAmpl(int freq, float ampl);
    void    mrCreateFile(CString fileName);
    void    mrCloseFile();			  
protected:
    static DWORD WINAPI ThreadAnalysisTouchData( LPVOID param );
    void    AnalysisTouchData( const CString & strTouchData );

private:
    std::deque<CString>         m_dequeTouchData;   // Save adb original touch data
    std::mutex                  m_muTouchData;      // Sync signal
    std::condition_variable     m_cvTouchData;

    HANDLE                      m_hThread;          // Process touch data thread handle
    BOOL                        m_bStopFlag;        // Stop process touch data flag

    pfnAnalysisDataCallback     m_fnDataCallback;   // Process data call back
    pfnAnalysisErrorCallback    m_fnErrorCallback;  // Process error call back
    void                      * m_pUserData;        // Call back user data
	CStdioFile                  m_FileAdbReportInfo;		
    BOOL                        m_FileOpenOk;
    std::mutex                  m_muSaveData;
};

#endif // __TOUCHDATAANALYSIS_H__
