#include "stdafx.h"
#include <squish.h>
#include "texturefile.h"

extern "C" {
	#include <dxtn.h>
}

const unsigned int CTextureFile::tplTexDescSize = 8;
const unsigned int CTextureFile::tplHdrSize = 12;
const unsigned int CTextureFile::tplPalDescSize = 12;
const unsigned int CTextureFile::tplImgDescSize = 36;
const unsigned int CTextureFile::tplVersion = 0x0020AF30;

using namespace squish;

int CTextureFile::tplPad[] = {0,0,0,0,0,0,0,0};

unsigned short CTextureFile::bitrev(unsigned short v)
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

CTextureFile::CTextureFile(CConverter::_tImage *tImages)
{
	m_nTexDescBlockSize = 0;
	m_nImgDescBlockSize = 0;
	m_nPalDescBlockSize = 0;
	m_nImgBankSize = 0;
	m_nPalBankSize = 0;
	m_nImageDescPad = 0;
	m_nPalDescPad = 0;

	m_nOutputFileSize = 0;
	m_pOutputFile = NULL;
	m_tImages = tImages;
}

CTextureFile::~CTextureFile()
{
	if(m_pOutputFile) fclose(m_pOutputFile);

	m_nOutputFileSize = 0;
	m_pOutputFile = NULL;
	m_tImages = NULL;
}

void CTextureFile::GetMinMag(CConverter::_tImage *tImage,int& nMin,int& nMag)
{
	nMin = 0;
	nMag = 0;

	if(!tImage) return;

	switch(tImage->nColFmt) {
		case TF_CI4:
		case TF_CI8:
			nMin = TF_FILTER_LINEAR;
			nMag = TF_FILTER_LINEAR;
			break;
		default:
			if((tImage->nMaxLOD-tImage->nMinLOD+1)==1) {
				nMin = TF_FILTER_LINEAR;
				nMag = TF_FILTER_LINEAR;
			} else {
				nMin = TF_FILTER_LIN_MIP_LIN;
				nMag = TF_FILTER_LINEAR;
			}
			break;
	}
}

int CTextureFile::Seek(int offset,int origin,FILE *pFile)
{
	FILE *outFile;

	if(!m_pOutputFile && !pFile) return -1;

	outFile = (pFile!=NULL?pFile:m_pOutputFile);
	return fseek(outFile,offset,origin);
}

int CTextureFile::WriteValue(void *pValue,int nType,int nLen,FILE *pFile)
{
	int nRet = 0;
	FILE *outFile;

	if((!m_pOutputFile && !pFile) || !pValue) return 0;
	
	outFile = (pFile!=NULL?pFile:m_pOutputFile);
	switch(nType) {
		case VALUE_TYPE_CHAR:
		{
			nRet = fwrite(pValue,1,sizeof(char),outFile);
		}
		break;
		case VALUE_TYPE_SHORT:
		{
			short nValue = *(short*)pValue;
#if BYTE_ORDER == LITTLE_ENDIAN
			nValue = SwapShort(nValue);
#endif
			nRet = fwrite(&nValue,1,sizeof(short),outFile);
		}
		break;
		case VALUE_TYPE_INT:
		{
			int nValue = *(int*)pValue;
#if BYTE_ORDER == LITTLE_ENDIAN
			nValue = SwapInt(nValue);
#endif
			nRet = fwrite(&nValue,1,sizeof(int),outFile);
		}
		break;
		case VALUE_TYPE_FLOAT:
		{
			float nValue = *(float*)pValue;
#if BYTE_ORDER == LITTLE_ENDIAN
			nValue = SwapFloat(nValue);
#endif
			nRet = (int)fwrite(&nValue,1,sizeof(float),outFile);
		}
		break;
		case VALUE_TYPE_STRING:
		{
			if(nLen==-1) nLen = strlen((const char*)pValue);
			nRet = fwrite(pValue,1,nLen,outFile);
		}
		break;
		case VALUE_TYPE_DATA:
		{
			if(nLen>0) nRet = fwrite(pValue,1,nLen,outFile);
		}
		break;
	}
	return nRet;
}

