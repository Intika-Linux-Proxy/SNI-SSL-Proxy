# SNI-SSL-Proxy #

[![License](https://api.pxx.io/badge/badge/license-GPL%20v3.0-blue.svg)](https://www.gnu.org/licenses/gpl.html)
[![Build Status](https://ci.pxx.io/buildStatus/icon?job=sniproxy)](https://ci.pxx.io/job/sniproxy)

A SNI/TLS/HTTP/HTTPS/IMAPs/POP3s/SMTPs Proxy, Supporting Upstream SOCKS5 proxy

## Usage ##

`sniproxy -a 127.0.0.1 -w 8 --socks5 192.168.0.5:1080`

## Custom DNS Through Firejail ##

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

## Domain Names Filter ##

Domain names filter support can be easily added, but i am not needing that feature so i won't be adding it (feel free to PR), you can run multiple instances of sniproxy under different local IPs to handle different domains routing to different location this much more efficient then domain text filter

Example : 
```
Domain1 -> 10.0.0.1 -> Sniproxy-instance-1 -> Socks5-A
Domain2 -> 10.0.0.2 -> Sniproxy-instance-2 -> Socks5-B
```
 
## Build ##

### 1. Linux/OS X/FreeBSD ###

install GNU Autotools, then:

```bash
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


### 2. Cross compile ###

```bash
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


### 3. Build with static linking ###

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
