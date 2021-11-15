CC=gcc
CFLAGS=-c
SRCS=server.c
INC=../message.h
OBJS=$(SRCS:.c=.o)
EXES=$(SRCS:.c=)

all: $(EXES)

$(EXES): server.o
	$(CC) $^ -o $@

$(OBJS): $(SRCS) $(INC)
	$(CC) -g -Wall $(CFLAGS) $(SRCS)

clean:
	rm -f $(OBJS) $(EXES)

print:
	echo $(EXES)
