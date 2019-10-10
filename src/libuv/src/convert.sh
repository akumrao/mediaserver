#!/bin/bash
cd $1
for f in *.c; do 
mv  "$f" "${f%.*}.cpp"
done
