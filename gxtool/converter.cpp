#include "stdafx.h"
#include "image.h"
#include "tfbinarytdf.h"
#include "converter.h"

#define CHECK_RANGE_LOD(lod)			(((lod)<0)||((lod)>10)?(0):(1))

CConverter::_tsImage::_tsImage(const char *pszImagename,CImage *pImg)
{
	int nLen = (int)strlen(pszImagename);

	pszImageName = new char[nLen+1];
	if(pszImageName) strcpy(pszImageName,pszImagename);

	pImage = pImg;
}

CConverter::_tsImage::~_tsImage()
{
	if(pszImageName) delete [] pszImageName;
	if(pImage) delete pImage;
}

CConverter::_tLayer::_tLayer(int nColFmt,int nWidth,int nHeight)
{
	width = nWidth;
	height = nHeight;
	colfmt = nColFmt;
	next_p = NULL;
	img_p = NULL;
}

CConverter::_tLayer::_tLayer(int nColFmt,CImage *pImage)
{
	width = pImage->GetXSize();
	height = pImage->GetYSize();
	colfmt = nColFmt;
	next_p = NULL;

	img_p = new CImage(*pImage);
}

CConverter::_tLayer::~_tLayer()
{
	if(img_p) delete img_p;

	width = 0;
	height = 0;
	img_p = NULL;
	next_p = NULL;
}

void CConverter::_tLayer::BoxFilter(CImage *pSrcImg)
{
	if(pSrcImg==NULL) return;

	if(img_p!=NULL) delete img_p;

	img_p = pSrcImg->BoxFilter(width,height);
}

CConverter::_tImage::_tImage()
{
	nMinLOD = 0;
	nMaxLOD = 0;
	nRemapLOD = 0;
	nWrapT = -1;
	nWrapS = -1;
	nColFmt = -1;
	nPalFmt = -1;
	nImageDataLen = 0;
	nImageDescOffset = 0;
	nImageDataOffset = 0;
	nPalDescOffset = 0;
	nPalDataOffset = 0;
	nPalDataLen = 0;
	nPalCols = 0;
	pszID = NULL;
	pLayers = NULL;
	pImage = NULL;
	pPal = NULL;
	pNext = NULL;
}

CConverter::_tImage::~_tImage()
{
	ReleaseAll();

	nMinLOD = 0;
	nMaxLOD = 0;
	nRemapLOD = 0;
	nWrapT = -1;
	nWrapS = -1;
	nColFmt = -1;
	nPalFmt = -1;
	nImageDataLen = 0;
	nImageDescOffset = 0;
	nImageDataOffset = 0;
	nPalDescOffset = 0;
	nPalDataOffset = 0;
	nPalDataLen = 0;
	pszID = NULL;
	pLayers = NULL;
	pImage = NULL;
	pPal = NULL;
	pNext = NULL;
}

void CConverter::_tImage::ReleaseAll()
{
	_tLayer *ptr;

	if(pszID) delete [] pszID;
	if(pPal) delete [] pPal;
	while(pLayers) {
		ptr = pLayers;
		pLayers = pLayers->next_p;
		delete ptr;
	}
}

CConverter::_tLayer* CConverter::_tImage::AddLayer(int nWidth,int nHeight)
{
	_tLayer *ptr = NULL,*ptr2 = pLayers;

	ptr = new _tLayer(nColFmt,nWidth,nHeight);
	
	while(ptr2 && ptr2->next_p) ptr2 = ptr2->next_p;

	if(ptr2) ptr2->next_p = ptr;
	else pLayers = ptr;

	return ptr;
}

CConverter::_tLayer* CConverter::_tImage::AddLayer(CImage *pImage)
{
	_tLayer *ptr = NULL,*ptr2 = pLayers;

	ptr = new _tLayer(nColFmt,pImage);
	
	while(ptr2 && ptr2->next_p) ptr2 = ptr2->next_p;

	if(ptr2) ptr2->next_p = ptr;
	else pLayers = ptr;

	return ptr;
}

void CConverter::_tImage::SetID(const char *pszId)
{
	if(pszID) delete [] pszID;
	pszID = new char[strlen(pszId)+1];
	strcpy(pszID,pszId);
}

