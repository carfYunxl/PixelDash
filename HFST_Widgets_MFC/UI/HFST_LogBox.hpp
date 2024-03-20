#ifndef __HFST_LOG_BOX_HPP__
#define __HFST_LOG_BOX_HPP__

namespace HFST
{
	struct LogBoxColor
	{
		CString strText;
		COLORREF fgColor;
		COLORREF bgColor;
		LogBoxColor()
		{
			strText.Empty();
			fgColor = RGB(0, 0, 0);
			bgColor = RGB(255, 255, 255);
		}
	};

	class LogBox : public CListBox
	{
	public:
		LogBox();

	public:
		void AppendString(const CString& lpszText, COLORREF fgColor, COLORREF bgColor);
	public:
	public:
		virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	public:
		virtual ~LogBox();

	protected:
		afx_msg void OnDestroy();

		DECLARE_MESSAGE_MAP()
	};

#endif // __HFST_LOG_BOX_HPP__
}
