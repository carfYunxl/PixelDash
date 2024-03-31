#ifndef __HF_OUTPUT_WND_H__
#define __HF_OUTPUT_WND_H__

#include "HF_ListBox.h"
#include "HF_Header.h"

namespace HF
{
	class HF_OutputWnd : public CDockablePane
	{
		// 构造
	public:
		HF_OutputWnd() noexcept;

		void UpdateFonts();

		// 特性
	protected:
		CMFCTabCtrl	m_wndTabs;

		HF_ListBox m_wndOutputBuild;
		HF_ListBox m_wndOutputDebug;
		HF_ListBox m_wndOutputFind;

		CFont	  m_Font;

	protected:
		void AdjustHorzScroll(CListBox& wndListBox);

		// 实现
	public:
		virtual ~HF_OutputWnd();

	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);

		DECLARE_MESSAGE_MAP()

	public:
		void AddString(LogType type, const CString& str);
	};
}

#endif //__HF_OUTPUT_WND_H__