int CTextureFile::WriteFormatString(FILE *pFile,const char *pszFormat,...)
{
	va_list argList;
	char cBuffer[1024];
	FILE *outFile;

	if(!m_pOutputFile && !pFile) return 0;

	outFile = (pFile!=NULL?pFile:m_pOutputFile);

	va_start(argList, pszFormat);
	vsprintf(cBuffer,pszFormat,argList);
	va_end(argList);

	return fwrite(cBuffer,1,strlen(cBuffer),outFile);
}

int CTextureFile::GetNumTextures()
{
	int nTexs = 0;
	_tImage *tImgs = m_tImages;

	while(tImgs) {
		nTexs++;
		tImgs = tImgs->pNext;
	}
	return nTexs;
}

int CTextureFile::GetNumPalettes()
{
	int nPal = 0;
	_tImage *tImages = m_tImages;

	while(tImages) {
		switch(tImages->nColFmt) {
			case TF_CI4:
			case TF_CI8:		
				nPal++;
				break;
			default:
				break;
		}
		tImages = tImages->pNext;
	}
	return nPal;
}

int CTextureFile::ComputeTPLMipMapImageBufferSize(CConverter::_tImage *tImage)
{
	int nTotSize = 0;
	_tLayer *tLayers;
	
	if(!tImage) return 0;

	tLayers = tImage->GetLayers();
	while(tLayers) {
		nTotSize += ComputeTPLImageBufferSize(tLayers);
		tLayers = tLayers->next_p;
	}
	return nTotSize;
}

int CTextureFile::ComputeTPLImageBufferSize(CConverter::_tLayer *tLayer)
{
	int nSize = 0;

	if(!tLayer) return 0;

	switch(tLayer->colfmt) {
		case TF_I4:
			nSize = ComputeTPLImageBufferSize_4Bit(tLayer);
			break;
		case TF_I8:
			nSize = ComputeTPLImageBufferSize_8Bit(tLayer);
			break;
		case TF_IA4:
			nSize = ComputeTPLImageBufferSize_8Bit(tLayer);
			break;
		case TF_IA8:
			nSize = ComputeTPLImageBufferSize_16Bit(tLayer);
			break;
		case TF_CI4:
			nSize = ComputeTPLImageBufferSize_4Bit(tLayer);
			break;
		case TF_CI8:
			nSize = ComputeTPLImageBufferSize_8Bit(tLayer);
			break;
		case TF_RGB565:
			nSize = ComputeTPLImageBufferSize_16Bit(tLayer);
			break;
		case TF_RGB5A3:
			nSize = ComputeTPLImageBufferSize_16Bit(tLayer);
			break;
		case TF_RGBA8:
			nSize = ComputeTPLImageBufferSize_32Bit(tLayer);
			break;
		case TF_CMPR:
			nSize = ComputeTPLImageBufferSize_Cmpr(tLayer);
			break;
	}
	return nSize;
}

int CTextureFile::ComputeTPLImageBufferSize_4Bit(CConverter::_tLayer *tLayer)
{
	int nCols,nRows;
	int nWidth = 0,nHeight = 0;
	CImage *pImg;

	if(!tLayer) return 0;

	pImg = tLayer->img_p;
	if(pImg) {
		nWidth  = pImg->GetXSize();
		nHeight = pImg->GetYSize();
	}
	nCols = ((nWidth + 7) >> 3);
	nRows = ((nHeight + 7) >> 3);

	return (nCols*nRows*32);
}

int CTextureFile::ComputeTPLImageBufferSize_8Bit(CConverter::_tLayer *tLayer)
{
	int nCols,nRows;
	int nWidth = 0,nHeight = 0;
	CImage *pImg;

	if(!tLayer) return 0;

	pImg = tLayer->img_p;
	if(pImg) {
		nWidth  = pImg->GetXSize();
		nHeight = pImg->GetYSize();
	}
	nCols = ((nWidth + 7) >> 3);
	nRows = ((nHeight + 7) >> 2);

	return (nCols*nRows*32);
}

int CTextureFile::ComputeTPLImageBufferSize_16Bit(CConverter::_tLayer *tLayer)
{
	int nCols,nRows;
	int nWidth = 0,nHeight = 0;
	CImage *pImg;

	if(!tLayer) return 0;

	pImg = tLayer->img_p;
	if(pImg) {
		nWidth  = pImg->GetXSize();
		nHeight = pImg->GetYSize();
	}
	nCols = ((nWidth + 3) >> 2);
	nRows = ((nHeight + 3) >> 2);

	return (nCols*nRows*32);
}

