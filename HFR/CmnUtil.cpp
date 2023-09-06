/*******************************************************************************
*
* Copyright (C) 2013 - 2015, Jeffery Jiang, <china_jeffery@163.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
*******************************************************************************/


#include "CmnUtil.h"

#pragma comment(lib, "shlwapi.lib")
#include <io.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <WinReg.h>

#include <process.h>

#include <WtsApi32.h>
#include <UserEnv.h>
#include <shlobj.h> // for IsUserAnAdmin
#pragma comment (lib,"shell32.lib")
#pragma comment(lib,"WtsApi32.lib")
#pragma comment(lib,"UserEnv.lib")

#define FILE_ATTRIBUTES_FILTER (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)

namespace cpp4j {
    void chMB(PCSTR szMsg) {
        char szTitle[MAX_PATH];
        GetModuleFileNameA(NULL, szTitle, sizeof(szTitle));
        MessageBoxA(GetActiveWindow(), szMsg, szTitle, MB_OK);
    }

    void chFAIL(PSTR szMsg) {
        chMB(szMsg);
        DebugBreak();
    }

    void chASSERTFAIL(LPCSTR file, int line, PCSTR expr) {
        char sz[1024];
        StringCchPrintfA(sz, 1024, "File %s, line %d : %s", file, line, expr);
        chFAIL(sz);
    }

    double RoundEx(double v, int digits) {
        return floor(v * pow(10.f, digits) + 0.5) / pow(10.f, digits);
    }

    BOOL CREATEPROCESS(LPCTSTR szCmdLine, LPPROCESS_INFORMATION lpProcessInfo) {
        TCHAR szCL[MAX_PATH + 1] = {0};
        TCHAR szDir[MAX_PATH + 1] = {0};
        StringCchCopy(szCL, MAX_PATH, szCmdLine);
        StringCchCopy(szDir, MAX_PATH, szCmdLine);
        PathRemoveFileSpec(szDir);
        STARTUPINFO si = {sizeof(si)};
        return CreateProcess(NULL, szCL, NULL, NULL, FALSE, 0, NULL, szDir, &si, lpProcessInfo);
    }

    void TraceMsgW(const wchar_t *lpFormat, ...) {
        if(!lpFormat)
            return;

        wchar_t *pMsgBuffer = NULL;
        unsigned int iMsgBufCount = 0;

        va_list arglist;
        va_start(arglist, lpFormat);
        HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        while(hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;
            if(pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }
            pMsgBuffer = (wchar_t*) malloc(iMsgBufCount * sizeof(wchar_t));
            if(!pMsgBuffer) {
                break;
            }

            hr = StringCchVPrintfW(pMsgBuffer, iMsgBufCount, lpFormat, arglist);
        }
        va_end(arglist);
        if(hr == S_OK) {
            if (pMsgBuffer)
                OutputDebugStringW(pMsgBuffer);
        }

        if(pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }
    }

    void TraceMsgA(const char *lpFormat, ...) {
        if(!lpFormat)
            return;

        char *pMsgBuffer = NULL;
        unsigned int iMsgBufCount = 0;

        va_list arglist;
        va_start(arglist, lpFormat);
        HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        while(hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;
            if(pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }
            pMsgBuffer = (char*) malloc(iMsgBufCount * sizeof(char));
            if(!pMsgBuffer) {
                break;
            }

            hr = StringCchVPrintfA(pMsgBuffer, iMsgBufCount, lpFormat, arglist);
        }
        va_end(arglist);
        if(hr == S_OK) {
            if (pMsgBuffer)
                OutputDebugStringA(pMsgBuffer);
        }

        if(pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }
    }

    std::string GetExeDirA() {
        char szIniPath[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, szIniPath, MAX_PATH);
        PathRemoveFileSpecA(szIniPath);
        PathAddBackslashA(szIniPath);
        return szIniPath;
    }

    std::wstring GetExeDirW() {
        wchar_t szIniPath[MAX_PATH] = {0};
        GetModuleFileNameW(NULL, szIniPath, MAX_PATH);
        PathRemoveFileSpecW(szIniPath);
        PathAddBackslashW(szIniPath);
        return szIniPath;
    }

    bool IsWin64() {
        typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
        static LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
        BOOL bIsWow64 = FALSE;

        if(NULL == fnIsWow64Process) {
            HMODULE h = GetModuleHandleW(L"kernel32");
            if(h)
                fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(h, "IsWow64Process");
        }

        if(NULL != fnIsWow64Process) {
            fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
        }

        return bIsWow64 == 1;
    }

    BOOL CheckSingleInstance(LPCTSTR pszUniqueName) {
        HANDLE hMutex = CreateEvent(NULL, TRUE, FALSE, pszUniqueName);
        DWORD dwLstErr = GetLastError();
        BOOL bOneInstanceCheck = TRUE;

        if(hMutex) {
            if(dwLstErr == ERROR_ALREADY_EXISTS) {
                CloseHandle(hMutex);
                bOneInstanceCheck = FALSE;
            }
        }
        else {
            if(dwLstErr == ERROR_ACCESS_DENIED)
                bOneInstanceCheck = FALSE;
        }

        return bOneInstanceCheck;
    }


#if 0
    CritSec::CritSec() {
        InitializeCriticalSection(&m_CS);
        m_dwCurrentOwner = m_dwLockCount = 0;
        m_fTrace = FALSE;
    }
    CritSec::~CritSec() {
        DeleteCriticalSection(&m_CS);
    }
    void CritSec::Lock() {
        DWORD us = GetCurrentThreadId();
        DWORD currentOwner = m_dwCurrentOwner;
        if(currentOwner && (currentOwner != us)) {
            // already owned, but not by us
            if(m_fTrace) {
                TraceMsg(TEXT("Thread %d about to wait for lock %x owned by %d.\r\n"),
                         GetCurrentThreadId(), &m_CS, currentOwner);
                // if we saw the message about waiting for the critical
                // section we ensure we see the message when we get the
                // critical section
            }
        }
        EnterCriticalSection(&m_CS);
        if(0 == m_dwLockCount++) {
            // we now own it for the first time.  Set owner information
            m_dwCurrentOwner = us;

            if(m_fTrace) {
                TraceMsg(TEXT("Thread %d now owns lock %x.\r\n"), m_dwCurrentOwner, &m_CS);
            }
        }
    }
    void CritSec::Unlock() {
        if(0 == --m_dwLockCount) {
            // about to be unowned
            if(m_fTrace) {
                TraceMsg(TEXT("Thread %d releasing lock %x.\r\n"), m_dwCurrentOwner, &m_CS);
            }

            m_dwCurrentOwner = 0;
        }
        LeaveCriticalSection(&m_CS);
    }

#else
    CritSec::CritSec() {
        InitializeCriticalSection(&m_CS);
    }
    CritSec::~CritSec() {
        DeleteCriticalSection(&m_CS);
    }
    void CritSec::Lock() {
        EnterCriticalSection(&m_CS);
    }
    void CritSec::Unlock() {
        LeaveCriticalSection(&m_CS);
    }

#endif

