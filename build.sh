#!/bin/sh
gcc -Wall -pedantic file_to_c_code.c -o f_to_hex
./f_to_hex music.xm my_tracker_music music.inl
rm ./f_to_hex
g++ -Wall -pedantic -std=c++11 -Os -fomit-frame-pointer -fno-default-inline -fno-exceptions main.cpp demo.cpp -lpthread -lmikmod -lglfw -lGL -o demo
strip -s -R .comment -R .gnu.version -R .gnu.hash -R .note.gnu.build-id -R .jcr -R .note.ABI-tag ./demo
cat ./self_extract_script.txt > small_demo
#let's use LZMA compression, since it's very common these days
lzma -9e --stdout ./demo >> small_demo
chmod +x ./small_demo
