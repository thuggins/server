

CC = gcc
CFLAGS = -g -Iinclude -fPIC -fPIE
LDFLAGS = -lcurl -lpthread -fPIE -pie
TARGET = server
SRCS = src/main.c src/ws.c src/http.c src/worker.c src/weather.c src/ws_util.c src/sha1.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

