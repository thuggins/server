
# HTTP/WebSocket Server

Minimal C-based HTTP and WebSocket server, containerized for easy deployment.



# HTTP/WebSocket Server

Minimal C-based HTTP and WebSocket server, containerized for easy deployment.

## Features
- Serves static files from `public/` (HTML, JS, CSS)
- RFC 6455 WebSocket handshake (no SSL/TLS required)
- Per-connection worker threads for concurrency
- Weather demo: responds to WebSocket requests with weather data for US capitals

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
- The demo client in `public/app.js` connects to `ws://localhost:8080/ws`.


## Docker & Docker Compose
To build and run the server in a container:
```bash
docker compose build
docker compose up -d
```

Or use the provided script:
```bash
bash run.sh
```

Access the server at http://localhost:8080/

## Requirements
- GCC, Make, and libcurl development headers (for local builds)
- Docker & Docker Compose (for container builds)

## Source Structure
- `src/` — C source files
- `include/` — Header files
- `public/` — Static web assets
- `Makefile` — Build instructions
- `Dockerfile` — Container build instructions
- `docker-compose.yml` — Compose service definition
- `run.sh`, `rebuild.sh` — Helper scripts

## Recommended VS Code Extensions
- **C/C++** (ms-vscode.cpptools) — IntelliSense, debugging, and code browsing for C/C++
- **Clang-Format** (xaver.clang-format) — Auto-formatting and include sorting
- **Makefile Tools** (ms-vscode.makefile-tools) — Intellisense and build support for Makefiles

- GCC, Make, and libcurl development headers (for local builds)
- Docker & Docker Compose (for container builds)


## Recommended VS Code Extensions

- **C/C++** (ms-vscode.cpptools) - IntelliSense, debugging, and code browsing for C/C++
- **Clang-Format** (xaver.clang-format) - Auto-formatting and include sorting
- **Makefile Tools** (ms-vscode.makefile-tools) - Intellisense and build support for Makefiles
