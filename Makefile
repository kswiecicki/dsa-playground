CC = gcc
ifeq ($(DEBUG),1)
CFLAGS = -O0 -g
else
CFLAGS = -O2
endif
LDFLAGS =
LIBS =
INCS = -I./

# DML library
DML_ROOT = /usr/local
CFLAGS += -I$(DML_ROOT)/include
LDFLAGS += -L$(DML_ROOT)/lib64
LIBS += -l:libdml.a -ldl

# PMEM2 library
PMEM2_ROOT = /usr/local
CFLAGS += -I$(PMEM2_ROOT)/include
LDFLAGS += -L$(PMEM2_ROOT)/lib64
LIBS += -l:libpmem2.a -ldl -pthread -lndctl -ldaxctl

# MINIASYNC library
MINIASYNC_ROOT = /home/kswiecic/miniasync
CFLAGS += -I$(MINIASYNC_ROOT)/src/include
LDFLAGS += -L$(MINIASYNC_ROOT)/build/src
LIBS += -l:libminiasync.a -ldl -pthread

# extra MINIASYNC-DML library
EXTRAS_ROOT = $(MINIASYNC_ROOT)/extras
CFLAGS += -I$(EXTRAS_ROOT)/dml/include
LDFLAGS += -L$(MINIASYNC_ROOT)/build/extras/dml
LIBS += -l:libminiasync-dml.a -ldml

OBJS_DSA_BASIC = dsa_basic.o
OBJS_DSA_PMEM = dsa_pmem.o
OBJS_DSA_PMEM_MINIASYNC = dsa_pmem_miniasync.o
OBJS_DSA_BATCH = dsa_batch.o

BINARIES = dsa_basic dsa_pmem dsa_pmem_miniasync dsa_batch

all: $(BINARIES)

dsa_basic: $(OBJS_DSA_BASIC)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

dsa_pmem: $(OBJS_DSA_PMEM)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

dsa_pmem_miniasync: $(OBJS_DSA_PMEM_MINIASYNC)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

dsa_batch: $(OBJS_DSA_BATCH)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< $(INCS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJS_DSA_BASIC) $(OBJS_DSA_PMEM) $(OBJS_DSA_PMEM_MINIASYNC) $(OBJS_DSA_BATCH) $(BINARIES)