const char* CConverter::_tImage::GetFmtString()
{
	switch(nColFmt) {
		case TF_RGB565:
			return "RGB565";
		case TF_RGB5A3:
			return "RGB5A3";
		case TF_RGBA8:
			return "RGBA8";
		case TF_CMPR:
			return "CMPR";
		default:
			return "<unknown>";
	}
}

unsigned short CConverter::bitrev(unsigned short v)
{
	unsigned short res;
	res = ((v&0x03)<<6) |
		  ((v&0x0C)<<2) |
		  ((v&0x30)>>2) |
		  ((v&0xC0)>>6) |

		  ((v&0x0300)<<6) |
		  ((v&0x0C00)<<2) |
		  ((v&0x3000)>>2) |
		  ((v&0xC000)>>6) ;
	return res;
}

CConverter::CConverter(CParser *pParser)
{
	m_nMaxSrcImgAlloc = 0;
	m_nSrcImages = 0;
	m_pImages = NULL;
	m_ppSrcImages = NULL;
	m_pParser = pParser;
}

CConverter::~CConverter()
{
	ReleaseAll();

	m_nMaxSrcImgAlloc = 0;
	m_nSrcImages = 0;
	m_pImages = NULL;
	m_pParser = NULL;
	m_ppSrcImages = NULL;
}

void CConverter::ReleaseAll()
{
	int i;
	_tImage *ptr;

	if(m_ppSrcImages) {
		for(i=0;i<m_nMaxSrcImgAlloc;i++) {
			if(m_ppSrcImages[i]) delete m_ppSrcImages[i];
		}
		delete [] m_ppSrcImages;

	}

	while(m_pImages) {
		ptr = m_pImages;
		m_pImages = m_pImages->pNext;
		delete ptr;
	}
}

CConverter::_tImage* CConverter::AddImage(CImage *pImg)
{
	_tImage *ptr = NULL;
	_tImage *ptr2 = m_pImages;

	ptr = new _tImage;

	while(ptr2 && ptr2->pNext) ptr2 = ptr2->pNext;
	if(ptr2) ptr2->pNext = ptr;
	else m_pImages = ptr;

	if(ptr && pImg) ptr->pImage = pImg;

	return ptr;
}

CConverter::_tsImage* CConverter::AddImage(const char *pszImagename,int& nRet)
{
	_tsImage *ptr = NULL;
	CImage *pImg = new CImage();
	
	string imagePath = m_pParser->GetScriptPath();
	imagePath += pszImagename;

	nRet = pImg->Load(imagePath.c_str());
	if(nRet!=1) return NULL;

	if(pImg) {
		m_ppSrcImages[m_nSrcImages] = new _tsImage(pszImagename,pImg);
		ptr = m_ppSrcImages[m_nSrcImages++];
	}
	
	m_Deps.push_back(imagePath);

	return ptr;
}

int CConverter::GenerateTexture(CParser::_ttokenstringlist *pEntry)
{
	int nRet,nMaxLOD;
	int nMinLOD,nRemapLOD;
	int nColFmt,nSizeX,nSizeY;
	CImage *pImg;
	_tImage *pImage;

	if(!pEntry) return EXIT_FAILURE;

	pImg = GetImage(pEntry->GetTokenValue("filepath"),nRet);
	if(!pImg || !nRet) {
		fprintf(stderr,"error loading %s.\n",pEntry->GetTokenValue("filepath"));
		return EXIT_FAILURE;
	}

	nSizeX = atoi(pEntry->GetTokenValue("width","-1"));
	nSizeY = atoi(pEntry->GetTokenValue("height","-1"));
	if(nSizeX!=-1 && nSizeY!=-1) {
		if(nSizeX!=pImg->GetXSize() || nSizeY!=pImg->GetYSize()) pImg->Resize(nSizeX,nSizeY); 
	}

	pImage = NULL;
	pImage = AddImage(pImg);
	if(pImage) {
		pImage->SetID(pEntry->GetTokenValue("id","0"));

		nColFmt = atoi(pEntry->GetTokenValue("colfmt","6"));
		pImage->SetColorFmt(nColFmt);
		switch(nColFmt) {
			case TF_CI4:
			case TF_CI8:		
				pImage->SetPaletteFmt(atoi(pEntry->GetTokenValue("palfmt","1")));
				break;
			default:
				break;
			
		}

		if(strcasecmp(pEntry->GetTokenValue("mipmap","no"),"yes")==0) {
			nMinLOD = atoi(pEntry->GetTokenValue("minlod","0"));
			nMaxLOD = atoi(pEntry->GetTokenValue("maxlod","0"));
			nRemapLOD = atoi(pEntry->GetTokenValue("remaplod","0"));
			pImage->SetLOD(nMinLOD,nMaxLOD,nRemapLOD);
			GenerateMipMaps(pImage);
		} else
			GenerateImage(pImage);
	}

	return EXIT_SUCCESS;
}

