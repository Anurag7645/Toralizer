# Toralizer - Tor Proxy Rerouting Tool

## Overview
Toralizer is a tool that reroutes network traffic through the **Tor** network using **SOCKS4 proxying**. It hooks into the `connect()` function to intercept and redirect outgoing connections through a Tor proxy, providing anonymity for network requests.

## Features
- Transparent proxying of network connections via **SOCKS4**.
- Hooks the `connect()` system call to reroute traffic.
- Allows commands to be executed over the Tor network.

## Installation

1. Clone or navigate to the project directory:
   ```bash
   cd /mnt/d/study/Project/Toralizer
   ```
2. Compile the shared library:
   ```bash
   make
   ```
   This will produce `toralize.so`, the shared library that implements the hook.

3. Move the executable to `/usr/bin` for system-wide access:
   ```bash
   sudo cp toralize /usr/bin/
   sudo chmod 755 /usr/bin/toralize
   ```

## How It Works

### Hooking the `connect()` System Call
- The `connect()` function in `toralize.c` is overridden using **dynamic linking** (`dlsym(RTLD_NEXT, "connect")`).
- Instead of connecting directly to the target server, the connection is routed through the Tor **SOCKS4 proxy** running on `127.0.0.1:9050`.

### SOCKS4 Proxy Handling
- The `Req` structure constructs a **SOCKS4 CONNECT request**.
- The proxy verifies and establishes the connection before forwarding traffic.
- The `Res` structure handles the response from the proxy, determining success or failure.

### Usage
#### Basic Usage
To route a command through Tor:
```bash
toralize curl http://example.com
```
Expected output:
```
Connected to proxy
Successfully connected through the proxy to example.com
```

#### Checking Tor Status
Before using Toralizer, ensure Tor is running:
```bash
systemctl status tor
```
Expected output:
```
● tor.service - Anonymizing overlay network for TCP
   Active: active (running)
```

#### DNS Resolution via Tor
If you want to resolve hostnames through Tor, use:
```bash
host whatismyip.com
```

#### Example Proxying
```bash
toralize curl http://www.google.se
```
Expected output:
```
Connected to proxy
Successfully connected through the proxy to Google
```

## File Structure
- **toralize.c**: Implements the proxy hooking mechanism.
- **toralize.h**: Defines data structures and constants.
- **toralize.so**: Compiled shared library.
- **Makefile**: Compilation instructions.
- **Documentation.md**: This documentation.

## Dependencies
- **GCC** (for compilation)
- **Tor** (must be running)
- **bind9-host** (for DNS resolution via `host` command)

## Conclusion
Toralizer provides a seamless way to anonymize network requests via Tor. It integrates with system calls to transparently redirect traffic through a SOCKS4 proxy, enhancing privacy and security.

