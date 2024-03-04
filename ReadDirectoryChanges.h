#pragma once

#include "CThreadSafeQueue.h"

#include <string>
#include <utility>

typedef std::pair<DWORD, std::wstring> TDirectoryChangeNotification;

namespace ReadDirectoryChangesPrivate
{
	class CReadChangesServer;
}

class CReadDirectoryChanges
{
public:
	CReadDirectoryChanges(int nMaxChanges = 1000);
	~CReadDirectoryChanges();

	void Init();
	void Terminate();

	void AddDirectory(const std::wstring& wszDirectory, BOOL bWatchSubtree, DWORD dwNotifyFilter, DWORD dwBufferSize = 16384);

	HANDLE GetWaitHandle() { return m_Notifications.GetWaitHandle(); }

	bool Pop(DWORD& dwAction, std::wstring& wstrFilename);

	void Push(DWORD dwAction, const std::wstring& wstrFilename);

	bool CheckOverflow();

	unsigned int GetThreadId() { return m_dwThreadId; }

protected:
	ReadDirectoryChangesPrivate::CReadChangesServer* m_pServer;

	HANDLE m_hThread;

	unsigned int m_dwThreadId;

	CThreadSafeQueue<TDirectoryChangeNotification> m_Notifications;
};