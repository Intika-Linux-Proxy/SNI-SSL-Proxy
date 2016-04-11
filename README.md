# sniproxy #

A simple HTTP/SNI proxy

## Build ##

### 1. Linux/OS X ###

install GNU Autotools, then:

```bash
# build libmill
curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
mv libmill-master libmill
cd libmill
./autogen.sh
./configure
make
rm $(ls .libs/* | grep -v "\.a$")
cd ../
# build muon
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
./configure --host=arm-unknown-linux-gnueabihf
make
rm $(ls .libs/* | grep -v "\.a$")
cd ../
# build muon
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

Copyright (C) 2014 - 2016, Xiaoxiao <i@pxx.io>

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
