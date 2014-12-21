#include "stdafx.h"
#include "tokenstring.h"

CTokenString::CTokenString(const char *pszString,int nSize,int nMode)
{
	m_nLast = 0;
	m_pTokenList = NULL;
	SetString(pszString,nSize,nMode);
}

CTokenString::~CTokenString()
{
}

void CTokenString::ReAlloc()
{
	int i = 0;
	_TTOKEN *pTemp = new _TTOKEN[m_nLast+20];

	while(i<m_nLast) {
		pTemp[i].cName = m_pTokenList[i].cName;
		pTemp[i].cValue = m_pTokenList[i].cValue;
		i++;
	}

	while(i<(m_nLast+20)) {
		pTemp[i].cName = NULL;
		pTemp[i].cValue = NULL;
		i++;
	}

	if(m_pTokenList) delete [] m_pTokenList;
	m_pTokenList = pTemp;
	m_nLast += 20;
}

int CTokenString::SetString(const char *pszString,int nSize,int nMode)
{
	int i;
	string sName,sValue;

	DeleteString();

	if(pszString==NULL) return 1;

	if(nSize<0) nSize = (int)strlen(pszString);
	if(nSize<=0) return 0;

	i=0;
	while(i<nSize) {
		sName.clear();
		sValue.clear();

		while(i<nSize && isspace(pszString[i])) i++;

		while(i<nSize && pszString[i]!='=' && !isspace(pszString[i]))
			sName += pszString[i++];

		while(i<nSize && isspace(pszString[i])) i++;

		if(i<nSize && pszString[i]=='=') {
			i++;

			while(i<nSize && (pszString[i]==' ' || pszString[i]=='\t')) i++;

			int nAnf = 0;
			while(i<nSize) {
				if(isspace(pszString[i]) && !nAnf) break;
				if(pszString[i]=='"') {
					if(nAnf) nAnf = 0;
					else nAnf = 1;

					i++;
					continue;
				}
				sValue += pszString[i++];
			}
		}

		if(!sName.empty()) {
			if(nMode==TS_DUPLICATES) AppendToken(sName,sValue);
			else SetToken(sName,sValue);
		}
	}

	return nSize+1;
}

void CTokenString::DeleteString()
{
	int i = 0;

	if(m_pTokenList) {
		while(i<m_nLast) {
			if(m_pTokenList[i].cName!=NULL) delete [] m_pTokenList[i].cName;
			if(m_pTokenList[i].cValue!=NULL) delete [] m_pTokenList[i].cValue;
			m_pTokenList[i].cName = NULL;
			m_pTokenList[i].cValue = NULL;
			i++;
		}
		delete [] m_pTokenList;
		m_pTokenList = NULL;
		m_nLast = 0;
	}
}

void CTokenString::SetToken(string &sName,string &sValue)
{
	int i = 0;
	int nFree = -1;

	if(sName.empty()) return;

	while(i<m_nLast) {
		if(m_pTokenList[i].cName==NULL && nFree==-1) nFree = i;
		if(m_pTokenList[i].cName!=NULL && strcasecmp(m_pTokenList[i].cName,sName.c_str())==0) break;
		i++;
	}

	if(i==m_nLast) {
		if(nFree==-1) {
			i = m_nLast;
			ReAlloc();
		} else i = nFree;
		m_pTokenList[i].cName = new char[sName.length()+1];
		strcpy(m_pTokenList[i].cName,sName.c_str());
	}

	if(m_pTokenList[i].cValue!=NULL) delete [] m_pTokenList[i].cValue;
	if(!sValue.empty()) {
		m_pTokenList[i].cValue = new char[sValue.length()+1];
		strcpy(m_pTokenList[i].cValue,sValue.c_str());
	} else m_pTokenList[i].cValue = NULL;
}

void CTokenString::AppendToken(string &sName,string &sValue)
{
}

const char* CTokenString::GetToken(const char *pszName)
{
	int i = 0;

	if(pszName==NULL) return NULL;

	if(m_pTokenList) {
		while(i<m_nLast) {
			if(m_pTokenList[i].cName!=NULL && strcasecmp(m_pTokenList[i].cName,pszName)==0) {
				if(m_pTokenList[i].cValue!=NULL) return m_pTokenList[i].cValue;
				else return (char*)"";
			} else i++;
		}
	}
	return NULL;
}
