@echo off

rem download mingw here: https://github.com/skeeto/w64devkit
rem place the SDL3 development files in C:\SDL3

windres res.rc -O coff -o res.res
gcc src/*.c src/api/*.c lpeg/*.c -Os -s -std=c99 -fno-strict-aliasing -Isrc -IC:\SDL3\include -LC:\SDL3\lib -lSDL3 res.res -o twig.exe
