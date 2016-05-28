CC = gcc
RANLIB = ranlib

LIBSRC = CachingFileSystem.cpp CDE.h CDE.cpp CountChain.h CountChain.cpp LRUStack.h LRUStack.cpp

LIBOBJ = $(LIBSRC:.cpp=.o)

INCS = -I.
CFLAGS = -Wextra -Wall -std=c++11 -g $(INCS) 
LOADLIBES = -L./ 

TAR = tar
TARFLAGS = -cvf
TARNAME = ex4.tar
TARSRCS = $(LIBSRC) Makefile README

all: CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp
	g++ -Wall -std=c++11 CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp  `pkg-config fuse --cflags --libs` -o CachingFileSystem

clean:
	rm *.a *.o *.tar

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
	


