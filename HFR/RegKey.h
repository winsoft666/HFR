#pragma once

#include <windows.h>
#include <string>
#include <vector>


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

	bool DeleteKey(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, bool bPrefer64View);
	HRESULT RegKey::GetDWORDValue(LPCTSTR pszValueName, DWORD *pdwDataOut) const;
	HRESULT RegKey::GetBINARYValue(LPCTSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const;
	HRESULT RegKey::GetSZValue(LPCTSTR pszValueName, OUT std::wstring &strValue) const;
	HRESULT RegKey::GetMultiSZValue(LPCTSTR pszValueName, OUT std::vector<std::wstring> &vStrValues) const;
	int GetValueBufferSize(LPCTSTR pszValueName) const;
	HRESULT SetDWORDValue(LPCTSTR pszValueName, DWORD dwData);
	HRESULT SetBINARYValue(LPCTSTR pszValueName, const LPBYTE pbData, int cbData);
	HRESULT SetSZValue(LPCTSTR pszValueName, const std::wstring &strData);
	HRESULT SetMultiSZValue(LPCTSTR pszValueName, const std::vector<std::wstring> &vStrValues);

	static bool IsWin64();
private:
	RegKey(const RegKey &rhs);
	RegKey &operator = (const RegKey &rhs);
	HKEY         m_hkeyRoot;
	mutable HKEY m_hkey;
	DWORD        m_dwChangeFilter;
	std::wstring m_strSubKey;
	bool         m_bWatchSubtree;

	HRESULT GetValue(LPCTSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const;
	HRESULT SetValue(LPCTSTR pszValueName, DWORD dwValueType, const LPBYTE pbData, int cbData);
	LPTSTR CreateDoubleNulTermList(const std::vector<std::wstring> &vStrValues) const;
	static bool RegDeleteKey32_64(HKEY hKey, LPCTSTR pszSubKey, bool bPrefer64View);
	static bool RegDeleteSubKeys(HKEY hKey, bool bPrefer64View);
};