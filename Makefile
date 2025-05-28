CC = gcc
CFLAGS = -O3 -maes -msse4.2 -Wall
TARGET = elimac
SOURCES = main.c elimac.c elihash.c utils.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)