CC = gcc
CFLAGS = -g -Iinclude
LDFLAGS = -lws2_32 -ladvapi32 -lssl -lcrypto
TARGET = server.exe
SRCS = src/main.c src/ws.c src/http.c src/worker.c src/ssl_helper.c
OBJS = $(SRCS:.c=.o)

all: clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

