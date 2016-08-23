#!/bin/bash

if [ $# -gt 1 ]; then
    if [ "$1" == DEBUG ]; then
        gdb --args ./picomesh 0 10 0 0 5 5 $2 $3
    elif [ "$1" == VALGRIND ]; then
        valgrind --leak-check=yes ./picomesh 0 10 0 0 5 5 $2 $3
    else
        ./picomesh 0 10 0 0 5 5 $2 $3
        echo "Usage: ./edge_router.sh <DEBUG/VALGRIND> [size] [destination]"
    fi
else
    echo "c = $#"
    echo "Usage: ./edge_router.sh <DEBUG/VALGRIND> [size] [destination]"
    ./picomesh 0 10 0 0 5 5 $2
fi
