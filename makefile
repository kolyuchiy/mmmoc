# MMMOC -- учебная операционная система
# (c) 2003, Kolia Morev <kolia39@mail.ru>
# 
# Makefile.

CC	= bcc
AS	= tasm
LD	= tlink 

CFLAGS  = -3 -c -v -DDEBUG
ASFLAGS	= /m2
LDFLAGS	= /3

LISTING = ,
ASDEBUG = /zi  
LDDEBUG = /v /m

OBJS = startup.obj console.obj sys.obj util.obj ints.obj \
	tasks.obj main.obj 



all:  mmmoc.exe

mmmoc.exe:  $(OBJS)
           $(LD) $(LDFLAGS) $(LDDEBUG) $(OBJS), mmmoc.exe,,

.asm.obj:
           $(AS) $(ASFLAGS) $(ASDEBUG) $<, $(LISTING)

.c.obj:
           $(CC) $(CFLAGS) $<

clean:
	del *.obj
	del *.lst
	del mmmoc.map
	del mmmoc.exe
