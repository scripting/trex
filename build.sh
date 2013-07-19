#!/bin/sh

trexdir=$PWD

echo "Creating build directory..."
mkdir -p build
rm -rf build/*
cd build

echo "Build deps..."
cd "${trexdir}/deps"
mkdir -p build
mkdir -p usr
mkdir -p usr/lib

trexoutdir=${trexdir}/deps/usr

echo "Building v8"
mkdir -p "${trexdir}/deps/build/v8"
cd "${trexdir}/deps/v8"
make dependencies
make OUTDIR=../build/v8 library=shared native
cp ../build/v8/native/libv8.* ${trexoutdir}/lib/
cp ../build/v8/native/lib.target/libv8.* ${trexoutdir}/lib/

echo "Building curl"
cd "${trexdir}/deps/curl"
./buildconf
${trexdir}/deps/curl/configure --with-ssl --prefix=${trexoutdir}
make
make install

echo "Building leveldb"
cd "${trexdir}/deps/leveldb"
make
mkdir -p "${trexdir}/deps/build/leveldb"
cp libleveldb.* ${trexoutdir}/lib/

echo "Building libxml2"
cd "${trexdir}/deps/libxml2"
autoreconf -i
mkdir -p "${trexdir}/deps/build/libxml2"
cd "${trexdir}/deps/build/libxml2"
${trexdir}/deps/libxml2/configure --with-threads --prefix=${trexoutdir}
make
make install

echo "Building libxslt"
cd "${trexdir}/deps/libxslt"
autoreconf -i
mkdir -p "${trexdir}/deps/build/libxslt"
cd "${trexdir}/deps/build/libxslt"
${trexdir}/deps/libxslt/configure --prefix=${trexoutdir}
make
make install

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
../configure CXXFLAGS="-I${trexdir}/deps/v8/src -I${trexdir}/deps/curl/include -I${trexdir}/deps/leveldb/include -I${trexdir}/deps/libxml2/include -I${trexdir}/deps/libxslt/libxslt" LDFLAGS="-L${trexoutdir}/lib"

echo "Running make..."
make

echo "When running Trex set LD_LIBRARY_PATH to:"
echo "LD_LIBRARY_PATH=${trexoutdir}/lib"
