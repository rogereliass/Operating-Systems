CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`
INCLUDES = -Iinclude

SRC = src/main.c src/os.c src/memory.c src/semaphore.c src/parser.c \
      src/priority_queue.c src/fcfs_scheduler.c src/round_robin_scheduler.c \
      src/mlfq_scheduler.c src/gui.c
OBJ = $(SRC:.c=.o)

TARGET = os_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)