# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = gcdsptool
noinst_LTLIBRARIES      = libgdopcode.la 

libgdopcode_la_SOURCES	= gdopcode/dbg.h gdopcode/dtypes.h gdopcode/assemble.cpp gdopcode/disassemble.cpp \
			  gdopcode/disassemble.h gdopcode/opcodes.cpp gdopcode/opcodes.h

libgdopcode_la_CPPFLAGS = -I$(top_srcdir)/gdtool

gcdsptool_SOURCES	= gdtool/gdsp_tool.h gdtool/gdtool.cpp

gcdsptool_LDADD		= libgdopcode.la

gcdsptool_CPPFLAGS 	= -I$(top_srcdir)/gdopcode

EXTRA_DIST	= autogen.sh doc/GC_DSP.doc doc/GC_DSP.pdf doc/manual.txt doc/opcodes_list.xls \
		  doc/operation.txt doc/reverse.txt doc/reverse_new.txt

