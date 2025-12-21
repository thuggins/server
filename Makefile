CC = gcc
CFLAGS = -g -Iinclude
LDFLAGS = -lws2_32 -ladvapi32
TARGET = server.exe
SRCS = src/main.c src/ws.c src/http.c src/worker.c
OBJS = $(SRCS:.c=.o)


all: clean-docs docs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: docs clean-docs

DOCS_DIR = docs
DOXYFILE = Doxyfile
# Detect platform-specific null device
ifeq ($(OS),Windows_NT)
	NULLDEV = nul
else
	NULLDEV = /dev/null
endif

docs:
	@where doxygen >$(NULLDEV) 2>$(NULLDEV) || ( \
	echo Error: doxygen not found. Install it and re-run 'make docs'. && \
	echo Options: Winget 'winget search doxygen' then 'winget install <Id>', Scoop 'scoop install doxygen', or download from https://www.doxygen.nl/download.html && \
	exit 127 )
	doxygen $(DOXYFILE)

clean-docs:
	rm -rf $(DOCS_DIR)/html $(DOCS_DIR)/latex
