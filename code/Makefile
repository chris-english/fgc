INCLUDE_PATHS=. include include/fgc blaze libosmium/include protozero/include pdqsort

ifdef BOOST_DIR
INCLUDE_PATHS += $(BOOST_DIR)
endif

INCLUDE=$(patsubst %,-I%,$(INCLUDE_PATHS))
CXX?=g++
STD?=c++17
WARNINGS+=-Wall -Wextra -Wpointer-arith -Wformat -pedantic -Wunused-variable -Wno-attributes
OPT?=O3
LDFLAGS+=-lz
CXXFLAGS+=-$(OPT) -std=$(STD) -march=native $(WARNINGS) $(INCLUDE) $(LDFLAGS)
      

EX=$(patsubst src/%.cpp,%dbg,$(wildcard src/*.cpp))
FEX=$(patsubst %,%,$(EX))
PGEX=$(patsubst %,%pg,$(EX))
PGFEX=$(patsubst %,%pgf,$(EX))
all: $(EX)
fall: $(FEX)
pgall: $(PGEX)
pgfall: $(PGFEX)
that:
ALL: pgall pgfall \
    all that fall

ifdef OMP
CXXFLAGS += -fopenmp
endif

ifdef TBBDIR
CXXFLAGS += -DUSE_TBB -L$(TBBDIR)/lib -I$(TBBDIR)/include -ltbb
endif

TESTS=tbm coreset_test bztest btest osm2dimacs dmlsearch diskmattest graphtest jvtest kmpptest tbas

clust: kzclustexpdbg kzclustexp kzclustexpf

tests: $(TESTS)
print_tests:
	@echo "Tests: " $(TESTS)

%dbg: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -o $@

graphrun: src/graphtest.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -o $@ -DNDEBUG -fopenmp

dmlrun: src/dmlsearch.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -o $@ -DNDEBUG -fopenmp

%: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -o $@ -DNDEBUG -fopenmp -O3

mpi%: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -o $@ -DNDEBUG -fopenmp -Ofast -DUSE_BOOST_PARALLEL=1

%pg: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -pg -o $@

%pgf: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -pg -fopenmp -o $@

%f: src/%.cpp $(wildcard include/fgc/*.h)
	$(CXX) $(CXXFLAGS) $< -fopenmp -o $@


osm2dimacsdbg: src/osm2dimacs.cpp
	$(CXX) $(CXXFLAGS) \
        $(OSINC) -pthread \
        $< -lz -lbz2 -lexpat -o $@

osm2dimacs: src/osm2dimacs.cpp
	$(CXX) $(CXXFLAGS) \
        $(OSINC) -pthread \
        $< -lz -lbz2 -lexpat -o $@ -O3 -fopenmp -DNDEBUG

osm2dimacspgf: src/osm2dimacs.cpp
	$(CXX) $(CXXFLAGS) \
        $(OSINC) -pthread \
        $< -lz -lbz2 -lexpat -o $@ -O3 -lbz2 -lexpat -pg -DNDEBUG -fopenmp

osm2dimacspg: src/osm2dimacs.cpp
	$(CXX) $(CXXFLAGS) \
        $(OSINC) -pthread \
        $< -lz -lbz2 -lexpat -o $@ -O3 -lbz2 -lexpat -pg


clean:
	rm -f $(EX) graphrun dmlrun $(FEX) $(PGEX) $(PGFEX)
