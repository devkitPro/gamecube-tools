#include "stdafx.h"
#include "tfbinarytdf.h"

CTFBinaryTDF::CTFBinaryTDF(_tImage *tImages) : CTextureFile(tImages)
{
}

CTFBinaryTDF::~CTFBinaryTDF()
{
}


int CTFBinaryTDF::Write(CParser *pParser)
{
	int nRet,nTexs;
	int nHdrSize;

	if(pParser==NULL) return EXIT_FAILURE;

	ComputeTPLSize();

	nRet = EXIT_FAILURE;
	m_pOutputFile = fopen(pParser->GetOutputFilename(),"wb");
	if(m_pOutputFile && m_tImages) {
		nRet = EXIT_SUCCESS;

		WriteValue((void*)&tplVersion,VALUE_TYPE_INT);
		
		nTexs = GetNumTextures();
		WriteValue(&nTexs,VALUE_TYPE_INT);

		nHdrSize = tplHdrSize;
		WriteValue(&nHdrSize,VALUE_TYPE_INT);

		WriteTextureDescBlock();
		WriteImageDescBlock();
		WritePalDescBlock();
		WriteImageDataBlock();
		WritePalDataBlock();
		
		string headerFilename=pParser->GetOutputFilename();
		headerFilename.erase(headerFilename.find_last_of("."));
		headerFilename += ".h";
		
		FILE *headerFile = fopen(headerFilename.c_str(),"wb");
		int index=0;

		if(headerFile) {
			_tImage *tImages = m_tImages;
			while(tImages) {
				fprintf(headerFile,"#define %s %d\n",tImages->pszID,index);
				tImages=tImages->pNext;
				index++;
			}
			fclose(headerFile);
		}
	}
	return nRet;
}


int CTFBinaryTDF::WriteTexture_I4(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria,ucTmp;

	if(!tImage) return 0;

	nRet = 0;
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
							nRet += WriteValue(&ucTmp,VALUE_TYPE_CHAR);
						}
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_I8(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria;

	if(!tImage) return 0;

	nRet = 0;
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
						nRet += WriteValue(&ria,VALUE_TYPE_CHAR);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_IA4(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char ria,a,ucTmp;

	if(!tImage) return 0;

	nRet = 0;
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
						nRet += WriteValue(&ucTmp,VALUE_TYPE_CHAR);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_IA8(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short sTmp;
	unsigned char ria,a;

	if(!tImage) return 0;

	nRet = 0;
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
						nRet += WriteValue(&sTmp,VALUE_TYPE_SHORT);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_CI4(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned char *pPix;
	unsigned char ria,ucTmp;

	if(!tImage) return 0;

	nRet = 0;
	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		pImg->GetPalettized(&pPix,NULL,16);
		
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		for(y=0;y<yres;y+=8) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<8;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = pPix[((y+iy)*xres+(x+ix))];
						
						if(!(ix%2)) ucTmp = _SHIFTL(ria,4,4);
						else {
							ucTmp |= (ria&0xf);
							nRet += WriteValue(&ucTmp,VALUE_TYPE_CHAR);
						}
					}
				}
			}
		}
		delete [] pPix;

		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_CI8(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned char *pPix;
	unsigned char ria;

	if(!tImage) return 0;

	nRet = 0;
	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		pImg->GetPalettized(&pPix,NULL,256);

		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=8) {
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<8;++ix) {
						ria = pPix[((y+iy)*xres+(x+ix))];
						nRet += WriteValue(&ria,VALUE_TYPE_CHAR);
					}
				}
			}
		}
		delete [] pPix;

		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_RGB565(_tImage *tImage)
{
	int nRet;
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short color;

	if(!tImage) return 0;

	nRet = 0;
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
						color = (unsigned short)(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>3),11,5))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>2),5,6))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>3),0,5));
						nRet += WriteValue(&color,VALUE_TYPE_SHORT);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return nRet;
}

int CTFBinaryTDF::WriteTexture_RGB5A3(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned char a;
	unsigned short color;

	if(!tImage) return 0;

	tLayers = tImage->GetLayers();
	while(tLayers)  {
		pImg = tLayers->GetImage();
		xres = pImg->GetXSize();
		yres = pImg->GetYSize();

		bits = pImg->GetPixel();
		for(y=0;y<yres;y+=4) {
			for(x=0;x<xres;x+=4) {
				for(iy=0;iy<4;iy++) {
					for(ix=0;ix<4;ix++) {
						a = bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_ALPHA];
						if(a>=224)
							color = (unsigned short)((_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>3),10,5))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>3),5,5))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>3),0,5)))|0x8000;
						else
							color = (unsigned short)((_SHIFTL((a>>5),12,3))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_RED]>>4),8,4))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN]>>4),4,4))|(_SHIFTL((bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]>>4),0,4)))&~0x8000;
						WriteValue(&color,VALUE_TYPE_SHORT);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return 1;
}

