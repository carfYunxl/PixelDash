
#include "pch.h"
#include "TouchDataAnalysis.h"
#include <iostream>

enum {
    VALUE_TYPE_STRING = 0,
    VALUE_TYPE_NUMBER
};

struct {
    LPCTSTR strTouchType;
    unsigned char nDataType;

    unsigned char nValueType;   // string or number
    std::map< CString, unsigned char > mapValueMapping;
} TouchDataFilterType[] = {
    { _T( "ABS_MT_SLOT" )         , DATA_TYPE_SLOT        , VALUE_TYPE_NUMBER },
    { _T( "ABS_MT_TRACKING_ID" )  , DATA_TYPE_TRACKING_ID , VALUE_TYPE_NUMBER },
    { _T( "BTN_TOUCH" )         , DATA_TYPE_TOUCH       , VALUE_TYPE_STRING, {
                                                                             { _T( "DOWN" ), TOUCH_VALUE_DOWN },
                                                                             { _T( "UP" ), TOUCH_VALUE_UP } } },
    { _T( "ABS_MT_POSITION_X" ) , DATA_TYPE_X_POS       , VALUE_TYPE_NUMBER },
    { _T( "ABS_MT_POSITION_Y" ) , DATA_TYPE_Y_POS       , VALUE_TYPE_NUMBER },
    { _T( "MSC_TIMESTAMP" )     , DATA_TYPE_TIME_STAMP  , VALUE_TYPE_NUMBER },
    { _T( "SYN_MT_REPORT" )     , DATA_TYPE_MT_REPORT   , VALUE_TYPE_NUMBER },
    { _T( "SYN_REPORT" )        , DATA_TYPE_REPORT      , VALUE_TYPE_NUMBER }
};

static constexpr unsigned char MAX_TOUCH_DATA_DEQUE_SIZE = 100;

TouchDataAnalysis::TouchDataAnalysis() : m_hThread(NULL)
                                       , m_bStopFlag(FALSE)
                                       , m_fnDataCallback(nullptr)
                                       , m_fnErrorCallback(nullptr)
                                       , m_pUserData(nullptr)
{
    m_dequeTouchData.clear();
}

TouchDataAnalysis::~TouchDataAnalysis()
{
    m_bStopFlag = TRUE;
    while ( m_hThread ) {
        //thread 退不出来。
        MSG msg;
        m_cvTouchData.notify_one();
        DWORD dwRet = WaitForSingleObject(m_hThread, 100);
        if (dwRet != WAIT_TIMEOUT)
            break;

        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        DispatchMessage(&msg);
      /*WaitForSingleObject( m_hThread, INFINITE );*/
        m_hThread = NULL;
    }
}

DWORD WINAPI TouchDataAnalysis::ThreadAnalysisTouchData( LPVOID param )
{
    TouchDataAnalysis * pThis = static_cast<TouchDataAnalysis *>(param);
    do {
        std::unique_lock< std::mutex > locker( pThis->m_muTouchData );

        pThis->m_cvTouchData.wait( locker, [&] { return !pThis->m_bStopFlag && !pThis->m_dequeTouchData.empty(); } );
        if ( pThis->m_bStopFlag ) break;

        std::cout << "//-========================= awake thread! =======================================" << std::endl;
        CString strTouchData = pThis->m_dequeTouchData.front();
        std::cout << "get front" << std::endl;
        pThis->m_dequeTouchData.pop_front();
        std::cout << "m_dequeTouchData size = " << pThis->m_dequeTouchData.size() << std::endl;

        locker.unlock();

        std::lock_guard< std::mutex > lockerFile(pThis->m_muSaveData);
        {
            //文件关闭后thread会跑到这里，需要lock锁住共享资源
            if (pThis->m_FileOpenOk) {
                pThis->m_FileAdbReportInfo.WriteString(strTouchData);
                pThis->m_FileAdbReportInfo.WriteString(_T("\n"));
                pThis->m_FileAdbReportInfo.Flush();
            }
        }
        pThis->AnalysisTouchData( strTouchData );
        std::cout << "\n//=====================================================================//\n" << std::endl;
    } while ( 1 );
    return 0;
}

