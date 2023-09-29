# use g++ compiler
CXX=g++
CXXFLAGS?=-Wall -pedantic -std=c++11

# flag specifications for release and debug
RELEASEFLAGS?=$(CXXFLAGS) -O3
DEBUGFLAGS?=$(CXXFLAGS) -O0 -g #-pg
INCLUDE=-Ihtslib
LIBS=-llzma -lbz2 -lz -lcurl -pthread -lhts

# relevant constants
CPP_FILES=main.cpp common.cpp argparse.cpp count.cpp fasta.cpp primer.cpp
HEADER_FILES=common.h argparse.h count.h fasta.h primer.h
GLOBAL_DEPS=$(CPP_FILES) $(HEADER_FILES)
EXE=viral_consensus
DEBUG_SUFFIX=debug
DEBUG_EXE=$(EXE)_$(DEBUG_SUFFIX)

# compile
all: $(EXE)
$(EXE): $(GLOBAL_DEPS))
	$(CXX) $(RELEASEFLAGS) $(INCLUDE) -o $(EXE) $(CPP_FILES) $(LIBS)
debug: $(GLOBAL_DEPS)
	$(CXX) $(DEBUGFLAGS) $(INCLUDE) -o $(DEBUG_EXE) $(CPP_FILES) $(LIBS)
clean:
	$(RM) $(EXE) $(DEBUG_EXE) *.o