    Locker::Locker(CritSec *pCS) {
        m_pCS = pCS;
        m_pCS->Lock();
    }

    Locker::~Locker() {
        m_pCS->Unlock();
    }




    std::string UnicodeToAnsi(const std::wstring &str) {
        std::string strRes;
        int iSize = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if(iSize == 0)
            return strRes;

        char *szBuf = new (std::nothrow) char[iSize];
        if(!szBuf)
            return strRes;
        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::wstring AnsiToUnicode(const std::string &str) {
        std::wstring strRes;

        int iSize = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

        if(iSize == 0)
            return strRes;

        wchar_t *szBuf = new (std::nothrow) wchar_t[iSize];
        if(!szBuf)
            return strRes;
        memset(szBuf, 0, iSize * sizeof(wchar_t));

        ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::string UnicodeToUtf8(const std::wstring &str) {
        std::string strRes;

        int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if(iSize == 0)
            return strRes;

        char *szBuf = new (std::nothrow) char[iSize];
        if(!szBuf)
            return strRes;
        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

	std::string UnicodeToUtf8_pach(const std::wstring &str) {
		std::string strRes;

		int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

		if (iSize == 0)
			return strRes;

		char *szBuf = new (std::nothrow) char[iSize+3];
		if (!szBuf)
			return strRes;
		memset(szBuf, 0, iSize+3);

		::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &szBuf[3], iSize, NULL, NULL);
		szBuf[0] = 0xef;
		szBuf[1] = 0xbb;
		szBuf[2] = 0xbf;

		strRes = szBuf;
		delete[] szBuf;

		return strRes;
	}

    std::wstring Utf8ToUnicode(const std::string &str) {
        std::wstring strRes;
        int iSize = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

        if(iSize == 0)
            return strRes;

        wchar_t *szBuf = new (std::nothrow) wchar_t[iSize];
        if(!szBuf)
            return strRes;
        memset(szBuf, 0, iSize * sizeof(wchar_t));
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::string AnsiToUtf8(const std::string &str) {
        return UnicodeToUtf8(AnsiToUnicode(str));
    }


	std::string AnsiToUtf8_pach(const std::string &str) {
		return UnicodeToUtf8_pach(AnsiToUnicode(str));
	}

    std::string Utf8ToAnsi(const std::string &str) {
        return UnicodeToAnsi(Utf8ToUnicode(str));
    }

    std::string _UrlEncode(const std::string &str) {
        char hex[] = "0123456789ABCDEF";
        std::string dst;

        for(size_t i = 0; i < str.size(); ++i) {
            unsigned char cc = str[i];

            if(cc >= 'A' && cc <= 'Z'
               || cc >= 'a' && cc <= 'z'
               || cc >= '0' && cc <= '9'
               || cc == '.'
               || cc == '_'
               || cc == '-'
               || cc == '*'
               || cc == '~') {
                dst += cc;
            }
            else {
                unsigned char c = static_cast<unsigned char>(str[i]);
                dst += '%';
                dst += hex[c / 16];
                dst += hex[c % 16];
            }
        }

        return dst;
    }

    /*
    Operating system                            dwMajorVersion   dwMinorVersion              Other
    Windows 10 Technical Preview                     10               0                  OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
    Windows Server Technical Preview                 10               0                  OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
    Windows 8.1                                      6                3                  OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
    Windows Server 2012 R2                           6                3                  OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
    Windows 8                                        6                2                  OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
    Windows Server 2012                              6                2                  OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
    Windows 7                                        6                1                  OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
    Windows Server 2008 R2                           6                1                  OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
    Windows Server 2008                              6                0                  OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
    Windows Vista                                    6                0                  OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
    Windows Server 2003 R2                           5                2                  GetSystemMetrics(SM_SERVERR2) != 0
    Windows Home Server                              5                2                  OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
    Windows Server 2003                              5                2                  GetSystemMetrics(SM_SERVERR2) == 0
    Windows XP Professional x64 Edition              5                2                  (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) && (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
    Windows XP                                       5                1                  Not applicable
    Windows 2000                                     5                0                  Not applicable
    */
#define VER_SUITE_WH_SERVER                 0x00008000

    OSVersion::OSVersion() {
        memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    }

    // 2003R2, 2003, XP ....
    //
    bool OSVersion::AtMostWin2003R2(void) {
        ____Init();
        return (osvi.dwMajorVersion <= 5 && osvi.dwMinorVersion <= 2);
    }

    // Vista, 7, 8 ....
    //
    bool OSVersion::AtLeastWinVista(void) {
        ____Init();
        return (osvi.dwMajorVersion >= 6);
    }

    void OSVersion::GetMajorMinorBuild(DWORD &dwMajor, DWORD &dwMinor, DWORD &dwBuild) {
        ____Init();
        dwMajor = osvi.dwMajorVersion;
        dwMinor = osvi.dwMinorVersion;
        dwBuild = osvi.dwBuildNumber;
    }


    bool OSVersion::IsWinXPProX64() {
        ____Init();
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        return (osvi.dwMajorVersion == 5 &&
                osvi.dwMinorVersion == 2 &&
                osvi.wProductType == VER_NT_WORKSTATION &&
                sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
    }


    OSVer OSVersion::GetOsVersion(void) {
        ____Init();

        if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
            return WIN_2000;
        else if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
            return WIN_XP;
        else if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
            if(IsWinXPProX64())
                return WIN_XP_PRO;
            else if(GetSystemMetrics(SM_SERVERR2) == 0)
                return WIN_2003;
            else if(osvi.wSuiteMask & VER_SUITE_WH_SERVER)
                return WIN_HOME;
            else if(GetSystemMetrics(SM_SERVERR2) != 0)
                return WIN_2003_R2;
        }
        else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
            if(osvi.wProductType == VER_NT_WORKSTATION)
                return WIN_VISTA;
            else if(osvi.wProductType != VER_NT_WORKSTATION)
                return WIN_2008;
        }
        else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
            if(osvi.wProductType == VER_NT_WORKSTATION)
                return WIN_7;
            else if(osvi.wProductType != VER_NT_WORKSTATION)
                return WIN_2008_R2;
        }
        else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2) {
            if(osvi.wProductType == VER_NT_WORKSTATION)
                return WIN_8;
            else if(osvi.wProductType != VER_NT_WORKSTATION)
                return WIN_2012;
        }
        else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3) {
            if(osvi.wProductType == VER_NT_WORKSTATION)
                return WIN_8_1;
            else if(osvi.wProductType != VER_NT_WORKSTATION)
                return WIN_2012_R2;
        }
        else if(osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 3) {
            if(osvi.wProductType == VER_NT_WORKSTATION)
                return WIN_10_PRE;
            else if(osvi.wProductType != VER_NT_WORKSTATION)
                return WIN_SERVER_PRE;
        }

