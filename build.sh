#!/bin/sh

trexdir=$PWD

echo "Building Trex..."

echo "Running autoconf..."
autoconf

echo "Running automake..."
automake --add-missing

echo "Creating build directory..."
mkdir -p build
rm -rf build/*
cd build

echo "Build deps..."
cd "${trexdir}/deps"
mkdir -p build
cd build

echo "Building v8"
mkdir -p "${trexdir}/deps/build/v8"
cd "${trexdir}/deps/v8"
make OUTDIR=../build/v8 dependencies
make OUTDIR=../build/v8 library=shared native

echo "Building curl"
cd "${trexdir}/deps/curl"
./buildconf
${trexdir}/deps/curl/configure
make

echo "Building leveldb"
cd "${trexdir}/deps/leveldb"
make
mkdir -p "${trexdir}/deps/build/leveldb"
mv libleveldb.* "${trexdir}/deps/build/leveldb"

echo "Building libxml2"
mkdir -p "${trexdir}/deps/build/libxml2"
cd "${trexdir}/deps/build/libxml2"
${trexdir}/deps/libxml2/configure --with-threads
make

echo "Building libxslt"
mkdir -p "${trexdir}/deps/build/libxslt"
cd "${trexdir}/deps/build/libxslt"
${trexdir}/deps/libxslt/configure --with-threads
make

echo "Running configure..."
cd "${trexdir}/build"
../configure CXXFLAGS="-I${trexdir}/deps/v8/src -I${trexdir}/deps/curl/include -I${trexdir}/deps/leveldb/include -I${trexdir}/deps/libxml2/include -I${trexdir}/deps/libxslt/libxslt" LDFLAGS="-L${trexdir}/deps/build/leveldb -L${trexdir}/deps/build/libxml2 -L${trexdir}/deps/build/libxslt/libxslt -L${trexdir}/deps/build/v8/native -L${trexdir}/deps/curl/lib"

echo "Running make..."
make