int CConverter::GenerateTextures()
{
	int nRet,nMaxLOD;
	int nMinLOD,nRemapLOD;
	int nColFmt,nSizeX,nSizeY;
	CImage *pImg;
	_tImage *pImage;
	const CParser::_ttokenstringlist *pList;

	if(!m_pParser) return EXIT_FAILURE;

	pList = m_pParser->GetEntries();
	while(pList) {
		pImg = GetImage(pList->GetTokenValue("filepath"),nRet);
		if(!pImg || !nRet) {
			fprintf(stderr,"error loading %s.\n",pList->GetTokenValue("filepath"));
			return EXIT_FAILURE;
		}

		nSizeX = atoi(pList->GetTokenValue("xsize","-1"));
		nSizeY = atoi(pList->GetTokenValue("ysize","-1"));
		if(nSizeX!=-1 && nSizeY!=-1) {
			if(nSizeX!=pImg->GetXSize() || nSizeY!=pImg->GetYSize()) pImg->Resize(nSizeX,nSizeY); 
		}

		pImage = NULL;
		pImage = AddImage(pImg);
		if(pImage) {
			pImage->SetID(pList->GetTokenValue("id","0"));

			nColFmt = atoi(pList->GetTokenValue("colfmt","6"));
			pImage->SetColorFmt(nColFmt);
			switch(nColFmt) {
				case TF_CI4:
				case TF_CI8:		
					pImage->SetPaletteFmt(atoi(pList->GetTokenValue("palfmt","1")));
					break;
				default:
					break;
				
			}

			if(strcasecmp(pList->GetTokenValue("mipmap","no"),"yes")==0) {
				nMinLOD = atoi(pList->GetTokenValue("minlod","0"));
				nMaxLOD = atoi(pList->GetTokenValue("maxlod","0"));
				nRemapLOD = atoi(pList->GetTokenValue("remaplod","0"));
				pImage->SetLOD(nMinLOD,nMaxLOD,nRemapLOD);
				GenerateMipMaps(pImage);
			} else
				GenerateImage(pImage);
		}

		pList = pList->pNext;
	}

	return EXIT_SUCCESS;
}

int CConverter::GenerateMipMaps(CConverter::_tImage *tImage)
{
	int nRet = 0;
	int nDstWidth,nDstHeight;	
	int check,numLOD,level;
	int nMinLOD,nMaxLOD,nRemapLOD;

	if(!tImage) return EXIT_FAILURE;

	nMinLOD = tImage->nMinLOD;
	nMaxLOD = tImage->nMaxLOD;
	nRemapLOD = tImage->nRemapLOD;
	if(!CHECK_RANGE_LOD(nMinLOD)) return EXIT_FAILURE;
	if(!CHECK_RANGE_LOD(nMaxLOD)) return EXIT_FAILURE;
	if(!CHECK_RANGE_LOD(nRemapLOD)) return EXIT_FAILURE;

	if(nMinLOD>nMaxLOD) return 0;

	check = nRemapLOD+(nMaxLOD-nMinLOD);
	if(check>10) return EXIT_FAILURE;

	check = CheckPow2(tImage);
	if(!check) return EXIT_FAILURE;

	check = CheckReduction(tImage);
	if(!check) return EXIT_FAILURE;

	numLOD = nMaxLOD-nMinLOD+1;
	for(level=nMinLOD;level<(nMinLOD+numLOD);level++) {
		nDstWidth = tImage->pImage->GetXSize()>>level;
		nDstHeight = tImage->pImage->GetYSize()>>level;

		CreateNextMipMapLevel(tImage,nDstWidth,nDstHeight);
	}
	
	return nRet;
}

