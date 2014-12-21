# Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gcdsptool elf2dol

noinst_LTLIBRARIES      = libgdopcode.la 

libgdopcode_la_SOURCES	= gdopcode/dbg.h gdopcode/dtypes.h gdopcode/assemble.cpp gdopcode/disassemble.cpp \
			  gdopcode/disassemble.h gdopcode/opcodes.cpp gdopcode/opcodes.h

libgdopcode_la_CPPFLAGS = -I$(top_srcdir)/gdtool

gcdsptool_SOURCES	= gdtool/gdsp_tool.h gdtool/gdtool.cpp

gcdsptool_LDADD		= libgdopcode.la

gcdsptool_CPPFLAGS 	= -I$(top_srcdir)/gdopcode

elf2dol_SOURCES	= elf2dol.c

EXTRA_DIST	= autogen.sh doc/GC_DSP.doc doc/GC_DSP.pdf doc/manual.txt doc/opcodes_list.xls \
		  doc/operation.txt doc/reverse.txt doc/reverse_new.txt
