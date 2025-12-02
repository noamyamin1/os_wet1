CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic-errors -pthread -DNDEBUG
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
TARGET = smash

all: $(TARGET)

$(TARGET): $(OBJS)
# Include the course-provided object file containing the 'my_system_call' implementation
	$(CC) $(CFLAGS) $(OBJS) my_system_call.o -o $(TARGET) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS)
