#ifndef __Pixel_CLIPBOARD_H__
#define __Pixel_CLIPBOARD_H__

class Pixel_Clipboard
{
public:
	static CString GetText();
	static BOOL GetText(LPSTR lpszBuffer, int nBufSize);
	static int GetTextLength();
	static BOOL SetText(LPCTSTR lpszBuffer);
};

#endif //__Pixel_CLIPBOARD_H__