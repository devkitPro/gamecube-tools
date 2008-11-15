#ifndef __TFBINARYTDF_H__
#define __TFBINARYTDF_H__

#include "texturefile.h"

class CTFBinaryTDF : public CTextureFile
{
public:
	CTFBinaryTDF(_tImage *tImages);
	virtual ~CTFBinaryTDF();

	int Write(CParser *pParser);

private:
	int WriteTextureDescBlock();
	int WritePalDescBlock();
	int WriteImageDescBlock();
	int WriteImageDataBlock();
	int WritePalDataBlock();


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

	int WritePalBlock_RGB565(int nEntries,RGBQUAD *pPal);
	int WritePalBlock_RGB5A3(int nEntries,RGBQUAD *pPal);
};

#endif
