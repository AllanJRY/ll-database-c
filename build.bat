IF NOT EXIST .\output\ ( mkdir .\output\ )

clang -g3 -march=native -std=c99^
 -Wall -Wextra -Wshadow -Wundef^
 -pedantic^
 -Iinclude -Isrc^
 -lws2_32^
 -DDEBUG^
 src/main.c -o output/db.exe
