PREFIX = /usr/local/wince/cross-tools

PATH := $(PATH):$(PREFIX)/bin:/var/tmp/synce/bin

#VERBOSE = -v

CC = arm-wince-pe-gcc $(VERBOSE)
CXX = arm-wince-pe-g++ $(VERBOSE)
LD = arm-wince-pe-ld $(VERBOSE)
DLLTOOL = arm-wince-pe-dlltool $(VERBOSE)

CPU=xscale
CPPFLAGS = -g -I$(PREFIX)/include -mcpu=$(CPU) -D__stdcall=
LDFLAGS = -L$(PREFIX)/lib 

LIBS = invokeme.dll
PROGRAMS = 

default: all

all: $(LIBS) $(PROGRAMS) 

invokeme.dll: invokeme.cc
	$(CC) -c -DBUILD_DLL $^
	$(CC) -shared -o invokeme.dll invokeme.o

install: invokeme.dll
	pcp $^ ':/Windows/invokeme.dll'

clean:
	rm -f *.o *.a *.exe *.dll *.exp
