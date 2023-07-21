CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = read test
SOURCE = read.c test.c
OBJECT = $(SOURCE:.c=.o)

all: $(TARGET)

$(TARGET): %: %.o
	$(CC) $(CFLAGS) $< -o $@ -L/Users/katherineharvey/cJSON -lcjson

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

debug: CFLAGS += -g
debug: all

clean:
	rm -f $(TARGET) $(OBJECT)