int CTextureFile::ComputeTPLImageBufferSize_32Bit(CConverter::_tLayer *tLayer)
{
	int nCols,nRows;
	int nWidth = 0,nHeight = 0;
	CImage *pImg;

	if(!tLayer) return 0;

	pImg = tLayer->img_p;
	if(pImg) {
		nWidth  = pImg->GetXSize();
		nHeight = pImg->GetYSize();
	}
	nCols = ((nWidth + 3) >> 2);
	nRows = ((nHeight + 3) >> 2);

	return (nCols*nRows*32*2);
}

int CTextureFile::ComputeTPLImageBufferSize_Cmpr(CConverter::_tLayer *tLayer)
{
	int nCols,nRows;
	int nWidth = 0,nHeight = 0;
	CImage *pImg;

	if(!tLayer) return 0;

	pImg = tLayer->img_p;
	if(pImg) {
		nWidth  = pImg->GetXSize();
		nHeight = pImg->GetYSize();
	}
	nCols = ((nWidth + 7) >> 3);
	nRows = ((nHeight + 7) >> 3);

	return (nCols*nRows*32);
}

int CTextureFile::ComputeFilterModeByDimension(CConverter::_tImage *tImage)
{
	int i;
	int xres,yres;
	int *p_dim;

	if(!tImage) return 0;
	if(tImage->nWrapS==-1 && tImage->nWrapT==-1) {
		xres = tImage->pImage->GetXSize();
		yres = tImage->pImage->GetYSize();
		p_dim = &xres;
		
		for(i=0;i<2;i++) {
			switch(*p_dim) {
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
					*p_dim = TF_WRAP_REPEAT;
					break;
				default:
					*p_dim = TF_WRAP_CLAMP;
					break;
			}
			p_dim = &yres;
		}
		if(xres==yres) return xres;
	}
	return TF_WRAP_CLAMP;
}

void CTextureFile::ComputeTPLSize()
{
	int nSize;
	int nTexs,nPal;

	nPal = GetNumPalettes();
	nTexs = GetNumTextures();
	m_nTexDescBlockSize = nTexs*tplTexDescSize;
	m_nImgDescBlockSize = nTexs*tplImgDescSize;	
	m_nPalDescBlockSize = nPal*tplPalDescSize;	

	nSize = tplHdrSize+m_nTexDescBlockSize+m_nImgDescBlockSize;
	if(nSize<32) m_nImageDescPad = 32-nSize;
	else if(nSize%32) m_nImageDescPad = 32-(nSize%32);

	nSize = tplHdrSize+m_nTexDescBlockSize+m_nImgDescBlockSize+m_nImageDescPad+m_nPalDescBlockSize;
	if(nSize<32) m_nPalDescPad = 32-nSize;
	else if(nSize%32) m_nPalDescPad = 32-(nSize%32);

	SetTPLTextureValues();
	SetTPLPaletteValues();
}

void CTextureFile::SetTPLTextureValues()
{
	int nPos;
	int nBankOffset;
	int nImgDescOffset;
	_tImage *tImages;

	nImgDescOffset = tplHdrSize+m_nTexDescBlockSize;
	nBankOffset = tplHdrSize+m_nTexDescBlockSize+m_nImgDescBlockSize+m_nImageDescPad+m_nPalDescBlockSize+m_nPalDescPad;

	nPos = 0;
	m_nImgBankSize = 0;
	tImages = m_tImages;
	while(tImages) {
		tImages->nImageDataOffset = nBankOffset;
		tImages->nImageDataLen = ComputeTPLMipMapImageBufferSize(tImages);
		tImages->nImageDescOffset = nImgDescOffset+(nPos*tplImgDescSize);

		nBankOffset += tImages->nImageDataLen;
		m_nImgBankSize += tImages->nImageDataLen;

		tImages = tImages->pNext;
		nPos++;
	}
}

