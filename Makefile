CC = gcc
CFLAGS = -Wall -Wextra -O3 -Iinclude
LDFLAGS = -lpthread -lm -ldl

SRC = src/main.c src/scanner.c src/playlist.c
OBJ = $(SRC:.c=.o)
TARGET = music_player

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean