# Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gcdsptool elf2dol gxtexconv

noinst_LTLIBRARIES      = libgdopcode.la libdxtn.la libsquish.la

libgdopcode_la_SOURCES	= gdopcode/dbg.h gdopcode/dtypes.h gdopcode/assemble.cpp gdopcode/disassemble.cpp \
			  gdopcode/disassemble.h gdopcode/opcodes.cpp gdopcode/opcodes.h

libgdopcode_la_CPPFLAGS = -I$(top_srcdir)/gdtool

libdxtn_la_SOURCES	= dxtn/source/dxtn.h dxtn/source/dxtn_compress.c dxtn/source/dxtn_fetch.c

libdxtn_la_CPPFLAGS	= -fvisibility=hidden

libsquish_la_SOURCES	= squish/source/alpha.cpp squish/source/alpha.h squish/source/clusterfit.cpp \
			  squish/source/clusterfit.h squish/source/colourblock.cpp squish/source/colourblock.h \
			  squish/source/colourfit.cpp squish/source/colourfit.h squish/source/colourset.cpp \
			  squish/source/colourset.h squish/source/config.h squish/source/maths.cpp \
			  squish/source/maths.h squish/source/rangefit.cpp squish/source/rangefit.h \
			  squish/source/simd.h squish/source/simd_sse.h squish/source/simd_ve.h \
			  squish/source/singlecolourfit.cpp squish/source/singlecolourfit.h \
			  squish/source/singlecolourlookup.inl squish/source/squish.cpp \
			  squish/source/squish.h squish/source/stdafx.h

libsquish_la_CPPFLAGS	= -fvisibility=hidden

gxtexconv_SOURCES	= gxtool/converter.cpp gxtool/converter.h gxtool/gxtexconv.cpp gxtool/image.cpp \
			  gxtool/image.h gxtool/parser.cpp gxtool/parser.h gxtool/stdafx.cpp gxtool/stdafx.h \
			  gxtool/texturefile.cpp gxtool/texturefile.h gxtool/tfbinary.cpp gxtool/tfbinary.h \
			  gxtool/tfbinarytdf.cpp gxtool/tfbinarytdf.h gxtool/tokenstring.cpp gxtool/tokenstring.h

gxtexconv_CPPFLAGS	= -fvisibility=hidden -I$(top_srcdir)/dxtn/source -I$(top_srcdir)/squish/source

gxtexconv_LDADD		= libsquish.la libdxtn.la $(FREEIMAGE_LIBS)


gcdsptool_SOURCES	= gdtool/gdsp_tool.h gdtool/gdtool.cpp

gcdsptool_CPPFLAGS 	= -I$(top_srcdir)/gdopcode

gcdsptool_LDADD		= libgdopcode.la


elf2dol_SOURCES		= elftool/elf2dol.c

EXTRA_DIST	= autogen.sh doc/GC_DSP.doc doc/GC_DSP.pdf doc/manual.txt doc/opcodes_list.xls \
		  doc/operation.txt doc/reverse.txt doc/reverse_new.txt