int CTFBinaryTDF::WriteTexture_RGBA8(_tImage *tImage)
{
	int x,y,ix,iy;
	int xres,yres;
	_tLayer *tLayers;
	CImage *pImg;
	BYTE *bits;
	unsigned short color;

	if(!tImage) return 0;

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
						WriteValue(&color,VALUE_TYPE_SHORT);
					}
				}
				for(iy=0;iy<4;++iy) {
					for(ix=0;ix<4;++ix) {
						color = _SHIFTL(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_GREEN],8,8)|(bits[(((y+iy)*(xres<<2))+((x+ix)<<2))+FI_RGBA_BLUE]&0xff);
						WriteValue(&color,VALUE_TYPE_SHORT);
					}
				}
			}
		}
		tLayers = tLayers->next_p;
	}
	return 1;
}

int CTFBinaryTDF::WriteTexture_CMPR(_tImage *tImage)
{
#define OFFSET(x,y) (((y)/4)*stride+((x)/4)*4)
#define SWAP(v)	SwapShort(bitrev((v)))

	int x,y,t,idx;
	int xres,yres,mx,my;
	_tLayer *tLayers;
	CImage *pImg;
	unsigned short *sdata,svalue;
	long stride;

	if(!tImage) return 0;

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

						svalue = sdata[OFFSET(mx,my)+0];
						WriteValue(&svalue,VALUE_TYPE_SHORT);

						svalue = sdata[OFFSET(mx,my)+1];
						WriteValue(&svalue,VALUE_TYPE_SHORT);

						svalue = SWAP(sdata[OFFSET(mx,my)+2]);
						WriteValue(&svalue,VALUE_TYPE_SHORT);

						svalue = SWAP(sdata[OFFSET(mx,my)+3]);
						WriteValue(&svalue,VALUE_TYPE_SHORT);
					}
				}
			}
			delete [] sdata;
		}
		tLayers = tLayers->next_p;
		idx++;
	}
	return 1;
}

int CTFBinaryTDF::WriteTextureDescBlock()
{
	int nRet;
	int iTmp;
	_tImage *tImages = m_tImages;

	if(!tImages) return 0;

	nRet = 0;
	while(tImages) {
		iTmp = tImages->nImageDescOffset;
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		iTmp = 0;
		if(tImages->pPal && tImages->nPalCols>0) iTmp = tImages->nPalDescOffset;
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		tImages = tImages->pNext;
	}
	return nRet;
}

int CTFBinaryTDF::WritePalDescBlock()
{
	int nRet,iTmp;
	short sTmp;
	_tImage *tImages;

	if(!m_tImages) return 0;

	nRet = 0;
	tImages = m_tImages;
	while(tImages) {
		Seek(tImages->nPalDescOffset,SEEK_SET);

		if(tImages->nPalCols>0) {
			sTmp = (short)tImages->nPalCols;
			nRet += WriteValue(&sTmp,VALUE_TYPE_SHORT);

			nRet += WriteValue(tplPad,VALUE_TYPE_DATA,2);

			iTmp = tImages->nPalFmt;
			nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

			iTmp = tImages->nPalDataOffset;
			nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		}
		tImages = tImages->pNext;
	}
	if(m_nPalDescPad>0) nRet += WriteValue(tplPad,VALUE_TYPE_DATA,m_nPalDescPad);

	return nRet;
}

int CTFBinaryTDF::WritePalDataBlock()
{
	int nRet;
	int nEntries;
	RGBQUAD *pPal;
	_tImage *tImages;

	nRet = 0;
	tImages = m_tImages;
	while(tImages) {
		Seek(tImages->nPalDataOffset,SEEK_SET);

		if(tImages->pPal && tImages->nPalCols>0) {
			pPal = tImages->pPal;
			nEntries = tImages->nPalCols;
			switch(tImages->nPalFmt) {
				case TF_TLUT_RGB565:
					nRet += WritePalBlock_RGB565(nEntries,pPal);
					break;
				case TF_TLUT_RGB5A3:
					nRet += WritePalBlock_RGB5A3(nEntries,pPal);
					break;
			}
		}

		tImages = tImages->pNext;
	}
	return nRet;
}

