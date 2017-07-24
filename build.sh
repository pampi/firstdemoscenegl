#!/bin/sh
gcc -Wall -pedantic file_to_c_code.c -o f_to_hex
./f_to_hex music.xm my_tracker_music music.inl
rm ./f_to_hex
g++ -Wall -pedantic -std=c++11 -Os main.cpp demo.cpp -lpthread -lmikmod -lglfw -lGL -o demo
strip -s ./demo
