CC=gcc
CFLAGS=-c
SRCS=client.c
INC=../message.h
OBJS=$(SRCS:.c=.o)
EXES=$(SRCS:.c=)

all: $(EXES)

$(EXES): client.o
	$(CC) $^ -o $@ -lpthread

$(OBJS): $(SRCS) $(INC)
	$(CC) -g -Wall $(CFLAGS) $(SRCS)

clean:
	rm -f $(OBJS) $(EXES)

print:
	echo $(EXES)
