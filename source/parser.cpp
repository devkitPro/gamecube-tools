#include "stdafx.h"
#include "parser.h"
#include "tokenstring.h"

CParser::_ttokenstringlist::_ttokenstringlist(const char *pszTokenString,int nLen)
{
	nTokens = 0;
	pNext = NULL;
	pToken = new CTokenString(pszTokenString,nLen);
}

CParser::_ttokenstringlist::~_ttokenstringlist()
{
	if(pToken) delete pToken;
	pToken = NULL;
	pNext = NULL;
	nTokens = 0;
}


const char* CParser::_ttokenstringlist::GetTokenValue(const char *pszTokenName,const char *pszDefault) const
{
	const char *p;
	if(!pToken) return NULL;
	return ((p=pToken->GetToken(pszTokenName))?p:pszDefault);
}

CParser::CParser()
{
	m_pArgList = NULL;
	m_sInFilename = "";
	m_sOutFilename = "";
}

CParser::~CParser()
{
	ReleaseAll();
}

void CParser::ReleaseAll()
{
	Empty();

	m_pArgList = NULL;
	m_sInFilename = "";
	m_sOutFilename = "";
}

void CParser::Empty()
{
	_ttokenstringlist *ptr;

	while(m_pArgList) {
		ptr = m_pArgList;
		m_pArgList = m_pArgList->pNext;
		delete ptr;
	}
}

int CParser::NewTokenString(const char *pszTokenString,int nLen)
{
	_ttokenstringlist *ptr = NULL;
	_ttokenstringlist *ptr2 = m_pArgList;

	ptr = new _ttokenstringlist(pszTokenString,nLen);
	if(ptr) {
		while(ptr2 && ptr2->pNext) ptr2 = ptr2->pNext;
		if(ptr2) ptr2->pNext = ptr;
		else m_pArgList = ptr;

		return 1;
	}
	return 0;
}

int CParser::Parse(int argc,const char **argv)
{
	int arg;
	string id,cmdline = "";
	bool useScript,mipMap;

	if(!argc || !argv) return EXIT_FAILURE;

	useScript = false;
	for(arg=1;arg<argc;arg++) {
		if(argv[arg][0]=='-') {
			switch(argv[arg][1]) {
				case 'i':
					useScript = false;
					m_sInFilename = (argc>arg)?argv[++arg]:"";
					break;
				case 's':
					useScript = true;
					m_sInFilename = (argc>arg)?argv[++arg]:"";
					break;
				case 'o':
					m_sOutFilename = (argc>arg)?argv[++arg]:"";
					break;
				case 'd':
					m_sDepFilename = (argc>arg)?argv[++arg]:"";
					break;
			}
		}
	}
	
	
	if(m_sInFilename.empty()) return EXIT_FAILURE;

	id = m_sInFilename.substr(0,m_sInFilename.find('.'));
	if(m_sOutFilename.empty()) m_sOutFilename = id + ".tpl";

	if(useScript==true)
		LoadScriptfile(m_sInFilename.c_str());
	else {
		string palfmt = "";
		string colfmt = " colfmt=6";
		string mipmap = "",lods = "";

		id = m_sOutFilename.substr(0,m_sOutFilename.find('.'));

		cmdline = "filepath=\"";
		cmdline += m_sInFilename;
		cmdline += "\" id=\"";
		cmdline += id + "\"";

		mipMap = false;
		for(arg=1;arg<argc;arg++) {
			if(argv[arg][0]=='-') continue;

			if(strncasecmp(argv[arg],"colfmt=",7)==0) colfmt = argv[arg];
			if(strncasecmp(argv[arg],"palfmt=",7)==0) palfmt = argv[arg];
			else if(strncasecmp(argv[arg],"mipmap=yes",10)==0) {
				mipmap = argv[arg];
				mipMap = true;
			} else if(mipMap==true) {
				if(strncasecmp(argv[arg],"minlod=",7)==0
					|| strncasecmp(argv[arg],"maxlod=",7)==0
					|| strncasecmp(argv[arg],"remaplod=",9)==0) {
						if(!lods.empty()) lods += " ";
						lods += argv[arg];
					}
			}
		}
		
		cmdline += " " + colfmt;
		if(!palfmt.empty()) cmdline += " " + palfmt;
		if(mipMap==true) cmdline += " " + mipmap + " " + lods;

		NewTokenString(cmdline.c_str(),(int)cmdline.length());
	}
	return EXIT_SUCCESS;
}

int CParser::LoadScriptfile(const char *pszFilename)
{
	int nLen = 0;
	int nRet = EXIT_SUCCESS;
	FILE *pScf = NULL;
	char *pBuffer = NULL;

	pScf = fopen(pszFilename,"rb");
	if(pScf) {
		if(fseek(pScf,0,SEEK_END)==0)
			nLen = ftell(pScf);
		else 
			nLen = 0;

		if(nLen<=0 || fseek(pScf,0,SEEK_SET)!=0)
			nRet = EXIT_FAILURE;
		else {
			pBuffer = new char[nLen+1];
			nLen = (int)fread(pBuffer,1,nLen,pScf);
			if(nLen<=0)
				nRet = EXIT_FAILURE;
			else
				pBuffer[nLen] = 0;
		}
		fclose(pScf);
		m_sScriptPath = pszFilename;
		m_sScriptPath.erase(m_sScriptPath.find_last_of("/\\"));
		m_sScriptPath += "/";		
		
	} else
		nRet = EXIT_FAILURE;

	if(nRet==EXIT_SUCCESS && pBuffer && nLen>0) {
		nRet = LoadBuffer(pBuffer,nLen);
	}
	return nRet;
}

int CParser::LoadBuffer(char *pBuffer,int nLen)
{
	int nRet = EXIT_SUCCESS;

	Empty();
	if(!pBuffer || nLen<=0)
		nRet = EXIT_FAILURE;
	else {
		int nStart,nEnd;

		nStart = 0;
		while(nStart<nLen) {
			
			// search start of line
			while(nStart<nLen && isspace(pBuffer[nStart]))
				nStart++;

			nEnd = nStart;
			if(pBuffer[nStart]=='<') {
				while(nEnd<nLen && pBuffer[nEnd-1]!='/' && pBuffer[nEnd]!='>')
					nEnd++;
			} else {
				while(nEnd<nLen && pBuffer[nEnd]!='\n' && pBuffer[nEnd]!='\r')
					nEnd++;
			}

			if(nEnd==nStart || pBuffer[nStart]=='#') {
				while(nEnd<nLen && pBuffer[nEnd]!='\n' && pBuffer[nEnd]!='\r')
					nEnd++;
			}
			
			if(pBuffer[nStart]=='<') { 
				if(pBuffer[nEnd-1]=='/' && pBuffer[nEnd]=='>') NewTokenString(&pBuffer[nStart+1],(nEnd-nStart-2));
			}

			nStart = nEnd+1;
		}
	}

	return nRet;
}
