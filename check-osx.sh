#! /bin/bash

for file in Traverso.app/Contents/MacOS/*; do
  otool -L $file 
done

for file in Traverso.app/Contents/Frameworks/*.dylib; do
  otool -L $file
done