void TouchDataAnalysis::AnalysisTouchData( const CString & strTouchData )
{
    std::cout << "from AnalysisTouchData( const CString & strTouchData )\n";
    static CString strRemainTouchData = _T( "" );

    // Append last remain data
    CString tmpTouchData = strTouchData;
    if ( !strRemainTouchData.IsEmpty() ) {
        tmpTouchData = strRemainTouchData + strTouchData;
        strRemainTouchData = _T( "" );
    }

    int pos = 0;
    CString strLineData = tmpTouchData.Tokenize( _T("\r\n"), pos );

    int nTouchDataSize = tmpTouchData.GetLength();
    while ( -1 != pos ) {
        int linePos = 0;
        CStringW tmpStr = strLineData.Tokenize( _T( " " ), linePos );

        AnalysisDataOutputFormat fmt{ DATA_TYPE_NONE, TOUCH_VALUE_NONE };
        while ( -1 != linePos ) {
            // find filter type string
            auto itrFinder = std::find_if( std::begin( TouchDataFilterType ), std::end( TouchDataFilterType ), [&]( const auto & val ) {
                return (0 == tmpStr.Compare( val.strTouchType ) );
            } );

            // can't find, take next string
            if ( std::end( TouchDataFilterType ) == itrFinder ) {
                tmpStr = strLineData.Tokenize( _T( " " ), linePos );
                continue;
            }

            tmpStr = strLineData.Tokenize( _T( " \r\n" ), linePos ).Trim();
            if (-1 == linePos || tmpStr.IsEmpty()) break;

            // Find filter type
            fmt.nDataType = itrFinder->nDataType;
            if ( VALUE_TYPE_NUMBER == itrFinder->nValueType ) {
                // value is number
                fmt.nDataValue = _tcstoul( tmpStr.GetString(), nullptr, 16 ); //改成宽字节的转换，不然0xFFFFFFFF会变成0x7FFFFFFF
            } else {
                // value is string
                auto itrValueMapping = itrFinder->mapValueMapping.find( tmpStr );
                if ( itrValueMapping == itrFinder->mapValueMapping.end() ) 
                    m_fnErrorCallback( tmpStr + ( ": This value type not mapping" ), m_pUserData );
                else
                    fmt.nDataValue = itrValueMapping->second;
            }

            break;
        }

        if ( DATA_TYPE_NONE != fmt.nDataType && TOUCH_VALUE_NONE != fmt.nDataValue )
            m_fnDataCallback( fmt, m_pUserData );          
       
        // remain last incomplete string
        if ( pos > nTouchDataSize ) {
            strRemainTouchData = strLineData;
            break;
        }
        strLineData = tmpTouchData.Tokenize(_T("\r\n"), pos); //移下来，这样pos如果大于nTouchDataSize说明没有找到对应的字段，需要拼接
    }
}

int TouchDataAnalysis::Init( pfnAnalysisDataCallback fnDataCallback, pfnAnalysisErrorCallback fnErrorCallback, void * userData )
{
    if ( !fnDataCallback || !fnErrorCallback )
        return -1;

    m_hThread = CreateThread( NULL, 0, ThreadAnalysisTouchData, this, 0, NULL );
    if ( NULL == m_hThread ) return -2;

    m_fnDataCallback = fnDataCallback;
    m_fnErrorCallback = fnErrorCallback;
    m_pUserData = userData;

    return 1;
}

int TouchDataAnalysis::PushData( const char * data, int dataSize )
{
    CString strTouchData = _T( "" );
    strTouchData.Format( _T( "%S" ), data );

    std::lock_guard< std::mutex > locker( m_muTouchData );
    m_dequeTouchData.emplace_back( strTouchData );
    std::cout << "//-========================= push data success! =======================================" << std::endl;
    m_cvTouchData.notify_one();

    if ( m_dequeTouchData.size() > MAX_TOUCH_DATA_DEQUE_SIZE )
        return -1;

    return 1;
}

bool TouchDataAnalysis::IsQueueEmpty()
{
    return m_dequeTouchData.empty();
}

void TouchDataAnalysis::mrSetFreqAmpl(int freq, float ampl)
{
    CString tmpStr;
    tmpStr.Format(_T("\nFreq:%d, Ampl:%0.2f Adb data\n"), freq, ampl);
    m_FileAdbReportInfo.WriteString(tmpStr);
    m_FileAdbReportInfo.Flush();
}

void TouchDataAnalysis::mrCreateFile(CString fileName)
{
    m_FileOpenOk = m_FileAdbReportInfo.Open(fileName, CFile::modeCreate | CFile::modeWrite);
}

void TouchDataAnalysis::mrCloseFile()
{
    std::lock_guard< std::mutex > lockerFile(m_muSaveData);
    m_FileAdbReportInfo.Close();
    m_FileOpenOk = FALSE;
}