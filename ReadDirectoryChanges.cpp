#include "ApplicationHeaders.h"

#include "ReadDirectoryChangesPrivate.h"

#include "ReadDirectoryChanges.h"

using namespace ReadDirectoryChangesPrivate;

#include <Windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;      // Must be 0x1000.
    LPCSTR szName;      // Pointer to name (in user addr space).
    DWORD  dwThreadID;  // Thread ID (-1=caller thread).
    DWORD  dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
void SetThreadName(DWORD dwThreadID, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning( push )
#pragma warning( disable : 6320 6322 )
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
#pragma warning( pop )
}

CReadDirectoryChanges::CReadDirectoryChanges(int nMaxCount)
    : m_Notifications(nMaxCount)
{
    m_hThread = NULL;
    m_dwThreadId = 0;
    m_pServer = new CReadChangesServer(this);
}

CReadDirectoryChanges::~CReadDirectoryChanges()
{
    Terminate();
    delete m_pServer;
}

void CReadDirectoryChanges::Init()
{
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, CReadChangesServer::ThreadStartProc, m_pServer, 0, &m_dwThreadId);
}

void CReadDirectoryChanges::Terminate()
{
    if (m_hThread)
    {
        ::QueueUserAPC(CReadChangesServer::TerminateProc, m_hThread, (ULONG_PTR)m_pServer);
        ::WaitForSingleObjectEx(m_hThread, 10000, true);
        ::CloseHandle(m_hThread);

        m_hThread = NULL;
        m_dwThreadId = 0;
    }
}

void CReadDirectoryChanges::AddDirectory(const std::wstring& szDirectory, BOOL bWatchSubtree, DWORD dwNotifyFilter, DWORD dwBufferSize)
{
    if (!m_hThread)
        Init();

    if (m_hThread)
    {
        CReadChangesRequest* pRequest = new CReadChangesRequest(m_pServer, szDirectory, bWatchSubtree, dwNotifyFilter, dwBufferSize);
        ::QueueUserAPC(CReadChangesServer::AddDirectoryProc, m_hThread, (ULONG_PTR)pRequest);
    }
}

void CReadDirectoryChanges::Push(DWORD dwAction, const std::wstring& wstrFilename)
{
    m_Notifications.push(TDirectoryChangeNotification(dwAction, wstrFilename));
}

bool CReadDirectoryChanges::Pop(DWORD& dwAction, std::wstring& wstrFilename)
{
    TDirectoryChangeNotification pair;
    if (!m_Notifications.pop(pair))
        return false;

    dwAction = pair.first;
    wstrFilename = pair.second;

    return true;
}

bool CReadDirectoryChanges::CheckOverflow()
{
    bool b = m_Notifications.overflow();
    if (b)
        m_Notifications.clear();
    return b;
}