CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = read test
SOURCE = read.c test.c
OBJECT = $(SOURCE:.c=.o)

all: $(TARGET)

$(TARGET): %: %.o
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g
debug: all

clean:
	rm -f $(TARGET) $(OBJECT)
