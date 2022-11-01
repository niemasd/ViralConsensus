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

# relevant constants
CPP_FILES=main.cpp argparse.cpp
HEADER_FILES=argparse.h
GLOBAL_DEPS=$(CPP_FILES) $(HEADER_FILES)
EXE=viral_consensus_mp
DEBUG_SUFFIX=debug
DEBUG_EXE=$(EXE)_$(DEBUG_SUFFIX)

# compile
all: $(EXE)
$(EXE): $(GLOBAL_DEPS) htslib/libhts.a
	$(CXX) $(RELEASEFLAGS) -o $(EXE) $(CPP_FILES)
debug: $(GLOBAL_DEPS)
	$(CXX) $(DEBUGFLAGS) -o $(DEBUG_EXE) $(CPP_FILES)
clean:
	$(RM) $(EXE) $(DEBUG_EXE) *.o
htslib/Makefile:
	git submodule update --init --recursive
htslib/libhts.a: htslib/Makefile
	cd htslib && make libhts.a
htslib/$(HTSLIB): htslib/Makefile
	cd htslib && make $(HTSLIB)
