#include "stdafx.h"
#include "tfbinary.h"

using namespace std;

CTFBinary::CTFBinary(_tImage *tImages) : CTextureFile(tImages)
{
}

CTFBinary::~CTFBinary()
{
}

int CTFBinary::Write(CParser *pParser)
{
	int nRet,nIdx;
	_tImage *tImages;

	if(pParser==NULL) return EXIT_FAILURE;

	nIdx = 0;
	nRet = EXIT_SUCCESS;
	tImages = m_tImages;
	while(tImages) {
		WriteTexHeader(tImages);
		switch(tImages->nColFmt) {
			case TF_I4:
				WriteTexture_I4(tImages);
				break;
			case TF_I8:
				WriteTexture_I8(tImages);
				break;
			case TF_IA4:
				WriteTexture_IA4(tImages);
				break;
			case TF_IA8:
				WriteTexture_IA8(tImages);
				break;
			case TF_CI4:
				WriteTexture_CI4(tImages);
				break;
			case TF_CI8:
				WriteTexture_CI8(tImages);
				break;
			case TF_RGB565:
				WriteTexture_RGB565(tImages);
				break;
			case TF_RGB5A3:
				WriteTexture_RGB5A3(tImages);
				break;
			case TF_RGBA8:
				WriteTexture_RGBA8(tImages);
				break;
			case TF_CMPR:
				WriteTexture_CMPR(tImages);
				break;
		}
		tImages = tImages->pNext;
		m_pOutputFile = NULL;
	}
	return nRet;
}

int CTFBinary::WriteTexture_I4(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria,ucTmp;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	nRet = 0;
	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=8) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<8;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = (bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE])/3;
						if(!(ix%2)) ucTmp = ria&0xf0;
						else {
							ucTmp |= _SHIFTR(ria,4,4);
							nRet += WriteValue(&ucTmp,VALUE_TYPE_CHAR,1,pFile);
						}
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return nRet;
}

int CTFBinary::WriteTexture_I8(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	nRet = 0;
	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = (bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE])/3;
						nRet += WriteValue(&ria,VALUE_TYPE_CHAR,1,pFile);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return nRet;
}

int CTFBinary::WriteTexture_IA4(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria,a,ucTmp;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	nRet = 0;
	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<8;++ix) {
						if(pImg->IsTransparent()) a = bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_ALPHA];
						else a = 0xf0;
						
						ria = (bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE])/3;
						ucTmp = (a&0xf0)|_SHIFTR(ria,4,4);
						nRet += WriteValue(&ucTmp,VALUE_TYPE_CHAR,1,pFile);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return nRet;
}

int CTFBinary::WriteTexture_IA8(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short sTmp;
	unsigned char ria,a;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	nRet = 0;
	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=4) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<4;++ix) {
						if(pImg->IsTransparent()) a = bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_ALPHA];
						else a = 0xff;
						
						ria = (bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]+bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE])/3;
						sTmp = _SHIFTL(a,8,8)|(ria&0xff);
						nRet += WriteValue(&sTmp,VALUE_TYPE_SHORT,2,pFile);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return nRet;
}

