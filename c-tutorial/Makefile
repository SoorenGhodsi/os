CC = gcc
CFLAGS = -Wall -Wextra
TARGET = list_harness
OBJS = harness.o list.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

harness.o: harness.c list.h
	$(CC) $(CFLAGS) -c harness.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm -f $(OBJS) $(TARGET)

tar:
	tar czvf $(TARGET).tar.gz *.c *.h Makefile

.PHONY: all clean tar