        return WIN_UNKNOWN;
    }

    std::string OSVersion::GetOsVerSummary(void) {
        ____Init();
        char szBuf[100] = {0};
        StringCchPrintfA(szBuf, 100, "build:%d.%d.%d type:%d", osvi.dwMajorVersion,
                         osvi.dwMinorVersion,
                         osvi.dwBuildNumber,
                         osvi.wProductType
                         );
        return szBuf;
    }


    void OSVersion::____Init() {
        if(osvi.dwMajorVersion == 0) {
            GetVersionEx((LPOSVERSIONINFO) &osvi);
        }
    }




    ThreadTimer::ThreadTimer() {
        m_hTimer = NULL;
        m_pTimer = NULL;
    }

    ThreadTimer::~ThreadTimer() {

    }



    void CALLBACK ThreadTimer::TimerProc(void *param, BOOLEAN timerCalled) {
        UNREFERENCED_PARAMETER(timerCalled);
        ThreadTimer *timer = static_cast<ThreadTimer *>(param);

        timer->OnTimedEvent();
    }

    BOOL ThreadTimer::Start(DWORD ulInterval,  // ulInterval in ms
                            BOOL bImmediately,
                            BOOL bOnce) {
        BOOL bRet = FALSE;
        if(!m_hTimer) {
            bRet = CreateTimerQueueTimer(&m_hTimer,
                                         NULL,
                                         TimerProc,
                                         (PVOID)this,
                                         bImmediately ? 0 : ulInterval,
                                         bOnce ? 0 : ulInterval,
                                         WT_EXECUTELONGFUNCTION);
        }

        return bRet;
    }

    void ThreadTimer::Stop(bool bWait) {
        if(m_hTimer) {
            DeleteTimerQueueTimer(NULL, m_hTimer, bWait ? INVALID_HANDLE_VALUE : NULL);
            m_hTimer = NULL;
        }
    }

    void ThreadTimer::OnTimedEvent() {
    }




    RegKey::RegKey(void) : m_hkeyRoot(NULL),
        m_hkey(NULL),
        m_hChangeEvent(NULL),
        m_hNotifyThr(NULL),
        m_bWatchSubtree(false),
        m_dwChangeFilter(0) {
    }

    RegKey::RegKey(HKEY hkeyRoot, LPCTSTR pszSubKey) : m_hkeyRoot(hkeyRoot),
        m_hkey(NULL),
        m_hChangeEvent(NULL),
        m_hNotifyThr(NULL),
        m_bWatchSubtree(false),
        m_dwChangeFilter(0),
        m_strSubKey(pszSubKey) {
    }

    RegKey::~RegKey(void) {
        Close();

        if(NULL != m_hChangeEvent)
            CloseHandle(m_hChangeEvent);
    }

    HRESULT RegKey::Open(REGSAM samDesired, bool bCreate) {
        DWORD dwResult = ERROR_SUCCESS;
        Close();

        if(bCreate) {
            DWORD dwDisposition;
            dwResult = RegCreateKeyEx(m_hkeyRoot,
                                      (LPCTSTR) m_strSubKey.c_str(),
                                      0,
                                      NULL,
                                      0,
                                      samDesired,
                                      NULL,
                                      &m_hkey,
                                      &dwDisposition);
        }
        else {
            dwResult = RegOpenKeyEx(m_hkeyRoot,
                                    (LPCTSTR) m_strSubKey.c_str(),
                                    0,
                                    samDesired,
                                    &m_hkey);
        }

        return HRESULT_FROM_WIN32(dwResult);
    }

    bool RegKey::IsOpen(void) const {
        return NULL != m_hkey;
    }

    HKEY RegKey::GetHandle(void) const {
        return m_hkey;
    }

    void RegKey::Attach(HKEY hkey) {
        Close();
        m_strSubKey.clear();

        m_hkeyRoot = NULL;
        m_hkey = hkey;
    }

    void RegKey::Detach(void) {
        m_hkey = NULL;
    }

    void RegKey::Close(void) {
        if(NULL != m_hkey) {
            HKEY hkeyTemp = m_hkey;
            m_hkey = NULL;
            RegCloseKey(hkeyTemp);
        }
        if(m_hNotifyThr) {
            WaitForSingleObject(m_hNotifyThr, INFINITE);
        }
        if(m_hNotifyThr) {
            CloseHandle(m_hNotifyThr);
            m_hNotifyThr = NULL;
        }
    }

    HRESULT RegKey::WatchForChange(DWORD dwChangeFilter, bool bWatchSubtree) {
        HRESULT hr = E_FAIL;

        if(NULL != m_hChangeEvent || NULL == m_hkey)
            return E_FAIL;

        m_hChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if(NULL == m_hChangeEvent) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        m_dwChangeFilter = dwChangeFilter;
        m_bWatchSubtree = bWatchSubtree;

        unsigned int uThreadId = 0;
        m_hNotifyThr = (HANDLE) _beginthreadex(NULL,
                                      0,
                                      NotifyWaitThreadProc,
                                      this,
                                      0,
                                      &uThreadId);

        if(NULL != m_hNotifyThr) {
            hr = NOERROR;
        }

        return hr;
    }

    HRESULT RegKey::WaitForChange(DWORD dwChangeFilter, bool bWatchSubtree) {
        HRESULT hr = NOERROR;
        LONG lResult = RegNotifyChangeKeyValue(m_hkey,
                                               bWatchSubtree,
                                               dwChangeFilter,
                                               NULL,
                                               FALSE);

        if(ERROR_SUCCESS != lResult) {
            hr = HRESULT_FROM_WIN32(lResult);
        }

        return hr;
    }

    bool RegKey::DeleteKey(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, bool bPrefer64View) {
        HKEY hSubKey = NULL;

        if(pszSubKey) {
            REGSAM rsam = KEY_READ | KEY_WRITE;

            if(bPrefer64View)
                rsam |= KEY_WOW64_64KEY;

            RegOpenKeyEx(hKey, pszSubKey, 0, rsam, &hSubKey);
        }
        else {
            hSubKey = hKey;
        }

        if(hSubKey) {
            if(pszValName) {
                if(RegDeleteValue(hSubKey, pszValName) == ERROR_SUCCESS) {
                    if(hSubKey != hKey) {
                        if(hSubKey)
                            RegCloseKey(hSubKey);
                    }

                    return true;
                }
            }
            else {
                if(RegDeleteSubKeys(hSubKey, bPrefer64View)) {
                    if(hSubKey != hKey) {
                        if(hSubKey)
                            RegCloseKey(hSubKey);
                    }

                    return RegDeleteKey32_64(hKey, pszSubKey, bPrefer64View);
                }
            }

            if(hSubKey != hKey) {
                if(hSubKey)
                    RegCloseKey(hSubKey);
            }
        }

        return false;
    }

    HRESULT RegKey::GetDWORDValue(LPCTSTR pszValueName, DWORD *pdwDataOut) const {
        return GetValue(pszValueName, REG_DWORD, (LPBYTE) pdwDataOut, sizeof(DWORD));
    }

    HRESULT RegKey::GetBINARYValue(LPCTSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const {
        return GetValue(pszValueName, REG_BINARY, pbDataOut, cbDataOut);
    }

    HRESULT RegKey::GetSZValue(LPCTSTR pszValueName, OUT tstring &strValue) const {
        HRESULT hr = E_FAIL;
        int cb = GetValueBufferSize(pszValueName);

        if(cb <= 0)
            return hr;

        TCHAR *szTemp = new TCHAR[cb / sizeof(TCHAR)];
        memset(szTemp, 0, cb);

        if(NULL != szTemp && 0 < cb) {
            hr = GetValue(pszValueName, REG_SZ, (LPBYTE) szTemp, cb);
        }

        strValue = szTemp;
        SAFE_DELETE_ARRAY(szTemp);

        return hr;
    }

    HRESULT RegKey::GetMultiSZValue(LPCTSTR pszValueName, OUT std::vector<tstring> &vStrValues) const {
        HRESULT hr = E_FAIL;
        int cb = GetValueBufferSize(pszValueName);
        TCHAR *szTemp = new TCHAR[cb / sizeof(TCHAR)];
        TCHAR *szBegin = szTemp;

        if(NULL != szTemp && 0 < cb) {
            hr = GetValue(pszValueName, REG_MULTI_SZ, (LPBYTE) szTemp, cb);

            if(SUCCEEDED(hr)) {
                while(szTemp && TEXT('\0') != *szTemp) {
                    vStrValues.push_back(tstring(szTemp));
                    szTemp += lstrlen(szTemp) + 1;
                }
            }
        }

        SAFE_DELETE_ARRAY(szBegin);

        return hr;
    }

    int RegKey::GetValueBufferSize(LPCTSTR pszValueName) const {
        DWORD dwType;
        int cbData = 0;
        DWORD dwResult = RegQueryValueEx(m_hkey,
                                         pszValueName,
                                         0,
                                         &dwType,
                                         NULL,
                                         (LPDWORD) &cbData);
        return cbData;
    }

    HRESULT RegKey::SetDWORDValue(LPCTSTR pszValueName, DWORD dwData) {
        return SetValue(pszValueName, REG_DWORD, (const LPBYTE) &dwData, sizeof(dwData));
    }

    HRESULT RegKey::SetBINARYValue(LPCTSTR pszValueName, const LPBYTE pbData, int cbData) {
        return SetValue(pszValueName, REG_BINARY, pbData, cbData);
    }

    HRESULT RegKey::SetSZValue(LPCTSTR pszValueName, const tstring &strData) {
        return SetValue(pszValueName, REG_SZ, (const LPBYTE) strData.c_str(), (strData.length()) * sizeof(TCHAR));
    }

    HRESULT RegKey::SetMultiSZValue(LPCTSTR pszValueName, const std::vector<tstring> &vStrValues) {
        TCHAR *ptrValues = CreateDoubleNulTermList(vStrValues);
        int cch = 1;
        int n = vStrValues.size();

        for(int i = 0; i < n; i++)
            cch += vStrValues[i].length() + 1;

        HRESULT hr = SetValue(pszValueName, REG_MULTI_SZ, (const LPBYTE) ptrValues, cch * sizeof(TCHAR));

        SAFE_DELETE_ARRAY(ptrValues);

        return hr;
    }


    void RegKey::OnChange(HKEY hkey) {
        UNREFERENCED_PARAMETER(hkey);
        //
        // Default does nothing.
        //
    }



    HRESULT RegKey::GetValue(LPCTSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const {
        DWORD dwType;
        HRESULT hr = RegQueryValueEx(m_hkey,
                                         pszValueName,
                                         0,
                                         &dwType,
                                         pbData,
                                         (LPDWORD) &cbData);

        if(ERROR_SUCCESS == hr && dwType != dwTypeExpected)
            hr = ERROR_INVALID_DATATYPE;

        return hr;
    }

    HRESULT RegKey::SetValue(LPCTSTR pszValueName, DWORD dwValueType, const LPBYTE pbData, int cbData) {
        HRESULT hr = RegSetValueEx(m_hkey,
                                       pszValueName,
                                       0,
                                       dwValueType,
                                       pbData,
                                       cbData);

        return hr;
    }

    LPTSTR RegKey::CreateDoubleNulTermList(const std::vector<tstring> &vStrValues) const {
        size_t cEntries = vStrValues.size();
        size_t cch = 1; // Account for 2nd null terminate.

        for(size_t i = 0; i < cEntries; i++)
            cch += vStrValues[i].length() + 1;

        LPTSTR pszBuf = new TCHAR[cch];
        LPTSTR pszWrite = pszBuf;

        for(size_t i = 0; i < cEntries; i++) {
            const tstring &s = vStrValues[i];
            StringCchCopy(pszWrite, cch, s.c_str());
            pszWrite += s.length() + 1;
        }

        *pszWrite = TEXT('\0'); // Double null terminate.
        return pszBuf;
    }

    unsigned int _stdcall RegKey::NotifyWaitThreadProc(LPVOID pvParam) {
        RegKey *pThis = (RegKey *) pvParam;

        while(NULL != pThis->m_hkey) {
            LONG lResult = RegNotifyChangeKeyValue(pThis->m_hkey,
                                                   pThis->m_bWatchSubtree,
                                                   pThis->m_dwChangeFilter,
                                                   pThis->m_hChangeEvent,
                                                   true);

            if(ERROR_SUCCESS != lResult) {
                return 0;
            }
            else {
                switch(WaitForSingleObject(pThis->m_hChangeEvent, INFINITE)) {
                    case WAIT_OBJECT_0:
                        if(NULL != pThis->m_hkey) {
                            pThis->OnChange(pThis->m_hkey);
                        }

                        break;

                    case WAIT_FAILED:
                        break;

                    default:
                        break;
                }
            }
        }

        return 0;
    }

    bool RegKey::RegDeleteKey32_64(HKEY hKey, LPCTSTR pszSubKey, bool bPrefer64View) {
        REGSAM rsam = (bPrefer64View && IsWin64()) ? KEY_WOW64_64KEY : KEY_WOW64_32KEY;
        HMODULE hAdvAPI32 = LoadLibrary(TEXT("AdvAPI32.dll"));

        if(!hAdvAPI32)
            return false;

        LSTATUS ls;
        typedef LONG(WINAPI * PFN_RegDeleteKeyEx)(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
#if (defined UNICODE) || (defined _UNICODE)
#define RegDeleteKeyExFuncName "RegDeleteKeyExW"
#else
#define RegDeleteKeyExFuncName "RegDeleteKeyExA"
#endif
        PFN_RegDeleteKeyEx _RegDeleteKeyEx = (PFN_RegDeleteKeyEx) GetProcAddress(hAdvAPI32, RegDeleteKeyExFuncName);

        if(_RegDeleteKeyEx) {
            ls = _RegDeleteKeyEx(hKey, pszSubKey, rsam, 0);
            FreeLibrary(hAdvAPI32);
        }
        else {
            ls = RegDeleteKey(hKey, pszSubKey);
        }

        return (ls == ERROR_SUCCESS);
    }

    bool RegKey::RegDeleteSubKeys(HKEY hKey, bool bPrefer64View) {
        DWORD dwSubKeyCnt, dwMaxSubKey;
        const int iMaxKeySize = 256;

        if(RegQueryInfoKey(hKey, NULL, NULL, 0, &dwSubKeyCnt, &dwMaxSubKey, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            if(dwSubKeyCnt) {
                dwMaxSubKey += sizeof(TCHAR);
                TCHAR szKeyName[iMaxKeySize] = {0};

                do {
                    if(RegEnumKey(hKey, --dwSubKeyCnt, szKeyName, iMaxKeySize) == ERROR_SUCCESS) {
                        HKEY hSubKey = NULL;

                        if(ERROR_SUCCESS == RegOpenKeyEx(hKey, szKeyName, 0, KEY_READ | KEY_WRITE, &hSubKey) && hSubKey) {
                            if(RegDeleteSubKeys(hSubKey, bPrefer64View)) {
                                if(hSubKey)
                                    RegCloseKey(hSubKey);

                                if(!RegDeleteKey32_64(hKey, szKeyName, bPrefer64View))
                                    return false;
                            }
                            else {
                                if(hSubKey)
                                    RegCloseKey(hSubKey);

                                return false;
                            }
                        }
                        else {
                            return false;
                        }
                    }
                    else {
                        return false;
                    }
                } while(dwSubKeyCnt);
            }

            return true;
        }

        return false;
    }



#if 1
#if (defined DEBUG) || (defined _DEBUG)
#pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    void __SetThreadName(DWORD dwThreadID, const char *szThreadName) {
        if(!szThreadName)
            return;

        const DWORD MS_VC_EXCEPTION = 0x406D1388;
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = szThreadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        __try {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *) &info);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            ;
        }

        char szMsgBuf[512] = {0};
        sprintf_s(szMsgBuf, 511, "<< Thread >> ID: %ld, Name: %s.\n", dwThreadID, szThreadName);
        OutputDebugStringA(szMsgBuf);
    }
#endif
#endif
    HANDLE CREATETHREAD(void *pSecurity,
                        unsigned uiStackSize,
                        unsigned(__stdcall *pfnStartAddr) (void *),
                        void *pvParam,
                        unsigned uiCreateFlag,
                        unsigned *puiThreadID,
                        const char *szThreadName) {
        unsigned uiThreadID = 0;

        HANDLE hThread = (HANDLE) _beginthreadex(pSecurity, uiStackSize, pfnStartAddr, pvParam, uiCreateFlag, &uiThreadID);

        if(puiThreadID)
            *puiThreadID = uiThreadID;
#if 1
        if(hThread == NULL)
            return hThread;
#if (defined DEBUG) || (defined _DEBUG)
        __SetThreadName(uiThreadID, szThreadName);
#endif
#endif
        return hThread;
    }



    ProcessFinder::ProcessFinder(DWORD dwFlags /* = 0*/, DWORD dwProcessID /* = 0*/) {
        m_hSnapShot = INVALID_HANDLE_VALUE;
        CreateSnapShot(dwFlags, dwProcessID);
    }

    ProcessFinder::~ProcessFinder() {
        if(m_hSnapShot != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hSnapShot);
            m_hSnapShot = INVALID_HANDLE_VALUE;
        }
    }


    BOOL ProcessFinder::CreateSnapShot(DWORD dwFlag, DWORD dwProcessID) {
        if(m_hSnapShot != INVALID_HANDLE_VALUE)
            CloseHandle(m_hSnapShot);

        if(dwFlag == 0)
            m_hSnapShot = INVALID_HANDLE_VALUE;
        else
            m_hSnapShot = CreateToolhelp32Snapshot(dwFlag, dwProcessID);

        return (m_hSnapShot != INVALID_HANDLE_VALUE);
    }


    BOOL ProcessFinder::ProcessFirst(PPROCESSENTRY32 ppe) const {
        BOOL fOk = Process32Next(m_hSnapShot, ppe);
        if(fOk && (ppe->th32ProcessID == 0))
            fOk = ProcessNext(ppe); // remove the "[System Process]" (PID = 0)

        return fOk;
    }

    BOOL ProcessFinder::ProcessNext(PPROCESSENTRY32 ppe) const {
        BOOL fOk = Process32Next(m_hSnapShot, ppe);

        if(fOk && (ppe->th32ProcessID == 0))
            fOk = ProcessNext(ppe); // remove the "[System Process]" (PID = 0)

        return fOk;
    }

    BOOL ProcessFinder::ProcessFind(DWORD dwProcessId, PPROCESSENTRY32 ppe) const {
        BOOL fFound = FALSE;

        for(BOOL fOk = ProcessFirst(ppe); fOk; fOk = ProcessNext(ppe)) {
            fFound = (ppe->th32ProcessID == dwProcessId);

            if(fFound)
                break;
        }

        return fFound;
    }

    BOOL ProcessFinder::ProcessFind(PCTSTR pszExeName, PPROCESSENTRY32 ppe) const {
        BOOL  fFound = FALSE;

        for(BOOL fOk = ProcessFirst(ppe); fOk; fOk = ProcessNext(ppe)) {
            fFound = lstrcmpi(ppe->szExeFile, pszExeName) == 0;

            if(fFound)
                break;
        }

        return fFound;
    }

    BOOL ProcessFinder::ModuleFirst(PMODULEENTRY32 pme) const {
        return (Module32First(m_hSnapShot, pme));
    }

    BOOL ProcessFinder::ModuleNext(PMODULEENTRY32 pme) const {
        return (Module32Next(m_hSnapShot, pme));
    }

    BOOL ProcessFinder::ModuleFind(PVOID pvBaseAddr, PMODULEENTRY32 pme) const {
        BOOL fFound = FALSE;

        for(BOOL fOk = ModuleFirst(pme); fOk; fOk = ModuleNext(pme)) {
            fFound = (pme->modBaseAddr == pvBaseAddr);

            if(fFound)
                break;
        }

        return fFound;
    }

    BOOL ProcessFinder::ModuleFind(PTSTR pszModName, PMODULEENTRY32 pme) const {
        BOOL fFound = FALSE;

        for(BOOL fOk = ModuleFirst(pme); fOk; fOk = ModuleNext(pme)) {
            fFound = (lstrcmpi(pme->szModule, pszModName) == 0) ||
                (lstrcmpi(pme->szExePath, pszModName) == 0);

            if(fFound)
                break;
        }

        return fFound;
    }




    BOOL RunAsAdministrator(LPCTSTR szCommand, LPCTSTR szArgs, BOOL bWaitProcess) {
        SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
        sei.lpVerb = TEXT("runas");
        sei.lpFile = szCommand;
        sei.lpParameters = szArgs;
        sei.nShow = SW_SHOWNORMAL;

        BOOL ret = ShellExecuteEx(&sei);

        if(ret && bWaitProcess)
            WaitForSingleObject(sei.hProcess, INFINITE);

        return ret;
    }

    BOOL CreateProcessInCurrentSession(PCTSTR pszFilePath) {
        HANDLE hUserToken = NULL;
        HANDLE hUserTokenDup = NULL;
        HANDLE hPToken = NULL;
        HANDLE hProcess = NULL;


        DWORD active_session_id = WTSGetActiveConsoleSessionId();

        // 查找winlogon.exe进程，判断其是否在当前会话
        ProcessFinder ph(TH32CS_SNAPPROCESS);
        PROCESSENTRY32 pe = {sizeof(pe)};

        if(!ph.ProcessFind(TEXT("winlogon.exe"), &pe)) {
            return FALSE;
        }

        DWORD winlogon_process_id = pe.th32ProcessID;

        if(winlogon_process_id == 0)
            return FALSE;

        DWORD winlogon_session_id = 0;
        ProcessIdToSessionId(winlogon_process_id, &winlogon_session_id);

        if(winlogon_session_id != active_session_id)
            return FALSE;


        WTSQueryUserToken(active_session_id, &hUserToken);


        TOKEN_PRIVILEGES tp;
        LUID luid;
        hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogon_process_id);

        if(!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
            | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
            | TOKEN_READ | TOKEN_WRITE, &hPToken))
            return FALSE;

        if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
            return FALSE;

        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup);

        //Adjust Token privilege
        SetTokenInformation(hUserTokenDup, TokenSessionId, (void *) active_session_id, sizeof(DWORD));

        if(!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, NULL))
            return FALSE;

        if(GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            return FALSE;
        }

        DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
        LPVOID pEnv = NULL;

        if(CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE))
            dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
        else
            pEnv = NULL;

        PROCESS_INFORMATION pi;
        STARTUPINFO si;

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.lpDesktop = L"winsta0\\default";
        ZeroMemory(&pi, sizeof(pi));

        BOOL ret = CreateProcessAsUserW(
            hUserTokenDup,            // client's access token
            pszFilePath,         // file to execute
            NULL,              // command line
            NULL,              // pointer to process SECURITY_ATTRIBUTES
            NULL,              // pointer to thread SECURITY_ATTRIBUTES
            FALSE,             // handles are not inheritable
            dwCreationFlags,  // creation flags
            pEnv,              // pointer to new environment block
            NULL,              // name of current directory
            &si,               // pointer to STARTUPINFO structure
            &pi                // receives information about new process
            );


        if(hProcess)
            CloseHandle(hProcess);

        if(hUserToken)
            CloseHandle(hUserToken);

        if(hUserTokenDup)
            CloseHandle(hUserTokenDup);

        if(hPToken)
            CloseHandle(hPToken);

        SAFE_CLOSE(pi.hThread);
        SAFE_CLOSE(pi.hProcess);

        return (ret == TRUE);
    }

    BOOL EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable) {
        BOOL fOk = FALSE;
        HANDLE hToken = NULL;

        if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            LookupPrivilegeValue(NULL, szPrivilege, &tp.Privileges[0].Luid);
            tp.Privileges->Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
            fOk = (GetLastError() == ERROR_SUCCESS);

            CloseHandle(hToken);
        }

        return fOk;
    }

    BOOL CheckProcessUserIsAdmin(BOOL *pIsAdmin) {
        BOOL bResult = FALSE;
        HANDLE hToken = FALSE;
        DWORD dwSize;

        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return FALSE;

        TOKEN_ELEVATION_TYPE elevationType;

        if(GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize)) {
            if(elevationType == TokenElevationTypeLimited) {
                byte adminSID[SECURITY_MAX_SID_SIZE];
                dwSize = sizeof(adminSID);
                CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &dwSize);

                HANDLE hUnfilteredToken = NULL;
                GetTokenInformation(hToken, TokenLinkedToken, (VOID *) &hUnfilteredToken, sizeof(HANDLE), &dwSize);

                if(CheckTokenMembership(hUnfilteredToken, &adminSID, pIsAdmin))
                    bResult = TRUE;

                CloseHandle(hUnfilteredToken);
            }
            else {
                *pIsAdmin = IsUserAnAdmin();
                bResult = TRUE;
            }
        }

        CloseHandle(hToken);

        return bResult;
    }

    BOOL TerminateProcess(DWORD dwProcessId, DWORD dwExitCode) {
        BOOL bResult = FALSE;

        HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);

        if(h)
            bResult = ::TerminateProcess(h, dwExitCode);

        return bResult;
    }

    BOOL TerminateProcess(LPCTSTR pszExeName, DWORD dwExitCode) {
        BOOL bResult = FALSE;
        ProcessFinder f(TH32CS_SNAPALL, 0);
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if(f.ProcessFind(pszExeName, &pe)) {
            bResult = TerminateProcess(pe.th32ProcessID, dwExitCode);
        }

        return bResult;
    }



    inline TCHAR *lstrrchr(LPCTSTR string, int ch) {
        TCHAR *start = (TCHAR *) string;

        while(*string++)
            ;

        while(--string != start && *string != (TCHAR) ch)
            ;

        if(*string == (TCHAR) ch)
            return (TCHAR *) string;

        return NULL;
    }

    inline void DumpMiniDump(HANDLE hFile, PEXCEPTION_POINTERS excpInfo) {
        if(!excpInfo) {
            static int iTimes = 0;
            if(iTimes++ > 1)
                return;

            __try {
                RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
            }
            __except(DumpMiniDump(hFile, GetExceptionInformation()),
                     EXCEPTION_CONTINUE_EXECUTION) {
            }
        }
        else {
            MINIDUMP_EXCEPTION_INFORMATION eInfo;
            eInfo.ThreadId = GetCurrentThreadId();
            eInfo.ExceptionPointers = excpInfo;
            eInfo.ClientPointers = FALSE;

            MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                MiniDumpNormal,
                excpInfo ? &eInfo : NULL,
                NULL,
                NULL);
        }
    }

    int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS pExceptPtrs, const TCHAR *szDumpNamePrefix) {
        static bool bFirstTime = true;

        if(!bFirstTime)
            return EXCEPTION_CONTINUE_SEARCH;

        bFirstTime = false;

        // create a unique string.
        //
        TCHAR szLocalTime[50] = {0};
        SYSTEMTIME st;
        GetLocalTime(&st);
        StringCchPrintf(szLocalTime, 50, TEXT("%04d%02d%02d.%02d.%02d.%02d.%04d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        TCHAR szExeDir[MAX_PATH + 1] = {0};

        GetModuleFileName(NULL, szExeDir, MAX_PATH);

        if(TCHAR *p = lstrrchr(szExeDir, TEXT('\\'))) {
            *(p + 1) = 0;
        }

        TCHAR szDumpFileName[MAX_PATH + 1] = {0};
        _stprintf_s(szDumpFileName, MAX_PATH, TEXT("%s%s_%s.dmp"), szExeDir, szDumpNamePrefix, szLocalTime);

        HANDLE hMiniDumpFile = CreateFile(
            szDumpFileName,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL);

        if(hMiniDumpFile != INVALID_HANDLE_VALUE) {
            DumpMiniDump(hMiniDumpFile, pExceptPtrs);

            CloseHandle(hMiniDumpFile);
            hMiniDumpFile = NULL;
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void FileOp::CopyDir(const char* pszSource, const char* pszDest, bool bCopySource, int *pIgnoreNum) {
        char szSource[MAX_PATH] = {0};
        char szDest[MAX_PATH] = {0};
        strcpy_s(szSource, MAX_PATH, pszSource);
        strcpy_s(szDest, MAX_PATH, pszDest);

        if(bCopySource) {
            WIN32_FIND_DATAA filedata;
            HANDLE fhandle = FindFirstFileA(szSource, &filedata);
            size_t len = strlen(szDest);
            if(szDest[len - 1] != '\\')
                strcat_s(szDest, MAX_PATH, "\\");
            strcat_s(szDest, filedata.cFileName);
            CreateDirectoryA(szDest, NULL);
        }

        FileSearch(szSource, szDest, pIgnoreNum);
    }

    void FileOp::FileSearch(const char* szPath, const char* szDest, int *pIgnoreNum) {
        char szTemp[MAX_PATH] = {0};
        strcpy_s(szTemp, MAX_PATH, szPath);

        size_t len = strlen(szPath);
        if(szTemp[len - 1] != '\\')
            strcat_s(szTemp, MAX_PATH, "\\");
        strcat_s(szTemp, MAX_PATH, "*.*");

        WIN32_FIND_DATAA filedata;
        HANDLE fhandle = FindFirstFileA(szTemp, &filedata);
        if(fhandle != INVALID_HANDLE_VALUE) {
            AddFile(szPath, szDest, filedata, pIgnoreNum);

            while(FindNextFileA(fhandle, &filedata) != 0) {
                AddFile(szPath, szDest, filedata, pIgnoreNum);
            }
        }
        FindClose(fhandle);
    }

    void FileOp::AddFile(const char* szPath, const char* szDest, WIN32_FIND_DATAA file, int *pIgnoreNum) {
        if(file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if(((strcmp(file.cFileName, ".") != 0) && (strcmp(file.cFileName, "..") != 0))) {
                char szTemp[MAX_PATH] = {0};
                strcpy_s(szTemp, MAX_PATH, szPath);
                if(szTemp[strlen(szTemp) - 1] != '\\')
                    strcat_s(szTemp, MAX_PATH, "\\");

                strcat_s(szTemp, MAX_PATH, file.cFileName);
                //create directory.
                char szDir[MAX_PATH] = {0};
                strcpy_s(szDir, MAX_PATH, szDest);
                strcat_s(szDir, MAX_PATH, "\\");
                strcat_s(szDir, MAX_PATH, file.cFileName);
                if(_access_s(szDir, 0) != 0) {
                    if(!CreateDirectoryA(szDir, NULL)) {
                        if(pIgnoreNum) {
                            *pIgnoreNum += 1;
                        }
                    }
                }

                FileSearch(szTemp, szDir, pIgnoreNum);
            }
        }
        else {
            char szS[MAX_PATH] = {0};
            sprintf_s(szS, MAX_PATH, "%s\\%s", szPath, file.cFileName);
            char szD[MAX_PATH] = {0};
            sprintf_s(szD, MAX_PATH, "%s\\%s", szDest, file.cFileName);
            if(!CopyFileA(szS, szD, false)) {
                if(pIgnoreNum) {
                    *pIgnoreNum += 1;
                }
            }
        }
    }

    bool FileOp::DeleteDir(const char* pszDir) {
        if(!pszDir)
            return false;
        bool bRet = false;
        const int kBufSize = MAX_PATH * 4;
        HANDLE              hFind = INVALID_HANDLE_VALUE;
        CHAR                szTemp[kBufSize] = {0};
        WIN32_FIND_DATAA    wfd;

        StringCchCopyA(szTemp, kBufSize, pszDir);
        PathAddBackslashA(szTemp);
        StringCchCatA(szTemp, kBufSize, "*.*");

        hFind = FindFirstFileA(szTemp, &wfd);
        if(hFind == INVALID_HANDLE_VALUE) {
            return false;
        }

        do {
            if(lstrcmpiA(wfd.cFileName, ".") && lstrcmpiA(wfd.cFileName, "..")) {
                StringCchCopyA(szTemp, kBufSize, pszDir);
                PathAddBackslashA(szTemp);
                StringCchCatA(szTemp, kBufSize, wfd.cFileName);

                if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    bRet = DeleteDir(szTemp);
                }
                else {
                    bRet = (DeleteFileA(szTemp) == TRUE);
                    if(!bRet) {
                        SetFileAttributesA(szTemp, FILE_ATTRIBUTE_NORMAL);
                        bRet = (DeleteFileA(szTemp) == TRUE);
                    }
                }
                if(!bRet) {
                    break;
                }
            }

        } while(FindNextFileA(hFind, &wfd));

        FindClose(hFind);

        if(!bRet) {
            return bRet;
        }

        bRet = (RemoveDirectoryA(pszDir) == TRUE);
        if(!bRet) {
            DWORD dwAttr = GetFileAttributesA(pszDir);
            dwAttr &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
            SetFileAttributesA(pszDir, dwAttr);
            bRet = (RemoveDirectoryA(pszDir) == TRUE);
        }

        return bRet;
    }

    bool FileOp::CreateDir(const wchar_t* pszDir) {
        if(!pszDir)
            return false;
        wchar_t* p = NULL;
        wchar_t* szDirBuf = NULL;
        DWORD dwAttributes;
        size_t iLen = wcslen(pszDir);

        __try {
            szDirBuf = (wchar_t*) malloc((iLen + 1) * sizeof(wchar_t));
            if(szDirBuf == NULL)
                return false;

            StringCchCopyW(szDirBuf, iLen + 1, pszDir);
            p = szDirBuf;

            if((*p == L'\\') && (*(p + 1) == L'\\')) {
                p += 2;
                while(*p && *p != L'\\') p = CharNextW(p);
                if(*p) p++;
                while(*p && *p != L'\\') p = CharNextW(p);
                if(*p) p++;
            }
            else if(*(p + 1) == L':') {
                p += 2;
                if(*p && (*p == L'\\')) p++;
            }

            while(*p) {
                if(*p == L'\\') {
                    *p = L'\0';
                    dwAttributes = GetFileAttributesW(szDirBuf);

                    if(dwAttributes == 0xffffffff) {
                        if(!CreateDirectoryW(szDirBuf, NULL)) {
                            if(GetLastError() != ERROR_ALREADY_EXISTS) {
                                free(szDirBuf);
                                return false;
                            }
                        }
                    }
                    else {
                        if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
                            free(szDirBuf);
                            return false;
                        }
                    }

                    *p = L'\\';
                }

                p = CharNextW(p);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            free(szDirBuf);
            return false;
        }

        free(szDirBuf);
        return true;
    }

    bool FileOp::CreateDir(const char* pszDir) {
        if(!pszDir) return false;
        return CreateDir(AnsiToUnicode(pszDir).c_str());
    }

    bool GetFileContent(const std::string &strFilePath, std::string &strContent) {
        HANDLE hFile = CreateFileA(strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if(INVALID_HANDLE_VALUE != hFile) {
            DWORD dwRead = 0;
            LARGE_INTEGER liFileSize;
            GetFileSizeEx(hFile, &liFileSize);

            void *lpBuf = malloc((size_t) liFileSize.QuadPart);
            if(!lpBuf) {
                CloseHandle(hFile);
                return false;
            }

            ReadFile(hFile, lpBuf, (DWORD) liFileSize.QuadPart, &dwRead, NULL);
            if(dwRead != (DWORD) liFileSize.QuadPart) {
                CloseHandle(hFile);
                return false;
            }

            strContent.assign((const char*) lpBuf, dwRead);
            free(lpBuf);
            CloseHandle(hFile);
            return true;
        }

        return false;
    }



    std::string MysqlEscapeString(const std::string &strSql) {
        size_t iSrcSize = strSql.size();
        std::string strDest;
        for (size_t i = 0; i < iSrcSize; i++) {
            char ch = strSql[i];
            switch (ch)
            {
            case '\0':
                strDest.append("\\0");
                break;
            case '\n':
                strDest.append("\\n");
                break;
            case '\r':
                strDest.append("\\r");
                break;
            case '\'':
                strDest.append("\\'");
                break;
            case '"':
                strDest.append("\\\"");
                break;
            case '\\':
                strDest.append("\\\\");
                break;
            case '%':
                strDest.append("\\%");
                break;
            default:
                strDest.append(1, ch);
                break;
            }
        }

        return strDest;
    }

	bool FloatEqual(const float &f1, const float &f2)
	{
		const float EPSINON = 0.000001;
		if(abs(f1 - f2) <= EPSINON) {
			return true;
		}

		return false;
	}

	bool FloatNotEqual(const float &f1, const float &f2)
	{
		return !FloatEqual(f1, f2);
	}

}
