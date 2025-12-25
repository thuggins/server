
# HTTP/WebSocket Server

Minimal C-based HTTP and WebSocket server, containerized for easy deployment.


## Building Locally

To build the project locally:

```bash
make
```

To clean build artifacts:

```bash
make clean
```


## Running Locally

After building, run the server:

```bash
./server
```

- Open http://localhost:8080/ to load the demo UI.
- Static files are served from `public/`.
- WebSocket requests are upgraded on `/ws`.
- The demo client in `public/app.js` connects to `ws://localhost:8080/ws`.


## Features

- Minimal HTTP static file server (HTML, JS, CSS)
- RFC 6455 WebSocket handshake (no SSL/TLS required)
- Per-connection worker threads for concurrency



## Docker & Docker Compose

To build and run the server in a container:

```bash
docker compose build
docker compose up -d
```

Or use the provided scripts:

- **rebuild.sh**: Cleans, rebuilds, and starts the container
	```bash
	bash rebuild.sh
	```

- **run.sh**: (if present) Alternative script to start the container
	```bash
	bash run.sh
	```

Access the server at http://localhost:8080/


## Requirements

- GCC, Make, and libcurl development headers (for local builds)
- Docker & Docker Compose (for container builds)


## Recommended VS Code Extensions

- **C/C++** (ms-vscode.cpptools) - IntelliSense, debugging, and code browsing for C/C++
- **Clang-Format** (xaver.clang-format) - Auto-formatting and include sorting
- **Makefile Tools** (ms-vscode.makefile-tools) - Intellisense and build support for Makefiles
