
# **Project Toralizer**

Toralizer is a tool that reroutes network traffic through the **Tor network** using a dynamically linked shared library. It intercepts application traffic and transparently redirects it through a **SOCKS4 proxy**. This is useful for ensuring anonymity by tunneling traffic through Tor without modifying the application’s source code.

---

## **How Toralizer Works**
Toralizer leverages a **shared library hooking technique** to override the `connect()` function used by applications to establish network connections. Instead of directly connecting to a specified IP, it redirects the traffic through a local **SOCKS4 proxy** (Tor service).

---

## **SOCKS Proxy Overview**
**SOCKS (Socket Secure)** is an internet protocol that routes network packets through a proxy server. SOCKS4 is a lightweight version of this protocol that only supports **TCP connections**.

### **SOCKS4 Protocol:**
When a client wants to connect to a remote server through a SOCKS4 proxy, the following process occurs:

1. The client sends a **CONNECT request** to the proxy server with:
   - Version (`VN = 4`)
   - Command (`CD = 1` for CONNECT)
   - Destination Port (`DSTPORT`)
   - Destination IP (`DSTIP`)
   - User ID (`USERID`)
  
   **Packet Format:**
   ```
   +----+----+----+----+----+----+----+----+----+----+....+----+
   | VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
   +----+----+----+----+----+----+----+----+----+----+....+----+
      1    1      2              4           variable       1
   ```
2. The SOCKS4 server checks access permissions and attempts to connect.
3. The proxy server sends a **response** to the client:
   - Version (`VN = 0`)
   - Status Code (`CD`):
     - `90` - Request granted
     - `91` - Request rejected
     - `92` - Request failed (identd issue)
     - `93` - User ID mismatch
   - Destination Port
   - Destination IP
  
   **Response Packet Format:**
   ```
   +----+----+----+----+----+----+----+----+
   | VN | CD | DSTPORT |      DSTIP        |
   +----+----+----+----+----+----+----+----+
      1    1      2              4
   ```
4. If granted, the proxy relays packets between the client and the remote server.

---

## **Hooking Shared Library in C**
Toralizer implements **LD_PRELOAD hooking** to override `connect()` calls and redirect them through the Tor SOCKS4 proxy. This method allows us to modify network behavior without recompiling applications.

### **Key Steps in the Implementation:**
1. **Compile the code into a shared library (`.so` file).**
2. **Override the `connect()` function** used by applications.
3. **Extract the original IP and port** from the application's `connect()` request.
4. **Intercept and redirect connections** through Tor’s SOCKS4 proxy.

---

## **Implementation Details**

### **1. Proxy Request and Response Structs**
Defined in `toralize.h`:
```c
struct proxy_request {
    int8 vn;           // SOCKS version (4)
    int8 cd;           // Command code (1 for CONNECT)
    int16 dstport;     // Destination port
    int32 dstip;       // Destination IP address
    unsigned char userid[8];  // User ID (optional)
};
```
```c
struct proxy_response {
    int8 vn;  // SOCKS version (always 0)
    int8 cd;  // Response code (90 = success, others = failure)
    int16 _;  // Unused
    int32 __; // Unused
};
```
---

### **2. Hooking the `connect()` Function**
Defined in `toralize.c`:
```c
int connect(int s2, const struct sockaddr *sock2, socklen_t addrlen) {
    int s;
    struct sockaddr_in sock;
    Req *req;
    Res *res;
    char buf[ressize];
    int (*original_connect)(int, const struct sockaddr*, socklen_t);

    // Get the real connect() function from libc
    original_connect = dlsym(RTLD_NEXT, "connect");

    // Create a socket to connect to the proxy
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }

    // Set up the SOCKS proxy details
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    // Connect to the proxy server
    if (original_connect(s, (struct sockaddr *)&sock, sizeof(sock))) {
        perror("connect");
        return -1;
    }

    printf("Connected to proxy\n");

    // Send SOCKS4 CONNECT request
    req = request((struct sockaddr_in *)sock2);
    write(s, req, reqsize);

    // Read response from the proxy
    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1) {
        perror("read");
        free(req);
        close(s);
        return -1;
    }

    res = (Res *)buf;
    if (res->cd != 90) {
        fprintf(stderr, "Unable to traverse the proxy, error code: %d\n", res->cd);
        close(s);
        free(req);
        return -1;
    }

    printf("Successfully connected through the proxy\n");

    // Redirect application traffic through the proxy
    dup2(s, s2);
    free(req);
    return 0;
}
```
---

## **3. Compilation and Installation**
### **Compiling the Shared Library**
To compile `toralize.c` into a shared object file (`toralize.so`):
```sh
gcc toralize.c -o toralize.so -fPIC -shared -ldl -D_GNU_SOURCE
```

### **Setting Up LD_PRELOAD**
To use `toralize.so` to override the system `connect()` function:
```sh
export LD_PRELOAD=/path/to/toralize.so
```

### **Installing Toralizer**
To install and make `toralize` globally available:
```sh
sudo cp toralize.so /usr/lib/
sudo cp toralize /usr/bin/
sudo chmod 755 /usr/bin/toralize
```

---

## **4. Usage Examples**
To route a `curl` request through Tor:
```sh
toralize curl http://example.com
```

To check IP address via a Tor-protected request:
```sh
toralize curl http://whatismyip.com
```

---

## **5. Packages and Dependencies**
### **Required Libraries**
- **`unistd.h`** → System calls for file operations (`read()`, `write()`, `close()`)
- **`sys/socket.h`** → Socket API (`socket()`, `connect()`)
- **`arpa/inet.h`** → Internet address functions (`inet_addr()`, `htons()`)
- **`netinet/in.h`** → IP address structures
- **`dlfcn.h`** → Dynamic linking (`dlsym()`)
- **`stdlib.h`** → Memory allocation (`malloc()`, `free()`)
- **`stdio.h`** → Standard I/O functions (`printf()`, `perror()`)

### **Dependencies**
- **Tor SOCKS Proxy**
  ```sh
  sudo apt install tor
  ```
- **Networking Tools**
  ```sh
  sudo apt install curl net-tools bind9-host
  ```

---

## **6. Debugging**
To test if Tor is running:
```sh
systemctl status tor
```
To manually test the SOCKS proxy:
```sh
curl --socks4a 127.0.0.1:9050 http://check.torproject.org
```

---

## **Conclusion**
Toralizer effectively reroutes traffic through Tor without modifying applications. It leverages **SOCKS4**, **LD_PRELOAD hooking**, and **shared libraries** to provide anonymity and security.

---
