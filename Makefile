CC = gcc
RANLIB = ranlib

LIBSRC = MapReduceFramework.cpp Search.h Search.cpp

LIBOBJ = $(LIBSRC:.cpp=.o)

INCS = -I.
CFLAGS = -Wextra -Wall -std=c++11 -g $(INCS) 
LOADLIBES = -L./ 

LIB = MapReduceFramework.a
TARGETS = $(LIB) tar

TAR = tar
TARFLAGS = -cvf
TARNAME = ex3.tar
TARSRCS = $(LIBSRC) Makefile README *.jpg 

all: CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp
	g++ -Wall -std=c++11 CDE.cpp LRUStack.cpp CountChain.cpp CachingFileSystem.cpp  `pkg-config fuse --cflags --libs` -o caching

clean:
	rm *.a *.o *.tar

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
	


