/*******************************************************************************
*
* Copyright (C) 2013 - 2015, Jeffery Jiang, <china_jeffery@163.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
*******************************************************************************/

#ifndef __CMN_UTIL_H_SAF234_
#define __CMN_UTIL_H_SAF234_
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _INC_WINDOWS
#include <Windows.h>
#endif
#include <tchar.h>
#include <strsafe.h>
#pragma warning ( disable : 4995 )
#include <string>
#include <vector>
#include <ctime>
#include <math.h>
#include <limits>
#include <Shlwapi.h>
#include <TlHelp32.h>

typedef std::basic_string<TCHAR> tstring;

#ifndef SAFE_FREE
#define SAFE_FREE(p) do{if((p) != NULL){free(p); (p)=NULL;}}while(false)
#endif

#ifndef SAFE_CLOSE
#define SAFE_CLOSE(p) do{if((p) != NULL){CloseHandle((p)); (p)=NULL;}}while(false)
#endif

#ifndef SAFE_CLOSE_2
#define SAFE_CLOSE_2(p) do{if((p) != INVALID_HANDLE_VALUE){CloseHandle((p)); (p)=INVALID_HANDLE_VALUE;}}while(false)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) do{if((p) != NULL){delete[](p); (p)=NULL;}}while(false)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) do{if((p) != NULL){delete (p); (p)=NULL;}}while(false)
#endif

#define chSTR2(x) #x
#define chSTR(x)  chSTR2(x)
#define chMSG(desc) message(__FILE__ "(" chSTR(__LINE__) "):" #desc)

#ifdef _X86_
#define DebugBreak()    _asm { int 3 }
#endif

namespace cpp4j {
    void chMB(PCSTR szMsg);
    void chFAIL(PSTR szMsg);
    void chASSERTFAIL(LPCSTR file, int line, PCSTR expr);
    double RoundEx(double v, int digits);
    BOOL CREATEPROCESS(LPCTSTR szCmdLine, LPPROCESS_INFORMATION lpProcessInfo);
    void TraceMsgW(const wchar_t *lpFormat, ...);
    void TraceMsgA(const char *lpFormat, ...);
    std::string GetExeDirA();
    std::wstring GetExeDirW();
    bool IsWin64();
    BOOL CheckSingleInstance(LPCTSTR pszUniqueName);
    bool GetFileContent(const std::string &strFilePath, std::string &strContent);

    std::string MysqlEscapeString(const std::string &strSql);
  
	bool FloatEqual(const float &f1, const float &f2);
	bool FloatNotEqual(const float &f1, const float &f2);

    class CritSec {
    public:
#if 0
        DWORD   m_dwCurrentOwner;
        DWORD   m_dwLockCount;
        BOOL    m_fTrace;        // Trace this one
        CritSec();
        ~CritSec();
        void Lock();
        void Unlock();
#else
        CritSec();
        ~CritSec();
        void Lock();
        void Unlock();
#endif
    private:
        CritSec(const CritSec &refCritSec);
        CritSec &operator=(const CritSec &refCritSec);
        CRITICAL_SECTION m_CS;
    };

    class Locker {
    public:
        explicit Locker(CritSec *pCS);
        ~Locker();
    private:
        Locker(const Locker &lock);
        Locker &operator=(const Locker &lock);
        CritSec* m_pCS;
    };



    class NoCopyable {
    protected:
        NoCopyable() {};
        virtual ~NoCopyable() {};

    private:
        NoCopyable(const NoCopyable &);
        const NoCopyable &operator =(const NoCopyable &);
    };


    std::string UnicodeToAnsi(const std::wstring &str);
    std::wstring AnsiToUnicode(const std::string &str);
    std::string UnicodeToUtf8(const std::wstring &str);
    std::wstring Utf8ToUnicode(const std::string &str);
    std::string AnsiToUtf8(const std::string &str);
    std::string Utf8ToAnsi(const std::string &str);
    std::string _UrlEncode(const std::string &str);

	std::string UnicodeToUtf8_pach(const std::wstring &str);
	std::string AnsiToUtf8_pach(const std::string &str);


#pragma warning(disable: 4996)
    enum OSVer {
        WIN_UNKNOWN = 0,
        WIN_2000,
        WIN_XP,
        WIN_XP_PRO,
        WIN_2003,
        WIN_2003_R2,
        WIN_HOME,
        WIN_VISTA,
        WIN_2008,
        WIN_2008_R2,
        WIN_7,
        WIN_2012,
        WIN_8,
        WIN_2012_R2,
        WIN_8_1,
        WIN_SERVER_PRE,
        WIN_10_PRE
    };
    class OSVersion {
    public:
        OSVersion();

