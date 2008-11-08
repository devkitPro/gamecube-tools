// GXTexConv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "parser.h"
#include "converter.h"
#include "tokenstring.h"

static void usage()
{
	fprintf(stderr, "gxtexconv v0.1.3\n");
	fprintf(stderr, "coded by shagkur, #wiidev blitzed\n");
	fprintf(stderr, "powered by FreeImage library\n\n");
	fprintf(stderr, "usage: gxtexconv -i <imagepath> [-o <outputfile>.bin -t <ouput-type>][colfmt=<texfmt> mipmap=yes minlod=<min level> maxlod=<max level> width=<newwidth> height=<newheight> [palfmt=<palcolfmt>]]\n");
	fprintf(stderr, "       gxtexconv -s <scriptfile>.scf [-o <outputfile>.tdf -t <output-type>]\n\n");
	fprintf(stderr, "       supported output formats:\n");
	fprintf(stderr, "       0:  Output to single binary file.\n");
	fprintf(stderr, "       1:  Output to TDF file.\n\n");
	fprintf(stderr, "       supported texture formats:\n");
	fprintf(stderr, "       4:  RGB565 (R5G6B5)\n");
	fprintf(stderr, "       5:  RGB5A3 (R5G5B5 or A3R4G4B4)\n");
	fprintf(stderr, "       6:  RGBA8  (A8R8G8B8)\n");
	fprintf(stderr, "       14: CMPR (Compressed Format)\n");
}


int main(int argc, const char** argv)
{
	CParser cParser;
	CConverter cConv;

	if(argc<3) {
		usage();
		return 0;
	}

	cParser.Parse(argc,argv);
	cConv = CConverter(&cParser);
	cConv.GenerateTextures();
	cConv.WriteTextures();

	return 0;
}