int CTFBinary::WriteTexture_CI4(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres,cols;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned char uctmp,ria,*pPix;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	WritePaletteBlock(tImage,16);

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();
		cols = pImg->GetPalettized(&pPix,NULL,16);

		for(y=0;y<yres;y+=8) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<8;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = pPix[((y+iy)*xres+(x+ix))];
						
						if(!(ix%2)) uctmp = _SHIFTL(ria,4,4);
						else {
							uctmp |= (ria&0xf);
							WriteValue(&uctmp,VALUE_TYPE_CHAR,1,pFile);
						}
					}
				}
			}
		}
		delete [] pPix;
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexture_CI8(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres,cols;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned char *pPix,ria;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	WritePaletteBlock(tImage,256);

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();
		cols = pImg->GetPalettized(&pPix,NULL,256);

		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = pPix[((y+iy)*xres+(x+ix))];
						WriteValue(&ria,VALUE_TYPE_CHAR,1,pFile);
					}
				}
			}
		}
		delete [] pPix;
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexture_RGB565(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short color;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();
		
		pImg->DiffuseError(0,5,6,5);

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=4) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<4;++ix) {
						color = (unsigned short)(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>3),11,5))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>2),5,6))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>3),0,5));
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexture_RGB5A3(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short color;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		if(!pImg->IsTransparent()) {
			bits = pImg->GetPixel();
			for(y=0;y<yres;y+=4) {
				for(x=0;x<xres;x+=4) {
					for(iy=0;iy<4;++iy) {
						for(ix=0;ix<4;++ix) {
							color = (unsigned short)(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>3),10,5)|_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>3),5,5)|_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>3),0,5))|0x8000;
							WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
						}
					}
				}
			}
		} else {
			bits = pImg->GetPixel();
			for(y=0;y<yres;y+=4) {
				for(x=0;x<xres;x+=4) {
					for(iy=0;iy<4;++iy) {
						for(ix=0;ix<4;++ix) {
							color = (unsigned short)(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_ALPHA]>>5),12,3)|_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>4),8,4)|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>4),4,4))|_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>4),0,4))&~0x8000;
							WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
						}
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexture_RGBA8(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres;
	unsigned short color;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=4) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<4;++ix) {
						color = _SHIFTL(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_ALPHA],8,8)|(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]&0xff);
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
					}
				}
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<4;++ix) {
						color = _SHIFTL(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN],8,8)|(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]&0xff);
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexture_CMPR(_tImage *tImage)
{
	#define OFFSET(x,y) (((y)/4)*stride+((x)/4)*4)
	#define SWAP(v)	SwapShort(bitrev((v)))

	int x,y,t,idx;
	int xres,yres,mx,my;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned short *sdata,color;
	long stride;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage) return 0;

	sFilename = tImage->pszID;
	sFilename += "tex.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	idx = 0;
	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();
		stride = xres;

		sdata = (unsigned short*)DXTCompress(pImg,CMPR_TYPE_TCX);
		if(sdata) {
			for(y=0;y<yres;y+=8) {
				for(x=0;x<xres;x+=8) {
					for(t=0;t<4;++t) {
						mx = x+(t&1)*4;
						my = y+(!!(t&2))*4;
						
						color = sdata[OFFSET(mx,my)+0];
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);

						color = sdata[OFFSET(mx,my)+1];
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);

						color = SWAP(sdata[OFFSET(mx,my)+2]);
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);

						color = SWAP(sdata[OFFSET(mx,my)+3]);
						WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
					}
				}
			}
			delete [] sdata;
		}
		tLayers = tLayers->next_p;
		idx++;
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WritePaletteBlock(_tImage *tImage,int nReqCols)
{
	int i;
	CImage *pImg;
	unsigned short color;
	unsigned char ria,a;
	string sFilename;
	FILE *pFile = NULL;

	if(!tImage || !tImage->pImage) return 0;

	pImg = tImage->pImage;
	switch(tImage->nPalFmt) {
		case TF_TLUT_RGB565:
			pImg->DiffuseError(0,5,6,5);
			break;
		case TF_TLUT_RGB5A3:
			if(pImg->IsTransparent())
				pImg->DiffuseError(3,4,4,4);
			else
				pImg->DiffuseError(0,5,5,5);
			break;
		default:
			break;
	}
	tImage->nPalCols = pImg->GetPalettized(NULL,&tImage->pPal,nReqCols);

	WritePalHeader(tImage);

	sFilename = tImage->pszID;
	sFilename += "pal.bin";
	pFile = fopen(sFilename.c_str(),"wb");
	if(pFile==NULL) return EXIT_FAILURE;

	for(i=0;i<tImage->nPalCols;i++) {
		color = 0;
		switch(tImage->nPalFmt){
			case TF_TLUT_IA8:
				if(!pImg->IsTransparent()) a = 0xff;
				else a = tImage->pPal[i].rgbReserved;
				
				ria = (tImage->pPal[i].rgbRed+tImage->pPal[i].rgbGreen+tImage->pPal[i].rgbBlue)/3;
				color = _SHIFTL(a,8,8)|(ria&0xff);
				break;
			case TF_TLUT_RGB565:
				color = (unsigned short)(_SHIFTL((tImage->pPal[i].rgbRed>>3),11,5))|(_SHIFTL((tImage->pPal[i].rgbGreen>>2),5,6))|(_SHIFTL((tImage->pPal[i].rgbBlue>>3),0,5));
				break;
			case TF_TLUT_RGB5A3:
				if(!pImg->IsTransparent()) {
					color = (unsigned short)(_SHIFTL((tImage->pPal[i].rgbRed>>3),10,5))|(_SHIFTL((tImage->pPal[i].rgbGreen>>3),5,5))|(_SHIFTL((tImage->pPal[i].rgbBlue>>3),0,5))|0x8000;		
				} else {
					color = (unsigned short)(_SHIFTL((tImage->pPal[i].rgbReserved>>5),12,3))|(_SHIFTL((tImage->pPal[i].rgbRed>>4),8,4))|(_SHIFTL((tImage->pPal[i].rgbGreen>>4),4,4))|(_SHIFTL((tImage->pPal[i].rgbBlue>>4),0,4))&~0x8000;
				}
				break;
			default:
				break;

		}
		WriteValue(&color,VALUE_TYPE_SHORT,2,pFile);
	}
	fclose(pFile);
	return 1;
}

