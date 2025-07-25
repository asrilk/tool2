
#include <iostream> 
#include <vector>

using namespace std;

#pragma once

// RegeditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegeditDlg dialog

enum
{
	TOKEN_REG_INFO,
	TOKEN_REG_KEY,
	TOKEN_REG_ERROR,
	TOKEN_REG_SUCCEED,

	COMMAND_REG_ENUM,
	COMMAND_REG_CREATEKEY,
	COMMAND_REG_DELKEY,
	COMMAND_REG_CREATKEY,
	COMMAND_REG_DELVAL,
	COMMAND_REG_RENAME,
};



enum MYKEY
{
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
enum KEYVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};

class CRegeditDlg : public CDialog
{
	// Construction
public:
	CRegeditDlg(CWnd* pParent, ISocketBase* pIOCPServer, ClientContext* pContext);   // standard constructor
	void OnReceiveComplete();
	void OnReceive();
	static void SetClipboardText(CString Data);

	// Dialog Data
		//{{AFX_DATA(CRegeditDlg)
	enum { IDD = IDD_REGEDIT };
	CListCtrl	m_list;
	CTreeCtrl	m_tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegeditDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnCancel();
	virtual void OnOK();

	//}}AFX_VIRTUAL

// Implementation
protected:
	ClientContext* m_pContext;
	ISocketBase* m_iocpServer;

	CStatusBar  m_wndStatusBar;
	CXTHeaderCtrl   m_heades;


	// Generated message map functions
	//{{AFX_MSG(CRegeditDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandingTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void ModifyDrawStyle(UINT uFlag, BOOL bRemove);
	afx_msg void EnableControls(BOOL bRedraw);
	afx_msg void OnMenuitemTreeNew();
	afx_msg void OnMenuitemTreeDel();
	afx_msg void OnMenuitemRegexit();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuitemRegEdit();
	afx_msg void OnMenuitemRegStr();
	afx_msg void OnMenuitemRegDword();
	afx_msg void OnMenuitemExstr();
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuitemRegDel();
	afx_msg void OnMenuitemTreeCopyname();
	BOOL IsTreeCtrlEditMessage(WPARAM KeyCode);
	HTREEITEM FindItem(HTREEITEM item, CString strText);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult); //开始编辑
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult); //结束编辑
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	HTREEITEM	m_hRoot;
	HTREEITEM	SelectNode;// 用户打开的节点

	CImageList m_ImageList, m_ImageTree;
	static LPCTSTR m_strComputer, m_strDefault;

	// 获取全路径
	CString GetFullPath(HTREEITEM hCurrent);
	BYTE GetFatherPath(CString& FullPath);
	void AddToTree(char* lpBuffer);
	void AddToList(char* lpBuffer);
	bool isEnable;    //控件是否可用
	void EnableCursor(bool bEnable);
	void GetRegTree(NM_TREEVIEW* pNMTreeView);
	void ShowSucceed();

	UINT nFlag;       // 1，删除项  2，新建项   3，删除键  4， 新建项  5，编辑项               
	CString strPath;

	HICON m_hIcon;

	// 获取类型
	BYTE GetEditType(int index);
	bool isEdit;      //是否正在编辑
	CString Key;
	CString Value;
	BYTE  type;
	int index;

	CMenu m_treeMenu;              
	CMenu m_listMenu;				
	BOOL m_bOnClose;
	bool isuser;
	CString g_sSelectStr; //oldname
};

