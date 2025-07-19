// 工作提示.cpp : DLL插件应用程序的入口点。
//
#include "stdafx.h"

struct plugInfo
{
	char mark[30];		//标记
	TCHAR szAddress[255];  //ip
	DWORD szPort;	//端口
	BOOL IsTcp;
	BOOL RunDllEntryProc;
}MyInfo =
{
"plugmark",
_T("127.0.0.1"),
6669,
1,
0,
};

HANDLE hThread = NULL;

DWORD WINAPI MainThread(LPVOID dllMainThread)
{
	// 显示"工作中"消息框
	MessageBox(NULL, _T("工作中"), _T("工作提示"), MB_OK | MB_ICONINFORMATION);
	return 0;
}

#ifndef _WINDLL

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR szCmdLine, int iCmdShow)
{
	// 隐藏控制台窗口
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	PostThreadMessageA(GetCurrentThreadId(), NULL, 0, 0);
	GetInputState();
	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, 0);	//启动线程
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	Sleep(300);
	return 0;
}
#endif

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (MyInfo.RunDllEntryProc && (hThread == NULL))
		{
			hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, 0);	//启动线程
			WaitForSingleObject(hThread, INFINITE);
		}
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) bool Main(TCHAR * ip, DWORD port, BOOL IsTcp, BOOL RunDllEntryProc)
{
	_tcscpy_s(MyInfo.szAddress, ip);
	MyInfo.szPort = port;
	MyInfo.IsTcp = IsTcp;
	MyInfo.RunDllEntryProc = RunDllEntryProc;
	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, 0);	//启动线程
	WaitForSingleObject(hThread, INFINITE);
	Sleep(300);
	return 0;
}

extern "C" __declspec(dllexport) bool run()
{
	HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, 0);	//启动线程
	WaitForSingleObject(hThread, INFINITE);
	Sleep(300);
	return 0;
}