#!/bin/bash

if [ -z "$1" ]; then
    echo "Target of build should be specified: 'db' or 'client'"
    exit
fi

if [ "$1" = "db" ]; then
    c_entry_point_file=main
elif [ "$1" = "client" ]; then
    c_entry_point_file=client
else
    echo "Invalid target: $1"
    exit
fi

if [ ! -d "output/" ] ; then 
    mkdir "output" ; 
fi

clang -g3 -march=native -fsanitize=address -std=c99 \
 -Wall -Wextra -Wshadow -Wundef \
 -pedantic \
 -Iinclude -Isrc \
 -DDEBUG \
 src/$c_entry_point_file.c -o output/$1
