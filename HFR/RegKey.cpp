#include <RegKey.h>
#include <strsafe.h>

#ifndef SAFE_CLOSE
#define SAFE_CLOSE(p) do{if((p) != NULL){CloseHandle((p)); (p)=NULL;}}while(false)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) do{if((p) != NULL){delete[](p); (p)=NULL;}}while(false)
#endif

RegKey::RegKey(void) : m_hkeyRoot(NULL),
	m_hkey(NULL),
	m_bWatchSubtree(false),
	m_dwChangeFilter(0) 
{
}

RegKey::RegKey(HKEY hkeyRoot, LPCTSTR pszSubKey) : m_hkeyRoot(hkeyRoot),
m_hkey(NULL),
m_bWatchSubtree(false),
m_dwChangeFilter(0),
m_strSubKey(pszSubKey)
{
}

RegKey::~RegKey(void) {
	Close();
}

HRESULT RegKey::Open(REGSAM samDesired, bool bCreate) {
	DWORD dwResult = ERROR_SUCCESS;
	Close();

	if (bCreate) {
		DWORD dwDisposition;
		dwResult = RegCreateKeyEx(m_hkeyRoot,
			(LPCTSTR)m_strSubKey.c_str(),
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
			(LPCTSTR)m_strSubKey.c_str(),
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
	if (NULL != m_hkey) {
		HKEY hkeyTemp = m_hkey;
		m_hkey = NULL;
		RegCloseKey(hkeyTemp);
	}
}

bool RegKey::DeleteKey(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, bool bPrefer64View) {
	HKEY hSubKey = NULL;

	if (pszSubKey) {
		REGSAM rsam = KEY_READ | KEY_WRITE;

		if (bPrefer64View)
			rsam |= KEY_WOW64_64KEY;

		RegOpenKeyEx(hKey, pszSubKey, 0, rsam, &hSubKey);
	}
	else {
		hSubKey = hKey;
	}

	if (hSubKey) {
		if (pszValName) {
			if (RegDeleteValue(hSubKey, pszValName) == ERROR_SUCCESS) {
				if (hSubKey != hKey) {
					if (hSubKey)
						RegCloseKey(hSubKey);
				}

				return true;
			}
		}
		else {
			if (RegDeleteSubKeys(hSubKey, bPrefer64View)) {
				if (hSubKey != hKey) {
					if (hSubKey)
						RegCloseKey(hSubKey);
				}

				return RegDeleteKey32_64(hKey, pszSubKey, bPrefer64View);
			}
		}

		if (hSubKey != hKey) {
			if (hSubKey)
				RegCloseKey(hSubKey);
		}
	}

	return false;
}

HRESULT RegKey::GetDWORDValue(LPCTSTR pszValueName, DWORD *pdwDataOut) const {
	return GetValue(pszValueName, REG_DWORD, (LPBYTE)pdwDataOut, sizeof(DWORD));
}

HRESULT RegKey::GetBINARYValue(LPCTSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const {
	return GetValue(pszValueName, REG_BINARY, pbDataOut, cbDataOut);
}

HRESULT RegKey::GetSZValue(LPCTSTR pszValueName, OUT std::wstring &strValue) const {
	HRESULT hr = E_FAIL;
	int cb = GetValueBufferSize(pszValueName);

	if (cb <= 0)
		return hr;

	TCHAR *szTemp = new TCHAR[cb / sizeof(TCHAR)];
	memset(szTemp, 0, cb);

	if (NULL != szTemp && 0 < cb) {
		hr = GetValue(pszValueName, REG_SZ, (LPBYTE)szTemp, cb);
	}

	strValue = szTemp;
	SAFE_DELETE_ARRAY(szTemp);

	return hr;
}

HRESULT RegKey::GetMultiSZValue(LPCTSTR pszValueName, OUT std::vector<std::wstring> &vStrValues) const {
	HRESULT hr = E_FAIL;
	int cb = GetValueBufferSize(pszValueName);
	TCHAR *szTemp = new TCHAR[cb / sizeof(TCHAR)];
	TCHAR *szBegin = szTemp;

	if (NULL != szTemp && 0 < cb) {
		hr = GetValue(pszValueName, REG_MULTI_SZ, (LPBYTE)szTemp, cb);

		if (SUCCEEDED(hr)) {
			while (szTemp && TEXT('\0') != *szTemp) {
				vStrValues.push_back(std::wstring(szTemp));
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
		(LPDWORD)&cbData);
	return cbData;
}

HRESULT RegKey::SetDWORDValue(LPCTSTR pszValueName, DWORD dwData) {
	return SetValue(pszValueName, REG_DWORD, (const LPBYTE)&dwData, sizeof(dwData));
}

HRESULT RegKey::SetBINARYValue(LPCTSTR pszValueName, const LPBYTE pbData, int cbData) {
	return SetValue(pszValueName, REG_BINARY, pbData, cbData);
}

HRESULT RegKey::SetSZValue(LPCTSTR pszValueName, const std::wstring &strData) {
	return SetValue(pszValueName, REG_SZ, (const LPBYTE)strData.c_str(), (strData.length()) * sizeof(TCHAR));
}

HRESULT RegKey::SetMultiSZValue(LPCTSTR pszValueName, const std::vector<std::wstring> &vStrValues) {
	TCHAR *ptrValues = CreateDoubleNulTermList(vStrValues);
	int cch = 1;
	int n = vStrValues.size();

	for (int i = 0; i < n; i++)
		cch += vStrValues[i].length() + 1;

	HRESULT hr = SetValue(pszValueName, REG_MULTI_SZ, (const LPBYTE)ptrValues, cch * sizeof(TCHAR));

	SAFE_DELETE_ARRAY(ptrValues);

	return hr;
}


HRESULT RegKey::GetValue(LPCTSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const {
	DWORD dwType;
	HRESULT hr = RegQueryValueEx(m_hkey,
		pszValueName,
		0,
		&dwType,
		pbData,
		(LPDWORD)&cbData);

	if (ERROR_SUCCESS == hr && dwType != dwTypeExpected)
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

LPTSTR RegKey::CreateDoubleNulTermList(const std::vector<std::wstring> &vStrValues) const {
	size_t cEntries = vStrValues.size();
	size_t cch = 1; // Account for 2nd null terminate.

	for (size_t i = 0; i < cEntries; i++)
		cch += vStrValues[i].length() + 1;

	LPTSTR pszBuf = new TCHAR[cch];
	LPTSTR pszWrite = pszBuf;

	for (size_t i = 0; i < cEntries; i++) {
		const std::wstring &s = vStrValues[i];
		StringCchCopy(pszWrite, cch, s.c_str());
		pszWrite += s.length() + 1;
	}

	*pszWrite = TEXT('\0'); // Double null terminate.
	return pszBuf;
}


bool RegKey::RegDeleteKey32_64(HKEY hKey, LPCTSTR pszSubKey, bool bPrefer64View) {
	REGSAM rsam = (bPrefer64View && IsWin64()) ? KEY_WOW64_64KEY : KEY_WOW64_32KEY;
	HMODULE hAdvAPI32 = LoadLibrary(TEXT("AdvAPI32.dll"));

	if (!hAdvAPI32)
		return false;

	LSTATUS ls;
	typedef LONG(WINAPI * PFN_RegDeleteKeyEx)(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
#if (defined UNICODE) || (defined _UNICODE)
#define RegDeleteKeyExFuncName "RegDeleteKeyExW"
#else
#define RegDeleteKeyExFuncName "RegDeleteKeyExA"
#endif
	PFN_RegDeleteKeyEx _RegDeleteKeyEx = (PFN_RegDeleteKeyEx)GetProcAddress(hAdvAPI32, RegDeleteKeyExFuncName);

	if (_RegDeleteKeyEx) {
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

	if (RegQueryInfoKey(hKey, NULL, NULL, 0, &dwSubKeyCnt, &dwMaxSubKey, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
		if (dwSubKeyCnt) {
			dwMaxSubKey += sizeof(TCHAR);
			TCHAR szKeyName[iMaxKeySize] = { 0 };

			do {
				if (RegEnumKey(hKey, --dwSubKeyCnt, szKeyName, iMaxKeySize) == ERROR_SUCCESS) {
					HKEY hSubKey = NULL;

					if (ERROR_SUCCESS == RegOpenKeyEx(hKey, szKeyName, 0, KEY_READ | KEY_WRITE, &hSubKey) && hSubKey) {
						if (RegDeleteSubKeys(hSubKey, bPrefer64View)) {
							if (hSubKey)
								RegCloseKey(hSubKey);

							if (!RegDeleteKey32_64(hKey, szKeyName, bPrefer64View))
								return false;
						}
						else {
							if (hSubKey)
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
			} while (dwSubKeyCnt);
		}

		return true;
	}

	return false;
}

bool RegKey::IsWin64() {
	typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	static LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	BOOL bIsWow64 = FALSE;

	if (NULL == fnIsWow64Process) {
		HMODULE h = GetModuleHandleW(L"kernel32");
		if (h)
			fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(h, "IsWow64Process");
	}

	if (NULL != fnIsWow64Process) {
		fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
	}

	return bIsWow64 == 1;
}