#!/bin/bash

if [ $# == 3 ]; then
    if [ "$1" == DEBUG ]; then
        gdb --args ./picomesh 0 10 0 0 5 5 $2 $3
    elif [ "$1" == VALGRIND ]; then
        valgrind --leak-check=yes ./picomesh 0 10 0 0 5 5 $2 $3
    else
        echo "Usage: ./edge_router.sh <DEBUG/VALGRIND> [size]"
    fi
else
    ./picomesh 0 10 0 0 5 5 $1 $2
fi 
