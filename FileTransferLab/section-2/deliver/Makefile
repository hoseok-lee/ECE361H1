CC=gcc
CFLAGS=-c
SRCS=deliver.c
INC=../packet.h
OBJS=$(SRCS:.c=.o)
EXES=$(SRCS:.c=)

all: $(EXES)

$(EXES): deliver.o
	$(CC) $^ -o $@

$(OBJS): $(SRCS) $(INC)
	$(CC) $(CFLAGS) $(SRCS)

clean:
	rm -f $(OBJS) $(EXES)

print:
	echo $(EXES)
