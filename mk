#! /bin/bash

echo "Removing executable"
rm a.out 2> /dev/null
echo "compiling using gcc *.c"
gcc *.c
echo "mk script done"