# SNI-SSL-Proxy #

A sniproxy supporting incoming HTTP/HTTPS/IMAPs/POP3s/SMTPs traffic and upstream SOCKS5 proxy

SniProxy: proxify incoming connections based on the hostname contained in the initial request of the TCP session. This can be used to proxify connections based on domain names with a custom DNS server 

When connecting to a domain through TLS/HTTPS the initial TCP session contain the domain name un-encrypted and thus sniproxy can redirect a TLS connection based on that initial negotiation without decrypting the traffic nor needing a private key. this technique require a custom DNS Server that redirect the targeted domains to our sniproxy server (dns server like Unbound, Bind or PowerDNS)

# Example #

Example: Domain > DNS > SniProxy > Socks5 > Real-Domain

Detailed example:

1. Requesting https://www.example.com 
2. Our custom DNS Server resolve example.com to our Sniproxy-Server IP 
3. SniProxy intercept incoming connection requesting example.com
4. SniProxy resolve example.com to get the real IP
5. SniProxy tunnel the connection to upstream

# Features #

- Supporting incoming HTTP/HTTPS/IMAPs/POP3s/SMTPs
- Support upstream SOCKS5 proxy 
- Name-based proxying of HTTPS without decrypting traffic.
- Supports both TLS and HTTP protocols.
- Multi-thread 
- Etc.

# Notes #

SniProxy can not work as a classic proxy and require a custom DNS-Server/Host-File

# Server Name Indication (SNI) #

Is an extension to the TLS computer networking protocol by which a client indicates which hostname it is attempting to connect to at the start of the handshaking process. This allows a server to present multiple certificates on the same IP address and TCP port number and hence allows multiple secure (HTTPS) websites (or any other service over TLS) to be served by the same IP address without requiring all those sites to use the same certificate. It is the conceptual equivalent to HTTP/1.1 name-based virtual hosting, but for HTTPS. The desired hostname is not encrypted in original SNI extension, so an eavesdropper can see which site is being requested. 

## Usage ##

`sniproxy -a 127.0.0.1 -w 8 --socks5 192.168.0.5:1080`

## Custom DNS ##

Can be done with Firejail, setting the upstream DNS server can be necessary if the custom DNS-Server and SniProxy are on the same machine (to avoid a DNS loop)
`firejail --dns=8.8.8.8 --noprofile sniproxy -a 127.0.0.1 -w 8 --socks5 192.168.0.5:1080`

## Command Details ##

```
$ sniproxy -h
usage: socks5 [options]
  -h, --help            show this help
  -a <addr>             listen address, default: 0.0.0.0
  -w <num>              number of workers
  --socks5 HOST[:PORT]  SOCKS5 proxy to use
```

## Listening Ports ##

```
HTTP: 80 
HTTPS: 443 
IMAPs: 993 
POP3s: 995 
SMTPs: 465, 587
```

## Multiple Filter ##

Domain names filter (each domain go through different Socks) can by achieved by running multiple instances of sniproxy under different local IPs to handle different domains routing to different location

Example : 
```
Domain1 -> 10.0.0.1 -> Sniproxy-instance-1 -> Socks5-A
Domain2 -> 10.0.0.2 -> Sniproxy-instance-2 -> Socks5-B
```
 
## Build ##

### Linux/OS X/FreeBSD ###

Install GNU Autotools, then:

```bash
# clone the project 
git clone https://github.com/Intika-Linux-Proxy/SNI-SSL-Proxy.git
cd SNI-SSL-Proxy
# build libmill
curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
mv libmill-master libmill
cd libmill
./autogen.sh
./configure --enable-shared=false
make libmill.la
cd ../
# build sniproxy
autoreconf -if
# export CFLAGS=-march=native
export CPPFLAGS=-I$(pwd)/libmill
export LDFLAGS=-L$(pwd)/libmill/.libs
./configure --prefix=/usr --sysconfdir=/etc
make
make check
sudo make install
```

### Cross compile ###

```bash
# clone the project 
git clone https://github.com/Intika-Linux-Proxy/SNI-SSL-Proxy.git
cd SNI-SSL-Proxy
# setup cross compile tool chain:
export PATH="$PATH:/pato/to/cross/compile/toolchain/bin/"
# build libmill
curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
mv libmill-master libmill
cd libmill
./autogen.sh
./configure --enable-shared=false --host=arm-unknown-linux-gnueabihf
make libmill.la
cd ../
# build sniproxy
autoreconf -if
# export CFLAGS=-march=native
export CPPFLAGS=-I$(pwd)/libmill
export LDFLAGS=-L$(pwd)/libmill/.libs
./configure --host=arm-unknown-linux-gnueabihf \
    --prefix=/usr --sysconfdir=/etc
make
```

### Build with static linking ###

append `--enable-static` while running `./configure`.

## License ##

Copyright (C) 2019, Intika <intika@librefox.org>

Copyright (C) 2016, Xiaoxiao <i@pxx.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
