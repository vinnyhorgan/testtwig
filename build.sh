#!/bin/bash

gcc src/*.c -O3 -s -std=c99 -fno-strict-aliasing -lSDL3 -o twig