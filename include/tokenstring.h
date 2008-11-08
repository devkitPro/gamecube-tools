#ifndef __TOKENSTRING_H__
#define __TOKENSTRING_H__

#include <string>

#define TS_MAXSTRINGLEN				1024

#define TS_NODUPLICATES				0
#define TS_DUPLICATES				1

using namespace std;

class CTokenString
{
public:
	typedef struct _TTOKEN {
		char *cName;
		char *cValue;
	} _TTOKEN;

public:
	CTokenString(const char *pszString = NULL,int nSize = -1,int nMode = TS_NODUPLICATES);
	virtual ~CTokenString();

	int SetString(const char *pszString = NULL,int nSize = -1,int nMode = TS_NODUPLICATES);
	
	void DeleteString();
	void SetToken(string &sName,string &sValue);
	void AppendToken(string &sName,string &sValue);

	const char* GetToken(const char *pszName);

private:
	void ReAlloc();

	int m_nLast;
	char m_cBuf[TS_MAXSTRINGLEN];
	_TTOKEN *m_pTokenList;
};

#endif
