#ifndef __Pixel_TREE_VIEW_H__
#define __Pixel_TREE_VIEW_H__

class CViewTree : public CTreeCtrl
{
public:
	CViewTree() noexcept;
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
	virtual ~CViewTree();

protected:
	DECLARE_MESSAGE_MAP()
};

#endif //__Pixel_TREE_VIEW_H__
