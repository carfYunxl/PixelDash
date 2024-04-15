#ifndef __HF_PROPERTY_GRID_PROPERTY_HPP__
#define __HF_PROPERTY_GRID_PROPERTY_HPP__

class HF_PropertyGridProperty final : public CMFCPropertyGridProperty
{
public:
	HF_PropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE);
	HF_PropertyGridProperty(
		const CString& strName,
		const COleVariant& varValue,
		LPCTSTR lpszDescr = NULL,
		DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL,
		LPCTSTR lpszEditTemplate = NULL,
		LPCTSTR lpszValidChars = NULL);

	virtual BOOL HasButton() const override;

	~HF_PropertyGridProperty();

	virtual void OnDrawButton(CDC* pDC, CRect rect) override;

	virtual void OnClickName(CPoint point) override;

	virtual BOOL HasValueField() const override;

	virtual void OnDrawName(CDC* pDC, CRect rect) override;

private:
	CRect		m_recButton;
	COLORREF	m_GroupBkColor{ RGB(27,103,160) };
};

#endif //__HF_PROPERTY_GRID_PROPERTY_HPP__
