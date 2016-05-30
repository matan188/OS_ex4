CC = gcc
RANLIB = ranlib

LIBSRC = CachingFileSystem.cpp CDE.h CDE.cpp CountChain.h
LIBSRC2 = CountChain.cpp LRUStack.h LRUStack.cpp

CPPFILES = CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp

PKGFLAGS = `pkg-config fuse --cflags --libs`

LIBOBJ = $(LIBSRC:.cpp=.o)

INCS = -I.
CFLAGS = -Wall -std=c++11 -g $(INCS) 
LOADLIBES = -L./ 

TAR = tar
TARFLAGS = -cvf
TARNAME = ex4.tar
TARSRCS = $(LIBSRC) $(LIBSRC2) Makefile README

all: CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp
	g++ -Wall -std=c++11 $(CPPFILES) $(PKGFLAGS) -o CachingFileSystem

clean:
	rm *.o *.tar

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
	


