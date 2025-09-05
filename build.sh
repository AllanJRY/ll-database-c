#!/bin/bash

# TODO: accept args to build as RELEASE or DEV

if [ ! -d "output/" ] ; then 
    mkdir "output" ; 
fi

clang -g3 -march=native -fsanitize=address -std=c99 \
 -Wall -Wextra -Wshadow -Wundef \
 -pedantic \
 -Iinclude -Isrc \
 -DDEBUG \
 src/main.c -o output/db
