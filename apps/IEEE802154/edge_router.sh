#!/bin/bash

if [ $# == 2 ]; then
    if [ "$1" == DEBUG ]; then
        gdb --args ./picomesh 0 10 0 0 5 5 fe80::ff:fe00:2 $2
    elif [ "$1" == VALGRIND ]; then
        valgrind --leak-check=yes ./picomesh 0 10 0 0 5 5 fe80::ff:fe00:2 $2
    else
        echo "Usage: ./edge_router.sh <DEBUG/VALGRIND> [size]"
    fi
else
        ./picomesh 0 10 0 0 5 5 fe80::ff:fe00:2 $1
fi 
