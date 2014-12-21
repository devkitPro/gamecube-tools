#ifndef __TFASMSRC_H__
#define __TFASMSRC_H__

#include "texturefile.h"

class CTFBinary : public CTextureFile
{
public:
	CTFBinary(_tImage *tImages);
	virtual ~CTFBinary();

	int Write(CParser *pParser);

private:
	int WriteTexHeader(_tImage *tImage);
	int WritePalHeader(_tImage *tImage);
	int WriteTexture_I4(_tImage *tImage);
	int WriteTexture_I8(_tImage *tImage);
	int WriteTexture_IA4(_tImage *tImage);
	int WriteTexture_IA8(_tImage *tImage);
	int WriteTexture_CI4(_tImage *tImage);
	int WriteTexture_CI8(_tImage *tImage);
	int WriteTexture_RGB565(_tImage *tImage);
	int WriteTexture_RGB5A3(_tImage *tImage);
	int WriteTexture_RGBA8(_tImage *tImage);
	int WriteTexture_CMPR(_tImage *tImage);
	int WritePaletteBlock(_tImage *tImage,int nReqCols);
};

#endif
