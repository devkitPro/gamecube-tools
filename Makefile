#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX			:=

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

test := $(shell uname -s)

ifneq (,$(findstring MINGW,$(test)))
	export exeext	:= .exe
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
# options for code generation
#---------------------------------------------------------------------------------
DEBUGFLAGS	:=	-g
CFLAGS	:=	$(DEBUGFLAGS) -Wall -O3\

CFLAGS	+=	$(INCLUDE)

LDFLAGS	=	$(DEBUGFLAGS) -Wl,-Map,$(notdir $@).map

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

export CC		:=	$(PREFIX)gcc
export CXX		:=	$(PREFIX)g++
export AR		:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
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
	@rm -fr $(BUILD) lib gdtool$(exeext)

#---------------------------------------------------------------------------------
all: clean $(BUILD)

#---------------------------------------------------------------------------------
install:
	cp gdtool$(exeext) $(PREFIX)


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(TOPDIR)/gdtool$(exeext)	:	gdtool.o $(TOPDIR)/lib/libgcdsp.a
	$(LD) $(LDFLAGS) $(^) -o $@

$(TOPDIR)/lib/libgcdsp.a	:	assemble.o disassemble.o opcodes.o

#---------------------------------------------------------------------------------
%.a:
	$(AR) rcs $@ $(^)

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD $(CFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.c
	@echo $(notdir $<)
	@$(CC) -MMD $(CFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.s
	@echo $(notdir $<)
	@$(CC) -MMD $(ASFLAGS) -o $@ -c $<


#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