int CConverter::GenerateImage(CConverter::_tImage *tImage)
{
	int nDstWidth,nDstHeight;	

	if(!tImage) return EXIT_FAILURE;

	nDstWidth = tImage->pImage->GetXSize();
	nDstHeight = tImage->pImage->GetYSize();
	CreateNextMipMapLevel(tImage,nDstWidth,nDstHeight);

	return EXIT_SUCCESS;
}

int CConverter::CheckPow2(CConverter::_tImage *tImage)
{
	int i,size;

	if(!tImage) return 0;

	if(!(tImage->nMinLOD || tImage->nMaxLOD || tImage->nRemapLOD)) {
		if(tImage->pImage->GetXSize()>1024 || tImage->pImage->GetYSize()>1024) return 0;
		return 1;
	}

	for(i=0;i<2;i++) {
		if(!i) size = tImage->pImage->GetXSize();
		else size = tImage->pImage->GetYSize();

		switch(size) {
			case 1024:
			case  512:
			case  256:
			case  128:
			case   64:
			case   32:
			case   16:
			case    8:
			case    4:
			case    2:
			case    1:
				break;
			default:
				return 0;
		}
	}
	return 1;
}

int CConverter::CheckReduction(CConverter::_tImage *tImage)
{
	int scale,checkWt,checkHt;
	
	if(!tImage) return 0;

	scale = tImage->nMaxLOD;
	if(!scale) return 1;

	checkWt = tImage->pImage->GetXSize()>>scale;
	checkHt = tImage->pImage->GetYSize()>>scale;
	if(!checkWt || !checkHt) return 0;

	return 1;
}

void CConverter::CreateNextMipMapLevel(CConverter::_tImage *tImage,int nWidth,int nHeight)
{
	_tLayer *pLayer;

	if(!tImage) return;

	pLayer = tImage->AddLayer(nWidth,nHeight);
	if(pLayer) pLayer->BoxFilter(tImage->pImage);
}

int CConverter::WriteTextures()
{
	int nRet;
	CTextureFile *pFile = NULL;

	if(!m_pParser) return EXIT_FAILURE;

	nRet = EXIT_FAILURE;
	if(m_pImages) {
		pFile = new CTFBinaryTDF(m_pImages);
		if(pFile) {
			nRet = pFile->Write(m_pParser);
			delete pFile;
		}
		string depsFilename = m_pParser->GetDepsFilename();
		if ( !depsFilename.empty()) {
			FILE *depsFile = fopen(depsFilename.c_str(),"wb");
			if (depsFile) {
				fprintf(depsFile,"%s: ", m_pParser->GetOutputFilename());
				fprintf(depsFile,"\\\n %s ",m_pParser->GetInputFilename());
				for(unsigned i=0; i<m_Deps.size(); i++) {
					fprintf(depsFile,"\\\n  %s ",m_Deps[i].c_str());
				}
				fprintf(depsFile,"\n");
				fclose(depsFile);
			}
			m_Deps.clear();
		}
	}
	return nRet;
}

CImage* CConverter::GetImage(const char *pszImagename,int& nRet)
{
	int i;
	_tsImage *ptr = NULL;
	CImage *pImg = NULL;

	if(!pszImagename) return NULL;
	if(m_nSrcImages>=m_nMaxSrcImgAlloc) ReallocImgArray();


	nRet = 0;
	for(i=0;i<m_nSrcImages;i++) {
		if(m_ppSrcImages[i] && m_ppSrcImages[i]->pszImageName) {
			if(strcasecmp(m_ppSrcImages[i]->pszImageName,pszImagename)==0) {
				ptr = m_ppSrcImages[i];
				nRet = 1;
				break;
			}
		}
	}
	if(!ptr) ptr = AddImage(pszImagename,nRet);
	if(ptr) pImg = ptr->pImage;

	return pImg;
}

void CConverter::ReallocImgArray()
{
	_tsImage **ppImages;

	ppImages = new _tsImage*[m_nMaxSrcImgAlloc+20];
	if(ppImages){
		memset(ppImages,0,(m_nMaxSrcImgAlloc+20)*sizeof(_tsImage*));
		if(m_ppSrcImages) {
			memcpy(ppImages,m_ppSrcImages,m_nMaxSrcImgAlloc);
			delete [] m_ppSrcImages;
		}
	}
	m_ppSrcImages = ppImages;
	m_nMaxSrcImgAlloc += 20;
}
