#!/usr/bin/env bash
#
# Build sniproxy with musl(https://www.musl-libc.org/) and static linking.
#

set -e


function build_libmill() {
    curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
    mv libmill-master libmill
    cd libmill
    ./autogen.sh
    export CC=musl-gcc
    ./configure --enable-shared=false
    make libmill.la
    cd ..
}

function build_sniproxy() {
    curl -s -L https://github.com/XiaoxiaoPu/sniproxy/archive/master.tar.gz | tar -zxf -
    mv sniproxy-master sniproxy
    cd sniproxy
    autoreconf -if
    export CPPFLAGS=-I$(pwd)/../libmill
    export LDFLAGS=-L$(pwd)/../libmill/.libs
    export CC=musl-gcc
    ./configure --prefix=/usr --sysconfdir=/etc --enable-static
    make
    strip src/sniproxy
    cd ..
}


build_libmill
build_sniproxy
cp sniproxy/src/sniproxy sniproxy-bin
rm -rf libmill sniproxy
mv sniproxy-bin sniproxy
