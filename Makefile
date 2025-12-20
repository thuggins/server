CC = gcc
CFLAGS = -g
LDFLAGS = -lws2_32
TARGET = server.exe

all: $(TARGET)

$(TARGET): server.c
	$(CC) $(CFLAGS) server.c -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