        // 2003R2, 2003, XP ....
        //
        bool AtMostWin2003R2(void);

        // Vista, 7, 8 ....
        //
        bool AtLeastWinVista(void);
        void GetMajorMinorBuild(DWORD &dwMajor, DWORD &dwMinor, DWORD &dwBuild);
        bool IsWinXPProX64();
        OSVer GetOsVersion(void);
        std::string GetOsVerSummary(void);
    private:
        void ____Init();
        OSVERSIONINFOEXW osvi;
    };

    class TimerMeter {
    public:
        TimerMeter() {
            lStartTime = std::clock();
        }

        void Restart() {
            lStartTime = std::clock();
        }

        double Elapsed() const {
            return  double(std::clock() - lStartTime) / CLOCKS_PER_SEC;
        }

        double ElapsedMax() const {
            return (double((std::numeric_limits<std::clock_t>::max)())
                    - double(lStartTime)) / double(CLOCKS_PER_SEC);
        }

        double ElapsedMin() const {
            return double(1) / double(CLOCKS_PER_SEC);
        }

    private:
        std::clock_t lStartTime;
    };


    class ThreadTimer {
    public:
        ThreadTimer();
        virtual ~ThreadTimer();
        static void CALLBACK TimerProc(void *param, BOOLEAN timerCalled);
        BOOL Start(DWORD ulInterval,  // ulInterval in ms
                   BOOL bImmediately,
                   BOOL bOnce);
        void Stop(bool bWait);
        virtual void OnTimedEvent();
    private:
        HANDLE m_hTimer;
        PTP_TIMER m_pTimer;
    };

    template <class T>
    class TTimer : public ThreadTimer {
    public:
        typedef private void (T::*POnTimer)(void);

        TTimer() {
            m_pfnOnTimer = NULL;
            m_pClass = NULL;
        }

        void SetTimedEvent(T *pClass, POnTimer pFunc) {
            m_pClass = pClass;
            m_pfnOnTimer = pFunc;
        }

    protected:
        void OnTimedEvent() {
            if(m_pfnOnTimer && m_pClass) {
                (m_pClass->*m_pfnOnTimer)();
            }
        }

    private:
        T *m_pClass;
        POnTimer m_pfnOnTimer;
    };



