#ifndef __CONVERTER_H__
#define __CONVERTER_H__

#include "parser.h"

#include <vector>

#define TF_I4									0
#define TF_I8									1
#define TF_IA4									2
#define TF_IA8									3
#define TF_RGB565								4
#define TF_RGB5A3								5
#define TF_RGBA8								6
#define TF_CI4									8
#define TF_CI8									9
#define TF_CMPR									14

/* tex filter */
#define TF_FILTER_NEAR							0
#define TF_FILTER_LINEAR						1
#define TF_FILTER_NEAR_MIP_NEAR					2
#define TF_FILTER_LIN_MIP_NEAR					3
#define TF_FILTER_NEAR_MIP_LIN					4
#define TF_FILTER_LIN_MIP_LIN					5

#define TF_WRAP_CLAMP							0
#define TF_WRAP_REPEAT							1
#define TF_WRAP_MIRROR							2

#define TF_TLUT_IA8								0
#define TF_TLUT_RGB565							1
#define TF_TLUT_RGB5A3							2

#define CHECK_RANGE_LOD(lod)			(((lod)<0)||((lod)>10)?(0):(1))

class CImage;

class CConverter
{
public:
	struct _tsImage {
		_tsImage(const char *pszImagename,CImage *pImg);
		~_tsImage();

		char *pszImageName;
		CImage *pImage;
	};
	
	struct _tLayer {
		_tLayer(int nColFmt,CImage *pImage);
		_tLayer(int nColFmt,int nWidth,int nHeight);
		~_tLayer();
		
		void BoxFilter(CImage *pSrcImg);
		CImage* GetImage() {return img_p;}

		int width;
		int height;
		int colfmt;
		CImage *img_p;
		struct _tLayer *next_p;	
	};

	struct _tImage {
		_tImage();
		~_tImage();

		void ReleaseAll();

		const char* GetFmtString();
	
		_tLayer* AddLayer(CImage *pImage);
		_tLayer* AddLayer(int nWidth,int nHeight);
		_tLayer* GetLayers() {return pLayers;}
		
		void SetID(const char *pszId);
		void SetColorFmt(int nColorFmt) {nColFmt = nColorFmt;}
		void SetPaletteFmt(int nPaletteFmt) {nPalFmt = nPaletteFmt;}
		void SetLOD(int minLOD,int maxLOD,int remapLOD)
		{
			nMinLOD = minLOD;
			nMaxLOD = maxLOD;
			nRemapLOD = remapLOD;
		}

		void SetWrapST(int wrapS,int wrapT)
		{
			nWrapS = wrapS;
			nWrapT = wrapT;
		}

		int nMinLOD,nMaxLOD;
		int nRemapLOD,nColFmt;
		int nWrapS,nWrapT;
		int nPalCols;
		int nPalFmt;
		int nPalDescOffset;
		int nPalDataOffset;
		int nPalDataLen;
		int nImageDescOffset;
		int nImageDataOffset;
		int nImageDataLen;
		char *pszID;
		RGBQUAD *pPal;
		CImage *pImage;			//this is the source image, it's 32Bit.
		_tLayer *pLayers;
		struct _tImage *pNext;
	};

public:
	CConverter(CParser *pParser = NULL);
	virtual ~CConverter();

	int WriteTextures();
	int GenerateTextures();
	int GenerateTexture(CParser::_ttokenstringlist *pEntry);
	void ReleaseAll();
	_tImage* GetImages() const { return m_pImages; }
	
private:
	void ReallocImgArray();
	CImage* GetImage(const char *pszImagename,int& nRet);
	int GenerateMipMaps(_tImage *tImage);
	int GenerateImage(_tImage *tImage);
	_tImage* AddImage(CImage *pImg);
	_tsImage* AddImage(const char *pszImagename,int& nRet);
	int ComputeMipMapBufferSize();
	int CheckPow2(_tImage *tImage);
	int CheckReduction(_tImage *tImage);
	void CreateNextMipMapLevel(_tImage *tImage,int nWidth,int nHeight);
	void BoxFilter(CImage *pSrcImg,CImage *pDstImg,unsigned int dstRow,unsigned int dstCol,unsigned int boxWid,unsigned int boxHgt);
	unsigned short bitrev(unsigned short v);

	int m_nMaxSrcImgAlloc;
	int m_nSrcImages;
	_tsImage **m_ppSrcImages;
	_tImage *m_pImages;
	CParser *m_pParser;
	std::vector<std::string> m_Deps;
};

#endif
