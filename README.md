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

- Open http://localhost:8080/ to load the demo UI.
- The server serves static files from `public/` and upgrades WebSocket requests on `/ws`.
- The demo client in `public/app.js` connects to `ws://localhost:8080/ws` and echoes messages.

## Features

- Minimal HTTP static file server (HTML, JS, CSS)
- RFC 6455 WebSocket handshake with basic text and ping/pong support
- Per-connection worker threads to avoid blocking on persistent sockets

## Local Development SSL Certificate

This project uses a self-signed certificate for HTTPS/WSS on localhost.

**Do not commit your private key or .pfx files to version control.**

To generate your own local certificate and key (Windows PowerShell):

```powershell
# Generate a self-signed cert for localhost
New-SelfSignedCertificate -DnsName "localhost" -CertStoreLocation "cert:\LocalMachine\My" -FriendlyName "LocalDevCert"

# Export to PFX (replace <Thumbprint> with your cert's thumbprint)
$pwd = ConvertTo-SecureString -String "password" -Force -AsPlainText
Export-PfxCertificate -Cert "cert:\LocalMachine\My\<Thumbprint>" -FilePath "localhost.pfx" -Password $pwd

# Convert to PEM (requires OpenSSL)
openssl pkcs12 -in localhost.pfx -nocerts -out localhost-key.pem -nodes
openssl pkcs12 -in localhost.pfx -clcerts -nokeys -out localhost-cert.pem
```

After generating, place `localhost-cert.pem` and `localhost-key.pem` in the project root.

## Documentation

This project includes Doxygen annotations. The generated HTML docs use `README.md` as the main page.

Generate docs from the terminal:

```bash
make docs
```

Open the documentation at:

- [docs/html/index.html](docs/html/index.html)

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
