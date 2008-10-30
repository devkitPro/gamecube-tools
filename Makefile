CC	:=	gcc

CFLAGS	:=	-Wall -Wno-char-subscripts -O2

ifneq (,$(findstring MINGW,$(shell uname -s)))
	exeext		:= .exe
endif

ifneq (,$(findstring Linux,$(shell uname -s)))
	CFLAGS += -static
endif

ifneq (,$(findstring Darwin,$(shell uname -s)))
	CFLAGS += -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc
endif


all:	elf2dol$(exeext)

clean:
	@rm -f elf2dol$(exeext)

elf2dol$(exeext)	:	elf2dol.c
	$(CC) $< -o $@ $(CFLAGS)

install:
	cp  elf2dol$(exeext) $(PREFIX)
