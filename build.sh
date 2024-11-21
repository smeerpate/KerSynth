#!/bin/bash

# Compile each module
gcc -c main.c -o main.o
gcc -c OLED.c -o OLED.o
gcc -c UI.c -o UI.o

# Link the object files
#gcc main.o module1.o module2.o -o myprogram
gcc main.o OLED.o UI.o -o kersynth -lfluidsynth -lasound

# Run the program
sudo ./kersynth

# doe eerst dit chmod +x build.sh
# voer dit uit met ./build.sh
