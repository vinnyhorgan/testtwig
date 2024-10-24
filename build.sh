#!/bin/bash

gcc src/*.c src/api/*.c src/lib/minilua/*.c src/lib/lpeg/*.c -Os -s -std=c99 -fno-strict-aliasing -Isrc -lm -lSDL3 -o twig
