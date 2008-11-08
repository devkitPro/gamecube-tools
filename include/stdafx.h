// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <string>
#include <iostream>

#ifdef WIN32
// Used for stdout mode change text->binary on dos/win plattforms (default text: 0a -> 0d 0a)
#include <io.h>
#include <fcntl.h>
#endif	// WIN32

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <FreeImage.h>


// TODO: reference additional headers your program requires here

#define TF_FILE_BINARY_ONE						0
#define TF_FILE_BINARY_TDF						1

#define _SHIFTL(v,s,w)							((unsigned int)(((unsigned int)(v)&((0x01<<(w))-1))<<(s)))
#define _SHIFTR(v,s,w)							((unsigned int)(((unsigned int)(v)>>(s))&((0x01<<(w))-1)))

#define LOWORD(l)   					        ((unsigned short)(l))
#define HIWORD(l)   					        ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)   					        ((unsigned char)(w))
#define HIBYTE(w)   					        ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

#define SwapInt(n)								(LOBYTE(LOWORD(n))<<24) + (HIBYTE(LOWORD(n))<<16) + (LOBYTE(HIWORD(n))<<8) + HIBYTE(HIWORD(n))
#define SwapShort(n)							(LOBYTE(n)<<8) + HIBYTE(n)

#ifdef LINUX
#define stricmp									strcasecmp
#define strnicmp								strncasecmp
#endif

#endif
