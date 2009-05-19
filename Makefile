#---------------------------------------------------------------------------------
# TARGET is the name of the output, if this ends with _mb generates a multiboot image
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	gxtexconv
#TARGET		:=	gxtexconv_d
BUILD		:=	build
SOURCES		:=	source
INCLUDE		:=	include
LIBDIRS		:=	dxtn squish

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

MACHDEP 	= 
CFLAGS 		= -O2 -Wall $(MACHDEP) $(INCLUDES)

LDFLAGS		= $(MACHDEP)

UNAME	:=	$(shell uname -s)

ifneq (,$(findstring MINGW,$(UNAME)))
	PLATFORM	:= win32
	EXEEXT		:= .exe
endif

ifneq (,$(findstring CYGWIN,$(UNAME)))
	CFLAGS	+= -mno-cygwin
	LDFLAGS	+= -mno-cygwin
	EXEEXT	:= .exe
endif

ifneq (,$(findstring Darwin,$(UNAME)))
	CFLAGS	+= -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -fvisibility=hidden
	ARCH	:= -arch i386 -arch ppc
	LDFLAGS += -arch i386 -arch ppc
endif

PREFIX		:=	

LIBS		:= -lfreeimage -ldxtn -lsquish
#LIBS		:= -lfreeimage_d -ldxtn_d -lsquish_d

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export CC	:=	$(PREFIX)gcc
export CXX	:=	$(PREFIX)g++
export AR	:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export LD	:=	$(CXX)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))

export OFILES	:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDES	:=	$(foreach dir,$(INCLUDE),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(CURDIR)/$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(CURDIR)/$(dir)/lib)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@make -C dxtn
	@make -C squish
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@make -C dxtn clean
	@make -C squish clean
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT)$(EXEEXT)

install:
	cp  $(OUTPUT) $(PREFIX)
		
#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT): $(DFILES) $(OFILES)
	@echo linking ... $(notdir $@)
	$(LD)  $^ $(LDFLAGS) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo $(notdir $<)
	$(CXX) -E -MMD $(CFLAGS) $< > /dev/null
	$(CXX) $(CFLAGS) $(ARCH) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.c
	@echo $(notdir $<)
	$(CC) -E -MMD $(CFLAGS) $< > /dev/null
	$(CC) $(CFLAGS) $(ARCH) -o $@ -c $<


-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