    class RegKey {
    public:
        RegKey(void);
        RegKey(HKEY hkeyRoot, LPCTSTR pszSubKey);
        ~RegKey(void);
        HRESULT Open(REGSAM samDesired, bool bCreate);
        bool IsOpen(void) const;
        HKEY GetHandle(void) const;
        void Attach(HKEY hkey);
        void Detach(void);
        void Close(void);
        HRESULT WatchForChange(DWORD dwChangeFilter, bool bWatchSubtree);
        HRESULT WaitForChange(DWORD dwChangeFilter, bool bWatchSubtree);
        bool DeleteKey(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, bool bPrefer64View);
        HRESULT RegKey::GetDWORDValue(LPCTSTR pszValueName, DWORD *pdwDataOut) const;
        HRESULT RegKey::GetBINARYValue(LPCTSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const;
        HRESULT RegKey::GetSZValue(LPCTSTR pszValueName, OUT tstring &strValue) const;
        HRESULT RegKey::GetMultiSZValue(LPCTSTR pszValueName, OUT std::vector<tstring> &vStrValues) const;
        int GetValueBufferSize(LPCTSTR pszValueName) const;
        HRESULT SetDWORDValue(LPCTSTR pszValueName, DWORD dwData);
        HRESULT SetBINARYValue(LPCTSTR pszValueName, const LPBYTE pbData, int cbData);
        HRESULT SetSZValue(LPCTSTR pszValueName, const tstring &strData);
        HRESULT SetMultiSZValue(LPCTSTR pszValueName, const std::vector<tstring> &vStrValues);

    protected:
        void RegKey::OnChange(HKEY hkey);
    private:
        RegKey(const RegKey &rhs);
        RegKey &operator = (const RegKey &rhs);
        HKEY         m_hkeyRoot;
        mutable HKEY m_hkey;
        HANDLE       m_hChangeEvent;
        HANDLE       m_hNotifyThr;
        DWORD        m_dwChangeFilter;
        tstring      m_strSubKey;
        bool         m_bWatchSubtree;

        HRESULT GetValue(LPCTSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const;
        HRESULT SetValue(LPCTSTR pszValueName, DWORD dwValueType, const LPBYTE pbData, int cbData);
        LPTSTR CreateDoubleNulTermList(const std::vector<tstring> &vStrValues) const;
        static unsigned int _stdcall NotifyWaitThreadProc(LPVOID pvParam);
        static bool RegDeleteKey32_64(HKEY hKey, LPCTSTR pszSubKey, bool bPrefer64View);
        static bool RegDeleteSubKeys(HKEY hKey, bool bPrefer64View);
    };



#if 1
#if (defined DEBUG) || (defined _DEBUG)
    void __SetThreadName(DWORD dwThreadID, const char *szThreadName);
#endif
#endif
    HANDLE CREATETHREAD(void *pSecurity,
                        unsigned uiStackSize,
                        unsigned(__stdcall *pfnStartAddr) (void *),
                        void *pvParam,
                        unsigned uiCreateFlag,
                        unsigned *puiThreadID,
                        const char *szThreadName);


    class ProcessFinder {
    public:
        explicit ProcessFinder(DWORD dwFlags = 0, DWORD dwProcessID = 0);
        ~ProcessFinder();
        BOOL CreateSnapShot(DWORD dwFlag, DWORD dwProcessID);
        BOOL ProcessFirst(PPROCESSENTRY32 ppe) const;
        BOOL ProcessNext(PPROCESSENTRY32 ppe) const;
        BOOL ProcessFind(DWORD dwProcessId, PPROCESSENTRY32 ppe) const;
        BOOL ProcessFind(PCTSTR pszExeName, PPROCESSENTRY32 ppe) const;
        BOOL ModuleFirst(PMODULEENTRY32 pme) const;
        BOOL ModuleNext(PMODULEENTRY32 pme) const;
        BOOL ModuleFind(PVOID pvBaseAddr, PMODULEENTRY32 pme) const;
        BOOL ModuleFind(PTSTR pszModName, PMODULEENTRY32 pme) const;
    private:
        HANDLE m_hSnapShot;
    };

    class FileOp {
    public:
        static bool DeleteDir(const char* pszDir);
        static bool CreateDir(const wchar_t* pszDir);
        static bool CreateDir(const char* pszDir);
        static void CopyDir(const char* pszSource, const char* pszDest, bool bCopySource, int *pIgnoreNum);
    private:
        static void FileSearch(const char* szPath, const char* szDest, int *pIgnoreNum);
        static void AddFile(const char* szPath, const char* szDest, WIN32_FIND_DATAA file, int *pIgnoreNum);
    private:
        FileOp();
        ~FileOp();
    };


    BOOL RunAsAdministrator(LPCTSTR szCommand, LPCTSTR szArgs, BOOL bWaitProcess);
    BOOL CreateProcessInCurrentSession(PCTSTR pszFilePath);
    BOOL EnablePrivilege(LPCTSTR szPrivilege, BOOL fEnable);
    BOOL CheckProcessUserIsAdmin(BOOL *pIsAdmin);
    BOOL TerminateProcess(DWORD dwProcessId, DWORD dwExitCode);
    BOOL TerminateProcess(LPCTSTR pszExeName, DWORD dwExitCode);


    typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

    int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS pExceptPtrs, const TCHAR *szDumpNamePrefix);
    TCHAR *lstrrchr(LPCTSTR string, int ch);
    void DumpMiniDump(HANDLE hFile, PEXCEPTION_POINTERS excpInfo);
}

#ifdef _DEBUG
#define chASSERT(x) if (!(x)) cpp4j::chASSERTFAIL(__FILE__, __LINE__, #x)
#else
#define chASSERT(x)
#endif

#ifdef _DEBUG
#define chVERIFY(x) chASSERT(x)
#else
#define chVERIFY(x) (x)
#endif


#if (defined UNICODE) || (defined _UNICODE)
#define TraceMsg cpp4j::TraceMsgW
#else
#define TraceMsg cpp4j::TraceMsgA
#endif


#if (defined UNICODE) || (defined _UNICODE)
#define GetExeDir cpp4j::GetExeDirW
#else
#define GetExeDir cpp4j::GetExeDirA
#endif


#ifdef UNICODE
#define TCHARToAnsi(str) cpp4j::UnicodeToAnsi(str)
#define TCHARToUtf8 cpp4j::UnicodeToUtf8(str)
#define AnsiToTCHAR(str) cpp4j::AnsiToUnicode(str)
#define Utf8ToTCHAR(str) cpp4j::Utf8ToUnicode(str)
#else
#define TCHARToAnsi (str)
#define TCHARToUtf8 cpp4j::AnsiToUtf8(str)
#define AnsiToTCHAR(str) (str)
#define Utf8ToTCHAR(str) Utf8ToAnsi(str)
#endif




#define SINGLETION_DECLARE(T) \
    public:\
    static T* GetInst(){\
        cpp4j::Locker l(&m_InstCS_342D_);\
        if(!m_pInst)\
            m_pInst = new T();\
        return m_pInst;\
    }\
    static void Release(){\
        cpp4j::Locker l(&m_InstCS_342D_);\
        if(m_pInst){\
            delete m_pInst;\
            m_pInst = NULL;\
                        }\
    }\
    private:\
    static cpp4j::CritSec m_InstCS_342D_;\
    static T*m_pInst;\

#define SINGLETION_DEFINE(T)\
    T* T::m_pInst = NULL;\
    cpp4j::CritSec T::m_InstCS_342D_;



#define WINMAIN_BEGIN(szDumpNamePrefix) \
    int __96A9695E_RUN_WINMAIN_FUNC(HINSTANCE hInstance, LPTSTR lpCmdLine);\
    LONG WINAPI __96A9695E_UnhandledExceptionHandler( _EXCEPTION_POINTERS *pExceptionInfo ) \
    { \
        OutputDebugString(TEXT("Create a dump file sine an exception occurred in sub-thread.\n")); \
        int iRet = cpp4j::RecordExceptionInfo(pExceptionInfo, szDumpNamePrefix); \
        CrashNotify(); \
        return iRet; \
    } \
    int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) \
    { \
        UNREFERENCED_PARAMETER(hPrevInstance); \
        UNREFERENCED_PARAMETER(nCmdShow); \
        ::SetUnhandledExceptionFilter( __96A9695E_UnhandledExceptionHandler );\
        int ret = 0;\
        __try\
        {\
            ret = __96A9695E_RUN_WINMAIN_FUNC(hInstance, lpCmdLine);\
        }\
        __except(cpp4j::RecordExceptionInfo(GetExceptionInformation(), szDumpNamePrefix))\
        {\
            OutputDebugString(TEXT("Create a dump file sine an exception occurred in main-thread.\n")); \
            CrashNotify(); \
        }\
        return ret;\
    }\
    int __96A9695E_RUN_WINMAIN_FUNC(HINSTANCE hInstance, LPTSTR lpCmdLine) \
    {

#define WINMAIN_END }




#define MAIN_BEGIN(szDumpName) \
    int __96A9695E_RUN_MAIN_FUNC(int argc, _TCHAR* argv[]);\
    LONG WINAPI __96A9695E_UnhandledExceptionHandler( _EXCEPTION_POINTERS *pExceptionInfo ) \
    { \
        OutputDebugString(TEXT("Create a dump file since an exception occurred in sub-thread.\n")); \
        int iRet = cpp4j::RecordExceptionInfo(pExceptionInfo, szDumpName); \
        CrashNotify(); \
        return iRet; \
    } \
    int _tmain(int argc, _TCHAR* argv[])\
    { \
        ::SetUnhandledExceptionFilter( __96A9695E_UnhandledExceptionHandler );\
        int ret = 0;\
        __try\
        {\
            ret = __96A9695E_RUN_MAIN_FUNC(argc, argv);\
        }\
        __except(cpp4j::RecordExceptionInfo(GetExceptionInformation(), szDumpName))\
        {\
            OutputDebugString(TEXT("Create a dump file since an exception occurred in main-thread.\n")); \
            CrashNotify(); \
        }\
        return ret;\
    }\
    int __96A9695E_RUN_MAIN_FUNC(int argc, _TCHAR* argv[]) \
    {

#define MAIN_END }


#endif