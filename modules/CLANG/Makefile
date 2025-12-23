CC = gcc
CFLAGS = -Wall -g -O0
LDFLAGS = -static
#LDFLAGS =

TARGET = example

SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c header.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

# $< — это имя первого (и единственного) prerequisite (например, main.c)
# Здесь $@ — это имя цели (например, main.o)
# $^ — это список всех prerequisites (например, main.o utils.o)
