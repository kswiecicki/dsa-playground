CXX = g++
CXXFLAGS = -std=c++11
ifeq ($(DEBUG),1)
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O2
endif
LDFLAGS =
LIBS = -pthread -ldl
INCS = -I./

# DML library
DML_ROOT = ../../DML
CXXFLAGS += -I$(DML_ROOT)/include
LDFLAGS += -L$(DML_ROOT)/build
LIBS += -l:libdml.a -ldl

# PMEM2 library
PMEM2_ROOT = /usr/local
CXXFLAGS += -I$(PMEM2_ROOT)/include
LDFLAGS += -L$(PMEM2_ROOT)/lib64
LIBS += -l:libpmem2.a -ldl -pthread -lndctl -ldaxctl

# MINIASYNC library
MINIASYNC_ROOT = /home/kswiecic/miniasync
CXXFLAGS += -I$(MINIASYNC_ROOT)/src/include
LDFLAGS += -L$(MINIASYNC_ROOT)/build/src
LIBS += -l:libminiasync.a -ldl -pthread

# extra MINIASYNC-DML library
EXTRAS_ROOT = $(MINIASYNC_ROOT)/extras
CXXFLAGS += -I$(EXTRAS_ROOT)/dml/include
LDFLAGS += -L$(MINIASYNC_ROOT)/build/extras/dml
LIBS += -l:libminiasync-dml.a -ldl -ldml

# Google benchmark
GBENCH_ROOT = ../googlebenchmark
CXXFLAGS += -I$(GBENCH_ROOT)/include
LDFLAGS += -L$(GBENCH_ROOT)/build/src
LIBS += -l:libbenchmark.a

OBJS = benchmark

all: $(OBJS)

benchmark: benchmark.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(INCS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJS)

.PHONY: $(OBJS)
