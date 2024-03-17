#include "pch.h"
#include "HFST_LogBox.hpp"

namespace HFST
{
	LogBox::LogBox() {
	}
	
	LogBox::~LogBox() {
	}

	BEGIN_MESSAGE_MAP(LogBox, CListBox)
		ON_WM_DESTROY()
	END_MESSAGE_MAP()

	void LogBox::AppendString(const CString& lpszText, COLORREF fgColor, COLORREF bgColor)
	{
		LogBoxColor* pInfo = new LogBoxColor;

		pInfo->strText = lpszText;
		pInfo->fgColor = fgColor;
		pInfo->bgColor = bgColor;

		SetItemDataPtr( AddString(pInfo->strText), pInfo );
	}

	void LogBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);

		CString strText(_T(""));
		GetText(lpMeasureItemStruct->itemID, strText);
		ASSERT(TRUE != strText.IsEmpty());

		CRect rect;
		GetItemRect(lpMeasureItemStruct->itemID, &rect);

		CDC* pDC = GetDC();
		lpMeasureItemStruct->itemHeight = pDC->DrawText(strText, -1, rect, DT_WORDBREAK | DT_CALCRECT);
		ReleaseDC(pDC);
	}

	void LogBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);

		LogBoxColor* pListBox = (LogBoxColor*)GetItemDataPtr(lpDrawItemStruct->itemID);
		ASSERT(NULL != pListBox);

		CDC dc;

		dc.Attach(lpDrawItemStruct->hDC);

		// Save these value to restore them when done drawing.
		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		// If this item is selected, set the background color 
		// and the text color to appropriate values. Also, erase
		// rect by filling it with the background color.
		if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
			(lpDrawItemStruct->itemState & ODS_SELECTED))
		{
			dc.SetTextColor(pListBox->bgColor);
			dc.SetBkColor(pListBox->fgColor);
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, pListBox->fgColor);
		}
		else
		{
			dc.SetTextColor(pListBox->fgColor);
			dc.SetBkColor(pListBox->bgColor);
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, pListBox->bgColor);
		}

		lpDrawItemStruct->rcItem.left += 5;
		// Draw the text.

		dc.DrawText(pListBox->strText, pListBox->strText.GetLength(), &lpDrawItemStruct->rcItem, DT_WORDBREAK);

		// Reset the background color and the text color back to their
		// original values.
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);

		dc.Detach();
	}

	void LogBox::OnDestroy()
	{
		CListBox::OnDestroy();

		// TODO: Add your message handler code here	
		int nCount = GetCount();
		for (int i = 0; i < nCount; i++)
		{
			LogBoxColor* pList = (LogBoxColor*)GetItemDataPtr(i);
			delete pList;
			pList = NULL;
		}
	}
}


