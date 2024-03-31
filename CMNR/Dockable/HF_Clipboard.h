#ifndef __HF_CLIPBOARD_H__
#define __HF_CLIPBOARD_H__

class HF_Clipboard
{
public:
	static CString GetText();
	static BOOL GetText(LPSTR lpszBuffer, int nBufSize);
	static int GetTextLength();
	static BOOL SetText(LPCTSTR lpszBuffer);
};

#endif //__HF_CLIPBOARD_H__