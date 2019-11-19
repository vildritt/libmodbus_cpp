#!/bin/bash

THIS=`basename "$0"`
SOURCES=$1
TARGET=$2

if [ -z "$SOURCES" ]
    then
        echo "$THIS: ERROR!: SOURCES is empty"
        exit 1
        .
fi

if [ -z "$TARGET" ]
    then
        echo "$THIS: ERROR!: TARGET is empty"
        exit 1
        .
fi

echo "$THIS: SOURCES=$SOURCES"
echo "$THIS: TARGET=$TARGET"
echo "$THIS: PWD=$PWD"

mkdir -p "$TARGET"
cd "$TARGET"
if [ -f "libmodbus.so" ]
    then
        echo "$THIS: libmodbus.so exists already"
        exit 0
        .
fi


need_some_tools() {
    echo "!!!! ERROR: [$1] NOT installed. HINT: # apt install autoconf automake libtool" 1>&2
    exit 1
}

! which aclocal    && need_some_tools aclocal
! which autoreconf && need_some_tools autoreconf
! which libtoolize && need_some_tools libtool


cd "$SOURCES"
./autogen.sh
./configure --prefix="$PWD/bin"
make install
for file in ./bin/lib/libmodbus.so*; do cp "$file" "$TARGET";done
rm -rf ./bin
