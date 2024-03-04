#include "ApplicationHeaders.h"

#include "ReadDirectoryChangesPrivate.h"

#include "ReadDirectoryChanges.h"

namespace ReadDirectoryChangesPrivate
{
	CReadChangesRequest::CReadChangesRequest(CReadChangesServer* pServer, const std::wstring& sz, BOOL b, DWORD dw, DWORD size)
	{
		m_pServer = pServer;
		m_dwFlags = dw;
		m_bChildren = b;
		m_DirectoryPath = sz;
		m_hDirectory = 0;

		::ZeroMemory(&m_Overlapped, sizeof(OVERLAPPED));

		m_Overlapped.hEvent = this;

		m_Buffer.resize(size);
		m_BackupBuffer.resize(size);
	}

	CReadChangesRequest::~CReadChangesRequest()
	{
		_ASSERTE(m_hDirectory == NULL);
	}

	bool CReadChangesRequest::operator==(const CReadChangesRequest& other) const
	{
		return (m_DirectoryPath == other.m_DirectoryPath && m_bChildren == other.m_bChildren && m_dwFlags == other.m_dwFlags);
	}

	bool CReadChangesRequest::OpenDirectory()
	{
		if (m_hDirectory)
			return true;

		m_hDirectory = ::CreateFileW(m_DirectoryPath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

		if (m_hDirectory == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		return true;
	}

	void CReadChangesRequest::BeginRead()
	{
		DWORD dwBytes = 0;

		::ReadDirectoryChangesW(m_hDirectory, &m_Buffer[0], (DWORD)m_Buffer.size(), m_bChildren, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME, &dwBytes, &m_Overlapped, &NotificationCompletion);
	}

	//Static
	VOID CALLBACK CReadChangesRequest::NotificationCompletion(DWORD dwErrorCode, DWORD dwNumberofBytesTransferred, LPOVERLAPPED lpOverlapped)
	{
		CReadChangesRequest* pBlock = (CReadChangesRequest*)lpOverlapped->hEvent;

		if (dwErrorCode == ERROR_OPERATION_ABORTED)
		{
			::InterlockedDecrement(&pBlock->m_pServer->m_nOutstandingRequests);
			delete pBlock;
			return;
		}

		_ASSERTE(dwNumberofBytesTransferred >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

		if (!dwNumberofBytesTransferred)
			return;

		pBlock->BackupBuffer(dwNumberofBytesTransferred);

		pBlock->BeginRead();

		pBlock->ProcessNotification();
	}

	void CReadChangesRequest::ProcessNotification()
	{
		char* pBase = (char*)&m_BackupBuffer[0];

		for (;;)
		{
			FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

			std::wstring wstrFilename(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
			fs::path filePath = m_DirectoryPath / wstrFilename;

			LPCWSTR wszFilename = ::PathFindFileNameW(filePath.c_str());
			int len = lstrlenW(wszFilename);
			if (len <= 12 && wcschr(wszFilename, L'~'))
			{
				wchar_t wbuf[MAX_PATH];
				if (::GetLongPathNameW(wstrFilename.c_str(), wbuf, _countof(wbuf)) > 0)
					filePath = wbuf;
			}

			m_pServer->m_pBase->Push(fni.Action, filePath.c_str());

			if (!fni.NextEntryOffset)
				break;
			pBase += fni.NextEntryOffset;
		}
	}
} //namespace ReadDirectoryChangesPrivate