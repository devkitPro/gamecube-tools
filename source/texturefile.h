#ifndef __TEXTUREFILE_H__
#define __TEXTUREFILE_H__

#include "image.h"
#include "converter.h"

#define VALUE_TYPE_CHAR		0
#define VALUE_TYPE_SHORT	1
#define VALUE_TYPE_INT		2
#define VALUE_TYPE_FLOAT	3
#define VALUE_TYPE_STRING	4
#define VALUE_TYPE_DATA		5

#define CMPR_TYPE_TCX		0
#define CMPR_TYPE_SQUISH	1

class CParser;

class CTextureFile
{
public:
	friend class CConverter;
	typedef CConverter::_tImage _tImage;
	typedef CConverter::_tLayer _tLayer;
public:
	CTextureFile(_tImage *tImages);
	virtual ~CTextureFile();

	virtual int Write(CParser *pParser) = 0;

protected:
	int GetNumTextures();
	int GetNumPalettes();
	int Seek(int offset,int origin,FILE *pFile = NULL);
	int WriteValue(void *pValue,int nType,int nLen = -1,FILE *pFile = NULL);
	int WriteFormatString(FILE *pFile,const char *pszFormat,...);
	
	void ComputeTPLSize();
	void SetTPLTextureValues();
	void SetTPLPaletteValues();

	int ComputeFilterModeByDimension(_tImage *tImage);
	int ComputeTPLMipMapImageBufferSize(_tImage *tImage);
	int ComputeTPLImageBufferSize(_tLayer *tLayer);

	int ComputeTPLImageBufferSize_4Bit(_tLayer *tLayer);
	int ComputeTPLImageBufferSize_8Bit(_tLayer *tLayer);
	int ComputeTPLImageBufferSize_16Bit(_tLayer *tLayer);
	int ComputeTPLImageBufferSize_32Bit(_tLayer *tLayer);
	int ComputeTPLImageBufferSize_Cmpr(_tLayer *tLayer);

	void* DXTCompress(CImage *pImage,int nCompType);
	void GetMinMag(_tImage *tImage,int& nMin,int& nMag);
	unsigned short bitrev(unsigned short v);

private:
	void* SquishCompress(CImage *pImage,int nFlags);
	
public:
	static const unsigned int tplTexDescSize;
	static const unsigned int tplHdrSize;
	static const unsigned int tplImgDescSize;
	static const unsigned int tplVersion;
	static const unsigned int tplPalDescSize;

	static int tplPad[8];

protected:
	int m_nImageDescPad;
	int m_nPalDescPad;
	int m_nTexDescBlockSize;
	int m_nPalDescBlockSize;
	int m_nImgDescBlockSize;
	int m_nImgBankSize;
	int m_nPalBankSize;

	int m_nOutputFileSize;
	FILE *m_pOutputFile;
	_tImage *m_tImages;
};

#endif