int CTFBinary::WriteTexHeader(_tImage *tImage)
{
	int xres,yres;
	int nMin,nMag;
	int nWrapS,nWrapT;
	CImage *pImg;
	_tLayer *tLayers;
	FILE *pFile = NULL;
	string sFilename,sID;

	if(!tImage) return 0;

	tLayers = tImage->GetLayers();
	pImg = tLayers->GetImage();

	xres = pImg->GetXSize();
	yres = pImg->GetYSize();

	GetMinMag(tImage,nMin,nMag);

	nWrapS = tImage->nWrapS;
	nWrapT = tImage->nWrapT;
	if(nWrapS==-1) nWrapS = ComputeFilterModeByDimension(tImage);
	if(nWrapT==-1) nWrapT = ComputeFilterModeByDimension(tImage);

	sID = tImage->pszID;
	sFilename = sID + "tex.h";
	pFile = fopen(sFilename.c_str(),"w");
	if(pFile==NULL) return EXIT_FAILURE;

	fprintf(pFile,"#ifndef __%s_TEX_H__\n",sID.c_str());
	fprintf(pFile,"#define __%s_TEX_H__\n\n",sID.c_str());
	fprintf(pFile,"struct _texdef {\n");
	fprintf(pFile,"\tint width;\n");
	fprintf(pFile,"\tint height;\n");
	fprintf(pFile,"\tint colfmt;\n");
	fprintf(pFile,"\tint minlod;\n");
	fprintf(pFile,"\tint maxlod;\n");
	fprintf(pFile,"\tint min;\n");
	fprintf(pFile,"\tint mag;\n");
	fprintf(pFile,"\tint wrap_s;\n");
	fprintf(pFile,"\tint wrap_t;\n");
	fprintf(pFile,"} %stex = {\n",sID.c_str());
	fprintf(pFile,"\t%d,\n",xres);
	fprintf(pFile,"\t%d,\n",yres);
	fprintf(pFile,"\t%d,\n",tImage->nColFmt);
	fprintf(pFile,"\t%d,\n",tImage->nRemapLOD);
	fprintf(pFile,"\t%d,\n",tImage->nRemapLOD+(tImage->nMaxLOD-tImage->nMinLOD));
	fprintf(pFile,"\t%d,\n",nMin);
	fprintf(pFile,"\t%d,\n",nMag);
	fprintf(pFile,"\t%d,\n",nWrapS);
	fprintf(pFile,"\t%d\n",nWrapT);
	fprintf(pFile,"};\n\n");
	fprintf(pFile,"#endif\n");
	fclose(pFile);

	return EXIT_SUCCESS;
}

int CTFBinary::WritePalHeader(_tImage *tImage)
{
	FILE *pFile = NULL;
	string sFilename,sID;

	if(!tImage) return 0;

	sID = tImage->pszID;
	sFilename = sID + "pal.h";
	pFile = fopen(sFilename.c_str(),"w");
	if(pFile==NULL) return EXIT_FAILURE;

	fprintf(pFile,"#ifndef __%s_PAL_H__\n",sID.c_str());
	fprintf(pFile,"#define __%s_PAL_H__\n\n",sID.c_str());
	fprintf(pFile,"struct _paldef {\n");
	fprintf(pFile,"\tint palfmt;\n");
	fprintf(pFile,"\tint ncols;\n");
	fprintf(pFile,"} %spal = {\n",sID.c_str());
	fprintf(pFile,"\t%d,\n",tImage->nPalFmt);
	fprintf(pFile,"\t%d\n",tImage->nPalCols);
	fprintf(pFile,"};\n\n");
	fprintf(pFile,"#endif\n");
	fclose(pFile);

	return EXIT_SUCCESS;
}

