#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

host_os := $(shell uname -s)

DEBUGFLAGS	:=

CFLAGS	:=	$(DEBUGFLAGS) -Wall -O3
CFLAGS	+=	$(INCLUDE)

CXXFLAGS	:=	$(CFLAGS)
LDFLAGS	:=	$(DEBUGFLAGS)

ifneq (,$(findstring MINGW,$(host_os)))
	export exeext	:= .exe
endif

ifneq (,$(findstring Darwin,$(host_os)))
	SDK	:=	/Developer/SDKs/MacOSX10.4u.sdk
	OSXCFLAGS	:= -mmacosx-version-min=10.4 -isysroot $(SDK) -arch i386 -arch ppc
	OSXCXXFLAGS	:=	$(OSXCFLAGS)
	CXXFLAGS	+=	-fvisibility=hidden
	LDFLAGS		+= -mmacosx-version-min=10.4 -arch i386 -arch ppc -Wl,-syslibroot,$(SDK)
endif

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	gdopcode gdtool
INCLUDES	:=	include


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export CC	:=	gcc
export CXX	:=	g++
export AR	:=	ar
export OBJCOPY	:=	objcopy
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export LD		:=	$(CXX)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD): lib
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

lib:
	mkdir -p lib

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) lib gcdsptool$(exeext)

#---------------------------------------------------------------------------------
all: clean $(BUILD)

#---------------------------------------------------------------------------------
install:
	cp gcdsptool$(exeext) $(PREFIX)


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(TOPDIR)/gcdsptool$(exeext)	:	gdtool.o $(TOPDIR)/lib/libgcdsp.a
	$(LD) $(LDFLAGS) $(^) -o $@

$(TOPDIR)/lib/libgcdsp.a	:	assemble.o disassemble.o opcodes.o

#---------------------------------------------------------------------------------
%.a:
	rm -f $@
	$(AR) rcs $@ $(^)

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo $(notdir $<)
	$(CXX) -E -MMD $(CFLAGS) $< > /dev/null
	$(CXX) $(OSXCXXFLAGS) $(CXXFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.c
	@echo $(notdir $<)
	$(CC) -E -MMD $(CFLAGS) $< > /dev/null
	$(CC) $(OSXCFLAGS) $(CFLAGS) -o $@ -c $<


#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
