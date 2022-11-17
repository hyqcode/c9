
// c9Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "c9.h"
#include "c9Dlg.h"
#include "afxdialogex.h"
#include "obj.h"
#include "tlhelp32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define CountArray(Array) (sizeof(Array) / sizeof(Array[0]))
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cc9Dlg 对话框



Cc9Dlg::Cc9Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_C9_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// 定义一个全局的dm对象
dmsoft* dm = NULL;

//窗口结构体
struct tagGetExeHwndCallbakeData {
	DWORD pid;
	CString strClass;
	CString strTitle;
	HWND* pHwnd;
};


//回调函数

BOOL CALLBACK EnumChildWindowCallBack(HWND hwnd, LPARAM lParam) {

	tagGetExeHwndCallbakeData* pData = (tagGetExeHwndCallbakeData*)lParam;
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hwnd, &dwPid);
	TCHAR szTitle[256] = _T("");
	SendMessage(hwnd, WM_GETTEXT, 256, (LPARAM)szTitle);
	TCHAR szClass[256] = _T("");

	CString strClass, strTitle;
	strClass = _T("MKSEmbedded");
	strTitle = _T("MKSWindow#0");
	if (pData->strClass == strClass && pData->strTitle == strTitle)
	{
		*(pData->pHwnd) = hwnd;
		return TRUE;
	}
	EnumChildWindows(hwnd, EnumChildWindowCallBack, lParam);
	return TRUE;
}



BOOL CALLBACK EnumWindowsCallBack(HWND hwnd, LPARAM lParam) {

	DWORD dwPid = 0;
	GetWindowThreadProcessId(hwnd, &dwPid);
	tagGetExeHwndCallbakeData* pData = (tagGetExeHwndCallbakeData*)lParam;
	if (dwPid == pData->pid)
		EnumChildWindows(hwnd, EnumWindowsCallBack, lParam);
	return TRUE;
}
//根据进程名称查找句柄
HWND GetExeHwnd(CString exeName, CString strClass, CString strTitle)
{
	DWORD pid = 0;
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	do
	{
		if (!Process32First(hSnapshot, &pe))
		{
			return 0;
		}
	} while (_tcscmp(pe.szExeFile, exeName));

	pid = pe.th32DefaultHeapID;

	HWND iRetHwnd = 0;
	tagGetExeHwndCallbakeData* pCallBackData = new tagGetExeHwndCallbakeData;
	pCallBackData->pid = pid;
	pCallBackData->strClass = strClass;
	pCallBackData->strTitle = strTitle;
	pCallBackData->pHwnd = &iRetHwnd;

	EnumWindows(EnumWindowsCallBack, (LPARAM)pCallBackData);

	return *(pCallBackData->pHwnd);

}


//进程目录
bool Cc9Dlg::GetWorkDirectory(TCHAR szWorkDirectory[], WORD wBufferCount)
{
	//模块路径
	TCHAR szModulePath[MAX_PATH] = TEXT("");
	GetModuleFileName(AfxGetInstanceHandle(), szModulePath, CountArray(szModulePath));

	//分析文件
	for (INT i = lstrlen(szModulePath); i > +0; i--)
	{
		if (szModulePath[i] == TEXT('\\'))
		{
			szModulePath[i] = 0;
			break;
		}
	}

	//设置结果
	ASSERT(szModulePath[0] != 0);
	lstrcpyn(szWorkDirectory, szModulePath, wBufferCount);

	return true;
}



void Cc9Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cc9Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &Cc9Dlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// Cc9Dlg 消息处理程序

BOOL Cc9Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//注册大漠插件
	CoInitializeEx(NULL, 0);
	TCHAR szDirectory[MAX_PATH] = TEXT("");
	GetWorkDirectory(szDirectory, CountArray(szDirectory));
	m_strWorkPath = szDirectory;
	m_hDMRegInstance = LoadLibrary(m_strWorkPath + _T("/DmReg.dll"));
	if (m_hDMRegInstance == NULL)
	{
		AfxMessageBox(_T("无法加载Reg.dll"));
		return FALSE;
	}
	TypeSetdllPathW pfnSetDllPthW = (TypeSetdllPathW)GetProcAddress(m_hDMRegInstance, "SetDllPathW");
	if (pfnSetDllPthW == NULL)
	{
		AfxMessageBox(_T("无法获取SetDllPathW"));
		return FALSE;
	}
	CString strDmPath = m_strWorkPath + _T("/dm.dll");
	long iRet = pfnSetDllPthW((WCHAR*)strDmPath.GetString(), 0);
	dm = new dmsoft;
	long dm_ret = dm->Reg(_T("y582058728811eb51b6c2e80ed0c0a14d7400f4687"), _T("1qunp"));
	Sleep(2000);
	if (dm_ret == 1) {
		AfxMessageBox(L"插件注册成功" + dm->Ver());
	}
	else {
		CString str;
		str.Format(_T("插件注册失败!返回值:%d"), dm_ret);
		AfxMessageBox(str);
		delete dm;
		return true;
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cc9Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cc9Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cc9Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Cc9Dlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//查找虚拟机窗口
	HWND vm = ::FindWindow(L"VMUIFrame", L"Windows 7 x64 -15 的克隆1 - VMware Workstation");//主虚拟机句柄


	DWORD vmid = 0;//vm主窗口
	GetWindowThreadProcessId(vm, &vmid);
	if (vmid != 0)
	{
		return;
	}

}
