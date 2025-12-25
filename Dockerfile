# Dockerfile for HTTP/WebSocket server
FROM debian:bullseye-slim

WORKDIR /app

RUN apt-get update && apt-get install -y gcc make libcurl4-openssl-dev && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app

# Build the server
RUN make clean && make

# Expose HTTP port
EXPOSE 8080

# Run the server
CMD ["/app/server"]
