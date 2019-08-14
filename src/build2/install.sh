#!/bin/bash
dest=door
mkdir $dest
mkdir $dest/lib
cp -r face-resource $dest
cp feden $dest
cp config.ini $dest
cp license.* $dest

deplist=$( ldd $1 | awk '{if (match($3,"/")){ print $3}}' )
for lib in $deplist
do
echo $lib
cp -u $lib $dest/lib
done
rm -f $dest/lib/libm.so.*
rm -f $dest/lib/libc.so.*
rm -f $dest/lib/libdl.so.*
