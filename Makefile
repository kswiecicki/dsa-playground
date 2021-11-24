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

# UASYNC library
UASYNC_ROOT = /home/kswiecic/libuasync
CFLAGS += -I$(UASYNC_ROOT)/src/include
LDFLAGS += -L$(UASYNC_ROOT)/build/src
LIBS += -l:libuasync.a -ldl -pthread

OBJS_DSA_BASIC = dsa_basic.o
OBJS_DSA_PMEM = dsa_pmem.o
OBJS_DSA_PMEM_UASYNC = dsa_pmem_uasync.o dml_mover.o

BINARIES = dsa_basic dsa_pmem dsa_pmem_uasync

all: $(BINARIES)

dsa_basic: $(OBJS_DSA_BASIC)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

dsa_pmem: $(OBJS_DSA_PMEM)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

dsa_pmem_uasync: $(OBJS_DSA_PMEM_UASYNC)
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< $(INCS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJS_DSA_BASIC) $(OBJS_DSA_PMEM) $(OBJS_DSA_PMEM_UASYNC) $(BINARIES)