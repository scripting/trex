#!/bin/sh

trexdir=$PWD

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
make dependencies
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
cd "${trexdir}/deps/libxml2"
autoconf
automake --add-missing
mkdir -p "${trexdir}/deps/build/libxml2"
cd "${trexdir}/deps/build/libxml2"
${trexdir}/deps/libxml2/configure --with-threads
make

echo "Building libxslt"
cd "${trexdir}/deps/libxslt"
autoconf
automake --add-missing
mkdir -p "${trexdir}/deps/build/libxslt"
cd "${trexdir}/deps/build/libxslt"
${trexdir}/deps/libxslt/configure
make

echo "Building Trex..."
cd "${trexdir}"

echo "Running aclocal..."
aclocal

echo "Running autoconf..."
autoconf

echo "Running automake..."
automake --add-missing

echo "Running configure..."
cd "${trexdir}/build"
../configure CXXFLAGS="-I${trexdir}/deps/v8/src -I${trexdir}/deps/curl/include -I${trexdir}/deps/leveldb/include -I${trexdir}/deps/libxml2/include -I${trexdir}/deps/libxslt/libxslt" LDFLAGS="-L${trexdir}/deps/build/leveldb -L${trexdir}/deps/build/libxml2 -L${trexdir}/deps/build/libxslt/libxslt -L${trexdir}/deps/build/v8/native -L${trexdir}/deps/curl/lib/.lib"

echo "Running make..."
make

echo "When running Trex set LD_LIBRARY_PATH to:"
echo "LD_LIBRARY_PATH=${trexdir}/deps/build/leveldb:${trexdir}/deps/build/libxml2:${trexdir}/deps/build/libxslt/libxslt:${trexdir}/deps/build/v8/native:${trexdir}/deps/curl/lib/.lib:/usr/lib:/usr/local/lib"
