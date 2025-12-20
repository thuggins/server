# Server

A C-based server application.

## Building

To build the project, run:

```bash
make
```

This will compile the source code and generate the executable.

To clean build artifacts:

```bash
make clean
```

## Running

After building, run the server with:

```bash
./server.exe
```

## Files

- `server.c` - Main server source code
- `Makefile` - Build configuration

## Requirements

**MSYS2**

- Download from [MSYS2](https://www.msys2.org/)
- Run the installer and follow the setup instructions
- In the MSYS2 terminal, install the toolchain:
  ```bash
  pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
  ```

**Verify Installation**

Open a command prompt or MSYS2 terminal and run:

```bash
gcc --version
make --version
```

## Recommended VS Code Extensions

For C development, consider installing these extensions:

- **C/C++** (ms-vscode.cpptools) - IntelliSense, debugging, and code browsing for C/C++
- **Makefile Tools** (ms-vscode.makefile-tools) - Intellisense and build support for Makefiles