void CTextureFile::SetTPLPaletteValues()
{
	int nColFmt;
	int nPalDescOffset;
	int nPos,nEntrySize;
	int nBankOffset,nCols;
	CImage *pImg;
	RGBQUAD *pPal;
	_tImage *tImages;

	nPalDescOffset = tplHdrSize+m_nTexDescBlockSize+m_nImgDescBlockSize+m_nImageDescPad;
	nBankOffset = tplHdrSize+m_nTexDescBlockSize+m_nImgDescBlockSize+m_nImageDescPad+m_nPalDescBlockSize+m_nPalDescPad+m_nImgBankSize;

	nPos = 0;
	m_nPalBankSize = 0;
	tImages = m_tImages;
	while(tImages) {
		nColFmt = tImages->nColFmt;
		switch(nColFmt) {
			case TF_CI4:
			case TF_CI8:		
			{
				tImages->nPalDataOffset = nBankOffset;
				tImages->nPalDescOffset = nPalDescOffset+(nPos*tplPalDescSize);
				switch(tImages->nPalFmt) {
					case TF_TLUT_RGB565:
					case TF_TLUT_RGB5A3:
						nEntrySize = 2;
						break;
					default:
						return;
						break;
				}

				nCols = 0;
				pPal = NULL;
				if(tImages->pImage) {
					pImg = tImages->pImage;
					if(nColFmt==TF_CI4)
						nCols = pImg->GetPalettized(NULL,&pPal,16);
					else 
						nCols = pImg->GetPalettized(NULL,&pPal,256);

					if(pPal && nCols>0 && nCols<=16384) {
						tImages->nPalCols = nCols;
						tImages->pPal = pPal;

						tImages->nPalDataLen = ((nCols+15)&0xfff0)*nEntrySize;
					}
				}
			}
			break;

			default:
				break;
		}
		
		nBankOffset += tImages->nPalDataLen;
		m_nPalBankSize += tImages->nPalDataLen;

		tImages = tImages->pNext;
		nPos++;
	}
}

void* CTextureFile::SquishCompress(CImage *pImage,int nFlags)
{
	bool aType;
	int i,j,x,y,px,py;
	int nXSize,nYSize;
	int nStride,nBytesPerBlock;
	int nTargetSize;
	BYTE *bits;
	char *pTarget,*pBuf;

	aType = pImage->IsTransparent();
	nXSize = pImage->GetXSize();
	nYSize = pImage->GetYSize();
	nStride = 4;

	nBytesPerBlock = (((nFlags&kDxt1)!=0)?8:16);
	nTargetSize = (nBytesPerBlock*nXSize*nYSize/16);
	pTarget = new char[nTargetSize];

	pBuf = pTarget;
	bits = pImage->GetPixelRGBA();
	for(y=0;y<nYSize;y+=4) {
		for(x=0;x<nXSize;x+=4) {
			unsigned char srcRgba[16*4];
			for(py=0,i=0;py<4;py++) {
				const char *row = (char*)(&bits[((y+py)*(nXSize<<2))+(x<<2)]);
				for(px=0;px<4;px++,i++) {
					for(j=0;j<3;j++)
						srcRgba[4*i+j] = *row++;

					if(aType==false)
						srcRgba[4*i+3] = 0xff;
					else
						srcRgba[4*i+3] = *row++;
				}
			}
			Compress(srcRgba,pBuf,nFlags);
			pBuf += nBytesPerBlock;
		}
	}
	delete [] bits;

	return pTarget;
}

void* CTextureFile::DXTCompress(CImage *pImage,int nCompType)
{
	int nXSize,nYSize;
	BYTE *bits;
	void *pBuf = NULL;

	nXSize = pImage->GetXSize();
	nYSize = pImage->GetYSize();
	if((nXSize%4)!=0 || (nYSize%4)!=0) return NULL;

	switch(nCompType) {
		case CMPR_TYPE_TCX:
			bits = pImage->GetPixelRGBA();
			pBuf = (void*)new unsigned short[(nXSize*nYSize)>>2];
			tx_compress_dxtn(4,nXSize,nYSize,(unsigned char*)bits,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,(GLubyte*)pBuf);
			delete [] bits;
			break;
		case CMPR_TYPE_SQUISH:
			pBuf = SquishCompress(pImage,kDxt1);
			break;
		default:
			pBuf = NULL;
			break;
	}
	return pBuf;
}
