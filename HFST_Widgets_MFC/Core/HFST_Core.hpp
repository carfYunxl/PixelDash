#ifndef __HFST_CORE_HPP__
#define __HFST_CORE_HPP__

#include <windows.h>
namespace HFST
{
    namespace UI
    {
		constexpr int ID_CLOSE = 0x400;
		constexpr int ID_MAX = 0x401;
		constexpr int ID_NORMAL = 0x402;
		constexpr int ID_WIN_TITLE = 0x403;

		const COLORREF GRAY = RGB(0x3E, 0x3E, 0x3E);
		const COLORREF WHITE = RGB(0xFF, 0xFF, 0xFF);
		const COLORREF RED = RGB(0xFF, 0x00, 0x00);

		constexpr int ICON_SIZE = 40;
		constexpr int GAP = 5;
		constexpr int HEIGHT = ICON_SIZE + GAP * 2;

		constexpr int PEN_SIZE = 5;
    }

    static CString GetFileVersion()
    {
        CString strVersion = _T("");
        HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
        if (hRsrc != NULL)
        {
            HGLOBAL hGlobalMemory = LoadResource(NULL, hRsrc);
            if (hGlobalMemory != NULL)
            {
                CString rVersion;
                LPVOID pVersionResouece = LockResource(hGlobalMemory);
                LPVOID pVersion = NULL;
                DWORD uLength, langD;
                BOOL retVal;
                retVal = VerQueryValue(pVersionResouece, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pVersion, (UINT*)&uLength);
                if (retVal && uLength == 4)
                {
                    memcpy(&langD, pVersion, 4);
                    rVersion.Format(_T("\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion"),
                        (langD & 0xff00) >> 8, langD & 0xff, (langD & 0xff000000) >> 24,
                        (langD & 0xff0000) >> 16);
                }
                else
                {
                    rVersion.Format(_T("\\StringFileInfo\\%04X04B0\\FileVersion"), GetUserDefaultLangID());
                }

                if (VerQueryValue(pVersionResouece, rVersion.GetBuffer(0), (LPVOID*)&pVersion, (UINT*)&uLength) != 0)
                {
                    strVersion.Format(_T("%s"), pVersion);
                }
            }
            FreeResource(hGlobalMemory);
        }
        return strVersion;
    }
}

#endif //__HFST_CORE_HPP__
