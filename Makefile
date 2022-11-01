# use g++ compiler
CXX=g++
CXXFLAGS?=-Wall -pedantic -std=c++11

# flag specifications for release and debug
RELEASEFLAGS?=$(CXXFLAGS) $(OUTFLAG) $(UINTFLAG) -O3
DEBUGFLAGS?=$(CXXFLAGS) $(OUTFLAG) $(UINTFLAG) -O0 -g #-pg

# relevant constants
CPP_FILES=main.cpp argparse.cpp
HEADER_FILES=argparse.h
GLOBAL_DEPS=$(CPP_FILES) $(HEADER_FILES)
EXE=viral_consensus_mp
DEBUG_SUFFIX=debug
DEBUG_EXE=$(EXE)_$(DEBUG_SUFFIX)

# compile
all: $(EXE)
$(EXE): $(GLOBAL_DEPS)
	$(CXX) $(RELEASEFLAGS) -o $(EXE) $(CPP_FILES)
debug: $(GLOBAL_DEPS)
	$(CXX) $(DEBUGFLAGS) -o $(DEBUG_EXE) $(CPP_FILES)
clean:
	$(RM) $(EXE) $(DEBUG_EXE) *.o
