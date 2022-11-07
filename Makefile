# use g++ compiler
CXX=g++
CXXFLAGS?=-Wall -pedantic -std=c++11

# flag specifications for release and debug
RELEASEFLAGS?=$(CXXFLAGS) -O3
DEBUGFLAGS?=$(CXXFLAGS) -O0 -g #-pg

# htslib stuff (from https://github.com/fbreitwieser/bamcov/blob/master/Makefile)
INCLUDE=-Ihtslib
ifneq "$(origin PLATFORM)" "file"
PLATFORM := $(shell uname -s)
endif
ifeq "$(PLATFORM)" "Darwin"
HTSLIB=libhts.dylib
else ifeq "$(findstring CYGWIN,$(PLATFORM))" "CYGWIN"
HTSLIB=cyghts-$(LIBHTS_SOVERSION).dll
else ifeq "$(findstring MSYS,$(PLATFORM))" "MSYS"
HTSLIB=hts-$(LIBHTS_SOVERSION).dll
else
HTSLIB=libhts.so
endif
HTSLIB_A=htslib/libhts.a
LIBS=-llzma -lbz2 -lz -lcurl -pthread

# relevant constants
CPP_FILES=main.cpp common.cpp argparse.cpp count.cpp fasta.cpp primer.cpp
HEADER_FILES=common.h argparse.h count.h fasta.h primer.h
GLOBAL_DEPS=$(CPP_FILES) $(HEADER_FILES)
EXE=viral_consensus
DEBUG_SUFFIX=debug
DEBUG_EXE=$(EXE)_$(DEBUG_SUFFIX)

# compile
all: $(EXE)
$(EXE): $(GLOBAL_DEPS) $(HTSLIB_A)
	$(CXX) $(RELEASEFLAGS) $(INCLUDE) -o $(EXE) $(CPP_FILES) $(HTSLIB_A) $(LIBS)
debug: $(GLOBAL_DEPS) $(HTSLIB_A)
	$(CXX) $(DEBUGFLAGS) $(INCLUDE) -o $(DEBUG_EXE) $(CPP_FILES) $(HTSLIB_A) $(LIBS)
clean:
	$(RM) $(EXE) $(DEBUG_EXE) *.o
htslib/Makefile:
	git submodule update --init --recursive
htslib/libhts.a: htslib/Makefile
	cd htslib && make libhts.a
htslib/$(HTSLIB): htslib/Makefile
	cd htslib && make $(HTSLIB)
