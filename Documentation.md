# Project Toralizer

This is basically a tor rerouting of the url or any piped command used with it.

eg:
``` bash
toralizer | curl www.example.com
```

## Socks proxy
SOCKS is a networking protocol that facilitates communication between clients and servers through a proxy server

The version which we are going to use in this project is v4 because it is less complex to implement.

### SOCKS4 RFC:
CONNECT

The client connects to the SOCKS server and sends a CONNECT request when
it wants to establish a connection to an application server. The client
includes in the request packet the IP address and the port number of the
destination host, and userid, in the following format.

		+----+----+----+----+----+----+----+----+----+----+....+----+
		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		+----+----+----+----+----+----+----+----+----+----+....+----+
    	   1    1      2              4           variable       1

VN is the SOCKS protocol version number and should be 4. CD is the
SOCKS command code and should be 1 for CONNECT request. NULL is a byte
of all zero bits.

The SOCKS server checks to see whether such a request should be granted
based on any combination of source IP address, destination IP address,
destination port number, the userid, and information it may obtain by
consulting IDENT, cf. RFC 1413.  If the request is granted, the SOCKS
server makes a connection to the specified port of the destination host.
A reply packet is sent to the client when this connection is established,
or when the request is rejected or the operation fails. 

		+----+----+----+----+----+----+----+----+
		| VN | CD | DSTPORT |      DSTIP        |
		+----+----+----+----+----+----+----+----+
 # of bytes:	   1    1      2              4

VN is the version of the reply code and should be 0. CD is the result
code with one of the following values:

	90: request granted
	91: request rejected or failed
	92: request rejected becasue SOCKS server cannot connect to
	    identd on the client
	93: request rejected because the client program and identd
	    report different user-ids

The remaining fields are ignored.

The SOCKS server closes its connection immediately after notifying
the client of a failed or rejected request. For a successful request,
the SOCKS server gets ready to relay traffic on both directions. This
enables the client to do I/O on its connection as if it were directly
connected to the application server.