#!/usr/bin/env bash

set -e


# build libmill
rm -rf libmill
curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
mv libmill-master libmill
cd libmill
./autogen.sh
./configure --enable-shared=false
make libmill.la
cd ../

# build with coverage
if [ -f Makefile ]; then
    make distclean
fi
autoreconf -if
export CPPFLAGS
CPPFLAGS=-I$(pwd)/libmill
export LDFLAGS
LDFLAGS=-L$(pwd)/libmill/.libs
export CFLAGS="-fprofile-arcs -ftest-coverage"
./configure --enable-debug
sed -i 's/nobody/root/g' src/main.c
make
sed -i 's/root/nobody/g' src/main.c

# run tests
src/sniproxy -h || true
src/sniproxy -a || true
src/sniproxy -w || true
src/sniproxy --nothisoption || true
sudo src/sniproxy -a 127.0.0.1 -w 4 &
sleep 1
curl -vv --resolve github.com:80:127.0.0.1 http://github.com/
curl -vv --resolve twitter.com:80:127.0.0.1 http://twitter.com/
curl -vv --resolve github.com:80:127.0.0.1 "http://github.com/?foo=$(dd if=/dev/urandom bs=1 count=8000 | base32 -w 0)"
curl -vv --resolve github.com:443:127.0.0.1 -o /dev/null https://github.com/
curl -vv --resolve twitter.com:443:127.0.0.1 -o /dev/null https://twitter.com/
sudo pkill -INT sniproxy
sudo src/sniproxy -a 127.0.0.1 -w 4 --socks5 127.0.0.1:1080 &
sleep 1
curl -vv --resolve github.com:80:127.0.0.1 http://github.com/
curl -vv --resolve twitter.com:80:127.0.0.1 http://twitter.com/
curl -vv --resolve github.com:80:127.0.0.1 "http://github.com/?foo=$(dd if=/dev/urandom bs=1 count=8000 | base32 -w 0)"
curl -vv --resolve github.com:443:127.0.0.1 -o /dev/null https://github.com/
curl -vv --resolve twitter.com:443:127.0.0.1 -o /dev/null https://twitter.com/
sudo pkill -INT sniproxy

# send coverage report to codecov.io
bash <(curl -s https://codecov.io/bash)

# cleanup
make distclean
