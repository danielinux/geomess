#!/bin/bash

if [ $# == 4 ]; then
    if [ "$1" == DEBUG ]; then
        gdb --args ./picomesh $2 10 0 $3 5 5 $4
    elif [ "$1" == VALGRIND ]; then
        valgrind --leak-check=yes ./picomesh $2 10 0 $3 5 5 $4
    else
        echo "Usage: ./host.sh <DEBUG/VALGRIND> [id] [dst]"
    fi
else
    ./picomesh $1 10 0 $2 5 5 $3
fi 