int CTFBinaryTDF::WriteImageDescBlock()
{
	char cTmp;
	int nMin,nMag;
	int nRet,iTmp;
	short sTmp;
	_tImage *tImages;

	if(!m_tImages) return 0;

	nRet = 0;
	tImages = m_tImages;
	while(tImages) {
		Seek(tImages->nImageDescOffset,SEEK_SET);

		sTmp = (unsigned short)(tImages->pImage->GetYSize()>>tImages->nMinLOD);
		nRet += WriteValue(&sTmp,VALUE_TYPE_SHORT);

		sTmp = (unsigned short)(tImages->pImage->GetXSize()>>tImages->nMinLOD);
		nRet += WriteValue(&sTmp,VALUE_TYPE_SHORT);

		iTmp = tImages->nColFmt;
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		iTmp = tImages->nImageDataOffset;
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		iTmp = tImages->nWrapS;
		if(iTmp==-1) iTmp = ComputeFilterModeByDimension(tImages);
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		iTmp = tImages->nWrapT;
		if(iTmp==-1) iTmp = ComputeFilterModeByDimension(tImages);
		nRet += WriteValue(&iTmp,VALUE_TYPE_INT);

		GetMinMag(tImages,nMin,nMag);
		nRet += WriteValue(&nMin,VALUE_TYPE_INT);
		nRet += WriteValue(&nMag,VALUE_TYPE_INT);

		iTmp = 0;
		nRet += WriteValue(&iTmp,VALUE_TYPE_FLOAT);

		cTmp = 0;
		nRet += WriteValue(&cTmp,VALUE_TYPE_CHAR);

		cTmp = (unsigned char)tImages->nRemapLOD;
		nRet += WriteValue(&cTmp,VALUE_TYPE_CHAR);

		cTmp = (unsigned char)(tImages->nRemapLOD+(tImages->nMaxLOD-tImages->nMinLOD));
		nRet += WriteValue(&cTmp,VALUE_TYPE_CHAR);

		nRet += WriteValue(tplPad,VALUE_TYPE_CHAR);

		tImages = tImages->pNext;
	}
	if(m_nImageDescPad>0) nRet += WriteValue(tplPad,VALUE_TYPE_DATA,m_nImageDescPad);

	return nRet;
}

int CTFBinaryTDF::WriteImageDataBlock()
{
	int nRet;
	_tImage *tImages;

	if(!m_tImages) return 0;

	nRet = 0;
	tImages = m_tImages;
	while(tImages) {
		Seek(tImages->nImageDataOffset,SEEK_SET);

		switch(tImages->nColFmt) {
			case TF_I4:
				nRet += WriteTexture_I4(tImages);
				break;
			case TF_I8:
				nRet += WriteTexture_I8(tImages);
				break;
			case TF_IA4:
				nRet += WriteTexture_IA4(tImages);
				break;
			case TF_IA8:
				nRet += WriteTexture_IA8(tImages);
				break;
			case TF_CI4:
				nRet += WriteTexture_CI4(tImages);
				break;
			case TF_CI8:
				nRet += WriteTexture_CI8(tImages);
				break;
			case TF_RGB565:
				nRet += WriteTexture_RGB565(tImages);
				break;
			case TF_RGB5A3:
				nRet += WriteTexture_RGB5A3(tImages);
				break;

			case TF_RGBA8:
				nRet += WriteTexture_RGBA8(tImages);
				break;

			case TF_CMPR:
				nRet += WriteTexture_CMPR(tImages);
				break;
		}
		tImages = tImages->pNext;
	}
	return nRet;
}

int CTFBinaryTDF::WritePalBlock_RGB565(int nEntries,RGBQUAD *pPal)
{
	int i,nPad,nRet = 0;
	unsigned short color;

	if(!pPal) return 0;

	nPad = 0;
	if(nEntries<16) nPad = 16-nEntries;
	else if(nEntries%16) nPad = 16-(nEntries%16);

	for(i=0;i<nEntries;i++) {
		color = (unsigned short)(_SHIFTL((pPal[i].rgbRed>>3),11,5))|(_SHIFTL((pPal[i].rgbGreen>>2),5,6))|(_SHIFTL((pPal[i].rgbBlue>>3),0,5));
		nRet += WriteValue(&color,VALUE_TYPE_SHORT);
	}
	if(nPad>0) nRet += WriteValue(tplPad,VALUE_TYPE_DATA,nPad*sizeof(short));

	return nRet;
}

int CTFBinaryTDF::WritePalBlock_RGB5A3(int nEntries,RGBQUAD *pPal)
{
	int i,nPad,nRet = 0;
	unsigned short color;

	if(!pPal) return 0;

	nPad = 0;
	if(nEntries<16) nPad = 16-nEntries;
	else if(nEntries%16) nPad = 16-(nEntries%16);

	for(i=0;i<nEntries;i++) {
		if(pPal[i].rgbReserved==0xff) {
			color = (unsigned short)((_SHIFTL((pPal[i].rgbRed>>3),10,5))|(_SHIFTL((pPal[i].rgbGreen>>3),5,5))|(_SHIFTL((pPal[i].rgbBlue>>3),0,5)))|0x8000;		
		} else {
			color = (unsigned short)((_SHIFTL((pPal[i].rgbReserved>>5),12,3))|(_SHIFTL((pPal[i].rgbRed>>4),8,4))|(_SHIFTL((pPal[i].rgbGreen>>4),4,4))|(_SHIFTL((pPal[i].rgbBlue>>4),0,4)))&~0x8000;
		}
		nRet += WriteValue(&color,VALUE_TYPE_SHORT);
	}
	if(nPad>0) nRet += WriteValue(tplPad,VALUE_TYPE_DATA,nPad*sizeof(short));

	return nRet;
}
