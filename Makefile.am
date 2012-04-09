# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = gxtexconv

noinst_LIBRARIES      = libdxtn.a libsquish.a

libdxtn_a_SOURCES	= dxtn/source/dxtn.h dxtn/source/dxtn_compress.c dxtn/source/dxtn_fetch.c

libdxtn_a_CPPFLAGS	= -fvisibility=hidden

libsquish_a_SOURCES	= squish/source/alpha.cpp squish/source/alpha.h squish/source/clusterfit.cpp \
			  squish/source/clusterfit.h squish/source/colourblock.cpp squish/source/colourblock.h \
			  squish/source/colourfit.cpp squish/source/colourfit.h squish/source/colourset.cpp \
			  squish/source/colourset.h squish/source/config.h squish/source/maths.cpp \
			  squish/source/maths.h squish/source/rangefit.cpp squish/source/rangefit.h \
			  squish/source/simd.h squish/source/simd_sse.h squish/source/simd_ve.h \
			  squish/source/singlecolourfit.cpp squish/source/singlecolourfit.h \
			  squish/source/singlecolourlookup.inl squish/source/squish.cpp \
			  squish/source/squish.h squish/source/stdafx.h

libsquish_a_CPPFLAGS	= -fvisibility=hidden

gxtexconv_SOURCES	= source/converter.cpp source/converter.h source/gxtexconv.cpp source/image.cpp \
			  source/image.h source/parser.cpp source/parser.h source/stdafx.cpp source/stdafx.h \
			  source/texturefile.cpp source/texturefile.h source/tfbinary.cpp source/tfbinary.h \
			  source/tfbinarytdf.cpp source/tfbinarytdf.h source/tokenstring.cpp source/tokenstring.h

gxtexconv_LDADD		= libsquish.a libdxtn.a $(FREEIMAGE_LIBS)
gxtexconv_CPPFLAGS	= -fvisibility=hidden -I$(top_srcdir)/dxtn/source -I$(top_srcdir)/squish/source

EXTRA_DIST = autogen.sh

