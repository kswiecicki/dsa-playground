CC = gcc
ifeq ($(DEBUG),1)
CCFLAGS = -O0 -g
else
CCFLAGS = -O2
endif
LDFLAGS =
LIBS =
INCS = -I./

# DML library
DML_ROOT = /usr/local
CCFLAGS += -I$(DML_ROOT)/include
LDFLAGS += -L$(DML_ROOT)/lib64
LIBS += -l:libdml.a -ldl

OBJS = main.o

playground: $(OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(INCS) $(LDFLAGS) $(LIBS)

%.o: %.c %.h
	$(CC) $(CCFLAGS) -c $< $(INCS) $(LDFLAGS) $(LIBS)
