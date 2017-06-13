CC=gcc
CFLAGS=-g -Wall
LIBS=-lportaudio -lasound -lpthread -lm
TARGET=run

all: $(TARGET)

$(TARGET): $(TARGET).c
	@$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	@rm -f $(TARGET)
