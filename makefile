CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = readAttractions findDistance linK revSub allPermutations
SOURCE = readAttractions.c findDistance.c linK.c revSub.c allPermutations
OBJECT = $(SOURCE:.c=.o)

all: $(TARGET)

$(TARGET): %: %.o
	$(CC) $(CFLAGS) $< -o $@ -L../cJSON -lcjson

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

debug: CFLAGS += -g
debug: all

clean:
	rm -f $(TARGET) $(OBJECT)
